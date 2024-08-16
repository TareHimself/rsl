import { Node, ModuleNode, IncludeNode, DefineNode, StructNode, NamedScopeNode, LayoutNode, PushConstantNode, FunctionNode, ELayoutType, DeclarationNode, EScopeType, ForNode, NoOpNode, ScopeNode, BufferDeclarationNode, BlockDeclarationNode, StructDeclarationNode, DeclarationScopeNode, IfNode, ReturnNode, IdentifierNode, FunctionArgumentNode, IntLiteral, FloatLiteral, AccessNode, ArrayLiteralNode, AssignNode, BinaryOpAndAssignNode, BinaryOpNode, CallNode, ConditionalNode, ConstNode, DecrementNode, DiscardNode, EBinaryOp, IncrementNode, IndexNode, NegateNode, PrecedenceNode, EDeclarationType } from "./node";
import Token, { ETokenType, KEYWORD_TO_TOKENS, TokenDebugInfo } from "./token";
import TokenList from "./TokenList";


function consumeTill(
    input: TokenList, 
    initialScope: number, 
    ...stopTokens: ETokenType[]
): TokenList {
    const result = new TokenList([]);
    let scope = initialScope;
    const stopTokensSet = new Set(stopTokens);

    while (!input.empty()) {
        const frontToken = input.front();

        switch (frontToken.type) {
            case ETokenType.OpenBrace:
            case ETokenType.OpenParen:
            case ETokenType.OpenBracket:
                if (stopTokensSet.has(frontToken.type) && scope === 0) {
                    return result;
                }
                scope++;
                break;

            case ETokenType.CloseBrace:
            case ETokenType.CloseParen:
            case ETokenType.CloseBracket:
                if (stopTokensSet.has(frontToken.type) && scope === 0) {
                    return result;
                }
                scope--;
                break;
        }

        if (stopTokensSet.has(frontToken.type) && scope === 0) {
            return result;
        }

        result.insertBack(input.removeFront());
    }

    return result;
}

 // Function to parse call arguments
 export function parseCallArguments(input: TokenList): Node[] {
    const callTokens = consumeTill(input, 0, ETokenType.CloseParen);
    input.expectFront(ETokenType.CloseParen).removeFront();
    callTokens.expectFront(ETokenType.OpenParen).removeFront();

    const args: Node[] = [];

    while (callTokens.notEmpty()) {
        const argumentTokens = consumeTill(callTokens, 0, ETokenType.Comma);
        if (callTokens.notEmpty() && callTokens.front().type === ETokenType.Comma) {
            callTokens.removeFront();
        }
        args.push(parseExpression(argumentTokens));
    }

    return args;
}

// Function to resolve token to literal or identifier
export function resolveTokenToLiteralOrIdentifier(token: Token): Node {
    const val = token.value;
    const asInt = parseInt(val);
    if (!isNaN(asInt)) return new IntLiteral(asInt,token.debug);

    const asFloat = parseFloat(val);
    if (!isNaN(asFloat)) return new FloatLiteral(asFloat,token.debug);

    return new IdentifierNode(token.value,token.debug);
}

// Function to parse array literal
export function parseArrayLiteral(input: TokenList): ArrayLiteralNode {
    input.expectFront(ETokenType.OpenBrace);
    const literalTokens = consumeTill(input, 0, ETokenType.CloseBrace);
    const start = literalTokens.expectFront(ETokenType.OpenBrace).removeFront();
    const end = literalTokens.expectBack(ETokenType.CloseBrace).removeBack();

    const expressions: Node[] = [];
    while (literalTokens.notEmpty()) {
        const expression = consumeTill(literalTokens, 0, ETokenType.Comma);
        expressions.push(parseExpression(expression));
        if (literalTokens.notEmpty() && literalTokens.front().type === ETokenType.Comma) {
            literalTokens.removeFront();
        }
    }

    return new ArrayLiteralNode(expressions,start.debug.combine(end.debug));
}

// Function to parse primary
export function parsePrimary(input: TokenList): Node {
    input.front(); // Assert we have input

    switch (input.front().type) {
        case ETokenType.Const:
            {
                const a = input.removeFront();
                return new ConstNode(parseDeclaration(input),a.debug);
            }
        case ETokenType.Identifier: {
            const front = input.removeFront();
            if (input.notEmpty() && input.front().type === ETokenType.Identifier) {
                input.insertFront(front);
                return parseDeclaration(input);
            }
            return resolveTokenToLiteralOrIdentifier(front);
        }
        case ETokenType.TypeFloat:
        case ETokenType.TypeInt:
        case ETokenType.TypeFloat2:
        case ETokenType.TypeInt2:
        case ETokenType.TypeFloat3:
        case ETokenType.TypeInt3:
        case ETokenType.TypeFloat4:
        case ETokenType.TypeInt4:
        case ETokenType.TypeMat3:
        case ETokenType.TypeMat4: {
            const targetToken = input.removeFront();
            if (input.notEmpty() && (input.front().type === ETokenType.Identifier || input.front().type === ETokenType.Unknown)) {
                input.insertFront(targetToken);
                return parseDeclaration(input);
            }
            return parseAccess(input, resolveTokenToLiteralOrIdentifier(targetToken));
        }
        case ETokenType.OpIncrement:
        case ETokenType.OpDecrement: {
            const op = input.removeFront();
            if (input.empty()) input.front();
            const next = parseAccess(input);
            if (op.type === ETokenType.OpIncrement) return new IncrementNode(next, true,op.debug);
            return new DecrementNode(next, true,op.debug);
        }
        case ETokenType.OpenParen: {
            const parenTokens = consumeTill(input, 0, ETokenType.CloseParen);
            const start = parenTokens.expectFront(ETokenType.OpenParen).removeFront();
            const end = input.expectFront(ETokenType.CloseParen).removeFront();
            return new PrecedenceNode(parseExpression(parenTokens),start.debug.combine(end.debug));
        }
        case ETokenType.OpenBrace:
            return parseArrayLiteral(input);
        case ETokenType.OpSubtract:
            {
                const a = input.removeFront();
                return new NegateNode(parsePrimary(input),a.debug);
            }
        case ETokenType.PushConstant:
            {
                const node = input.removeFront();
                return new IdentifierNode(node.value,node.debug);
            }
        case ETokenType.Discard:
            return new DiscardNode(input.removeFront().debug);
        default:
            throw new Error("Unknown Primary Token");
    }
}

// Function to parse access
export function parseAccess(input: TokenList, initialLeft?: Node): Node {
    let left = initialLeft || parsePrimary(input);

    while (input.notEmpty() && (input.front().type === ETokenType.Access || input.front().type === ETokenType.OpenBracket || (input.front().type === ETokenType.OpenParen && left instanceof IdentifierNode))) {
        switch (input.front().type) {
            case ETokenType.OpenParen:
                if (left instanceof IdentifierNode) left = new CallNode((left as IdentifierNode).id, parseCallArguments(input),left.debug);
                break;
            case ETokenType.Access:
                {
                    const access  = input.removeFront();
                    left = new AccessNode(left, parsePrimary(input),access.debug);
                break;
                }
            case ETokenType.OpenBracket:
                {
                    const start = input.removeFront();
                    const within = consumeTill(input, 0, ETokenType.CloseBracket);
                    const end = input.expectFront(ETokenType.CloseBracket).removeFront();
                    left = new IndexNode(left, parseExpression(within),start.debug.combine(end.debug));
                    break;
                }
        }
    }

    return left;
}

// Function to parse multiplicative expressions
export function parseMultiplicative(input: TokenList): Node {
    let left = parseAccess(input);
    while (input.notEmpty() && (input.front().type === ETokenType.OpMultiply || input.front().type === ETokenType.OpDivide || input.front().type === ETokenType.OpMod)) {
        const token = input.removeFront();
        const right = parseAccess(input);
        left = BinaryOpNode.FromTokenType(left, right, token.type,token.debug);
    }
    return left;
}

// Function to parse additive expressions
export function parseAdditive(input: TokenList): Node {
    let left = parseMultiplicative(input);
    while (input.notEmpty() && (input.front().type === ETokenType.OpAdd || input.front().type === ETokenType.OpSubtract)) {
        const token = input.removeFront();
        const right = parseMultiplicative(input);
        left = BinaryOpNode.FromTokenType(left, right, token.type,token.debug);
    }
    return left;
}

// Function to parse comparison expressions
export function parseComparison(input: TokenList): Node {
    let left = parseAdditive(input);
    while (input.notEmpty() && (input.front().type === ETokenType.OpEqual || input.front().type === ETokenType.OpNotEqual || input.front().type === ETokenType.OpLess || input.front().type === ETokenType.OpLessEqual || input.front().type === ETokenType.OpGreater || input.front().type === ETokenType.OpGreaterEqual)) {
        const token = input.removeFront();
        const right = parseAdditive(input);
        left = BinaryOpNode.FromTokenType(left, right, token.type,token.debug);
    }
    return left;
}

// Function to parse logical expressions
export function parseLogical(input: TokenList): Node {
    let left = parseComparison(input);
    while (input.notEmpty() && (input.front().type === ETokenType.OpAnd || input.front().type === ETokenType.OpOr || input.front().type === ETokenType.OpNot)) {
        const token = input.removeFront();
        const right = parseComparison(input);
        left = BinaryOpNode.FromTokenType(left, right, token.type,token.debug);
    }
    return left;
}

// Function to parse increment/decrement expressions
export function parseIncrementDecrement(input: TokenList): Node {
    let left = parseLogical(input);
    if (input.notEmpty() && (left instanceof IdentifierNode || left instanceof AccessNode) && (input.front().type === ETokenType.OpIncrement || input.front().type === ETokenType.OpDecrement)) {
        const op = input.removeFront();
        if (op.type === ETokenType.OpIncrement) return new IncrementNode(left, false,op.debug);
        return new DecrementNode(left, false,op.debug);
    }
    return left;
}

// Function to parse conditional expressions
export function parseConditional(input: TokenList): Node {
    const left = parseIncrementDecrement(input);
    if (input.notEmpty() && input.front().type === ETokenType.Conditional) {
        input.removeFront();
        const condition = left;
        const leftTokens = consumeTill(input, 0, ETokenType.Colon);
        input.expectFront(ETokenType.Colon).removeFront();
        const cLeft = parseExpression(leftTokens);
        const cRight = parseExpression(input);
        return new ConditionalNode(condition,cLeft,cRight,condition.debug.combine(cRight.debug));
    }
    return left;
}

// Function to parse assignment expressions
export function parseAssignment(input: TokenList): Node {
    let left = parseConditional(input);
    while (input.notEmpty() && (input.front().type === ETokenType.Assign || input.front().type === ETokenType.OpAddAssign || input.front().type === ETokenType.OpDivideAssign || input.front().type === ETokenType.OpMultiplyAssign || input.front().type === ETokenType.OpSubtractAssign)) {
        const tok = input.removeFront();
        if (tok.type === ETokenType.Assign) {
            const next = parseConditional(input);
            const debug = left.debug.combine(next.debug);
            left = new AssignNode(left,next,debug);
        } else {
            const next = parseConditional(input);
            const debug = left.debug.combine(next.debug);
            left = BinaryOpAndAssignNode.FromTokenType(left,next,tok.type,debug);
        }
    }
    return left;
}

// Function to parse expressions
export function parseExpression(input: TokenList): Node {
    return parseAssignment(input);
}

export function parseScope(input: TokenList): ScopeNode {
    const start = input.expectFront(ETokenType.OpenBrace).removeFront();
    const statements: Node[] = [];
    while (input.front().type !== ETokenType.CloseBrace)
    {
        switch (input.front().type)
        {
            case ETokenType.OpenParen:
                statements.push(parseScope(input));
                break;
            case ETokenType.For:
                statements.push(parseFor(input));
                break;
            case ETokenType.If:
                statements.push(parseIf(input));
                break;
            default:
                statements.push(parseStatement(input));
                break;
        }
    }

    const end = input.removeFront();

    return new ScopeNode(statements,start.debug.combine(end.debug));
}

export function parseFor(input: TokenList): ForNode {
    const start = input.expectFront(ETokenType.For).removeFront();
    input.expectFront(ETokenType.OpenParen).removeFront();
    var predicate = consumeTill(input, 0, ETokenType.StatementEnd);
    input.expectFront(ETokenType.StatementEnd).removeFront();
    var condition = consumeTill(input, 0, ETokenType.StatementEnd);
    input.expectFront(ETokenType.StatementEnd).removeFront();
    var after = consumeTill(input, 0, ETokenType.StatementEnd);
    input.expectFront(ETokenType.StatementEnd).removeFront();

    var initialNode = predicate.empty() ? new NoOpNode() : parseExpression(predicate);
    var conditionNode = condition.empty() ? new NoOpNode() : parseExpression(condition);
    var updateNode = after.empty() ? new NoOpNode() : parseExpression(after);
    const scope = parseScope(input);
    return new ForNode(initialNode, conditionNode, updateNode, scope,start.debug.combine(scope.debug));
}

export function parseIf(input: TokenList): IfNode {
    const start = input.expectFront(ETokenType.If).removeFront();
    input.expectFront(ETokenType.OpenParen).removeFront();
    var conditionTokens = consumeTill(input, 1, ETokenType.CloseParen);
    input.expectFront(ETokenType.CloseParen).removeFront();
    var condition = parseExpression(conditionTokens);
    var scope = parseScope(input);
    if (input.notEmpty() && input.front().type == ETokenType.Else)
    {
        input.removeFront();
        const next = input.front().type == ETokenType.If ? parseIf(input) : parseScope(input);
        return  new IfNode(condition, scope, next,start.debug.combine(next.debug));
    }
    else
    {
        return new IfNode(condition, scope, new NoOpNode(),start.debug.combine(scope.debug));
    }
}

export function parseDeclaration(input: TokenList): DeclarationNode {
    var type = input.removeFront();

    if(type.type === ETokenType.TypeBuffer){
        const name = input.expectFront(ETokenType.Unknown).removeFront();
        const declarations = parseStructScope(input);
        return new BufferDeclarationNode(name.value,-1,declarations,type.debug.combine(declarations.debug));
    }

    if(type.type === ETokenType.OpenBrace){
        input.insertFront(type);
        const declarations = parseStructScope(input);
        const name = input.expectFront(ETokenType.Unknown).removeFront();
        return new BlockDeclarationNode(name.value,-1,declarations,declarations.debug.combine(name.debug));
    }


    const name = input.front().type === ETokenType.Unknown ? input.expectFront(ETokenType.Unknown).removeFront() : "";

    let returnCount = 0;
    let debug = type.debug;
    if(input.notEmpty() && input.front().type == ETokenType.OpenBracket){
        input.removeFront();
        returnCount - input.front().type === ETokenType.Numeric ? parseInt(input.removeFront().value) : -1;

        input.expectFront(ETokenType.CloseBracket).removeFront();
    }
    
    return new DeclarationNode(EDeclarationType.Block,"",0,TokenDebugInfo.empty())//type.type === ETokenType.Unknown ? new StructDeclarationNode(type.value,name,returnCount)
}

export function parseInclude(input: TokenList): IncludeNode {
    const includeTok = input.expectFront(ETokenType.Include).removeFront();
    const identifierTok = input.expectFront(ETokenType.Identifier).removeFront();
    return new IncludeNode(includeTok.debug.file, new IdentifierNode(identifierTok.value,identifierTok.debug), includeTok.debug)
}

export function parseDefine(input: TokenList): DefineNode {
    const tok = input.expectFront(ETokenType.Define).removeFront();
    var id = input.expectFront(ETokenType.Identifier).removeFront();
    var expr = parseExpression(input);
    return new DefineNode(id.value, expr, tok.debug);
}

export function parseStructScope(input: TokenList): DeclarationScopeNode {
    const declarations: DeclarationNode[] = [];
    const start = input.expectFront(ETokenType.OpenBrace).removeFront();
    while (input.front().type !==  ETokenType.CloseBrace)
    {
        declarations.push(parseDeclaration(input));
        input.expectFront(ETokenType.StatementEnd).removeFront();
    }

    const end = input.expectFront(ETokenType.CloseBrace).removeFront();
    return new DeclarationScopeNode(declarations,start.debug.combine(end.debug));
}

export function parseStruct(input: TokenList): StructNode {
    const type = input.expectFront(ETokenType.TypeStruct).removeFront();
    var structIdentifier = input.expectFront(ETokenType.Identifier).removeFront();
    const scope = parseStructScope(input);
    return new StructNode(structIdentifier.value,scope,structIdentifier.debug);
}

export function parseNamedScope(input: TokenList): NamedScopeNode {
    const start = input.expectFront(ETokenType.VertexScope, ETokenType.FragmentScope).removeFront();
    input.expectFront(ETokenType.OpenBrace).removeFront();

    const statements: Node[] = [];
    while (input.front().type !== ETokenType.CloseBrace)
    {
        switch (input.front().type)
        {
            case ETokenType.Layout:
                statements.push(parseLayout(input));
                break;
            case ETokenType.Define:
                statements.push(parseDefine(input));
                break;
            case ETokenType.PushConstant:
                statements.push(parsePushConstant(input));
                break;
            case ETokenType.Function:
                statements.push(parseFunction(input));
                break;
            case ETokenType.Const:
            {
                statements.push(parseStatement(input));
            }
                break;
            case ETokenType.Include:
            {
                statements.push(parseInclude(input));
            }
                break;
            case ETokenType.TypeStruct:
            {
                statements.push(parseStruct(input));
            }
                break;
            default:
                throw new Error("Unexpected Token");
        }
    }

    const end = input.removeFront();

    return new NamedScopeNode(start.type === ETokenType.VertexScope ? EScopeType.Vertex : EScopeType.Fragment, statements,start.debug.combine(end.debug));
}

export function parseStatement(input: TokenList): Node {
    var statementTokens = consumeTill(input, 0, ETokenType.StatementEnd);
    input.expectFront(ETokenType.StatementEnd).removeFront();

    switch (statementTokens.front().type)
    {
        case ETokenType.Return:
        {
            const retToken = statementTokens.removeFront();

            return new ReturnNode(parseExpression(statementTokens),retToken.debug);
        }
        default:
            return parseExpression(statementTokens);
    }
}

export function parseLayout(input: TokenList): LayoutNode {
    const start = input.expectFront(ETokenType.Layout).removeFront();
    input.expectFront(ETokenType.OpenParen).removeFront();

    const tags: { [key: string]: string } = {};

    while (input.front().type !== ETokenType.CloseParen) {
        tags[input.expectFront(ETokenType.Identifier).removeFront().value] = input.expectFront(ETokenType.Identifier).removeFront().value;
    }

    input.expectFront(ETokenType.CloseParen).removeFront();

    let layoutType = ELayoutType.In;
    switch (input.removeFront().type) {
        case ETokenType.DataIn:
            layoutType = ELayoutType.In;
            break
        case ETokenType.DataOut:
            layoutType = ELayoutType.Out;
            break
        case ETokenType.Uniform:
            layoutType = ELayoutType.Uniform;
            break
        case ETokenType.ReadOnly:
            layoutType = ELayoutType.ReadOnly;
            break
        default:
            layoutType = ELayoutType.In;
    }

    const declaration = parseDeclaration(input);
    const end = input.expectFront(ETokenType.StatementEnd).removeFront();

    return new LayoutNode(tags,layoutType,declaration,start.debug.combine(end.debug));
}



export function parsePushConstant(input: TokenList): PushConstantNode {
    const start = input.expectFront(ETokenType.PushConstant).removeFront();
    input.expectFront(ETokenType.OpenParen).removeFront();

    const tags: { [key: string]: string } = {};

    while (input.front().type !== ETokenType.CloseParen) {
        tags[input.expectFront(ETokenType.Identifier).removeFront().value] = input.expectFront(ETokenType.Identifier).removeFront().value;
    }

    input.expectFront(ETokenType.CloseParen).removeFront();
    
    const next = input.front();

    const declarations = parseStructScope(input);

    const end = input.expectFront(ETokenType.StatementEnd).removeFront();

    return new PushConstantNode("push",new StructNode("",declarations,declarations.debug),tags,start.debug.combine(end.debug));
}

export function parseFunction(input: TokenList): FunctionNode {
    const fnToken = input.expectFront(ETokenType.Function).removeFront();
    const returnTokenType = KEYWORD_TO_TOKENS[fnToken.value] ?? ETokenType.Identifier;
    const returnCount = input.expectFront(ETokenType.DeclarationCount).removeFront();
    const fnName = input.expectFront(ETokenType.Identifier).removeFront();
    input.expectFront(ETokenType.OpenParen).removeFront();
    const args: FunctionArgumentNode[] = [];

    while (input.front().type !== ETokenType.CloseParen)
    {
        var isInput = true;
        if (input.front().type === ETokenType.DataIn || input.front().type === ETokenType.DataOut)
        {
            isInput = input.front().type == ETokenType.DataIn;
            input.removeFront();
        }
        const decl = parseDeclaration(input);
        args.push(new FunctionArgumentNode(decl,isInput,decl.debug));
    }

    input.expectFront(ETokenType.CloseParen).removeFront();

    var returnCountInt = parseInt(returnCount.value);

    let returnDeclaration: DeclarationNode | undefined = null;

    try {
        returnDeclaration = new DeclarationNode(DeclarationNode.tokenTypeToDeclarationType(returnTokenType), "", returnCountInt,fnToken.debug.combine(returnCount.debug))
    } catch (error) {
        returnDeclaration = new StructDeclarationNode("",fnToken.value,parseInt(returnCount.value),fnToken.debug.combine(returnCount.debug));
    }

    return new FunctionNode(fnName.value, returnDeclaration, args,parseScope(input),fnName.debug);
}

export function parse(input: TokenList) {
    if (input.empty()) return new ModuleNode("", [], TokenDebugInfo.empty());

    const debug = input.front().debug.combine(input.back().debug);

    const statements: Node[] = [];

    while (input.notEmpty()) {
        switch (input.front().type) {
            case ETokenType.Include:
                statements.push(parseInclude(input));
                break
            case ETokenType.Define:
                statements.push(parseDefine(input));
                break
            case ETokenType.TypeStruct:
                statements.push(parseStruct(input));
                break
            case ETokenType.VertexScope:
            case ETokenType.FragmentScope:
                statements.push(parseNamedScope(input));
                break
            case ETokenType.Const:
                statements.push(parseStatement(input));
                break
            case ETokenType.Layout:
                statements.push(parseLayout(input));
                break
            case ETokenType.PushConstant:
                statements.push(parsePushConstant(input));
                break
            case ETokenType.Function:
                statements.push(parseFunction(input));
                break
            default:
                throw new Error("Unexpected Token");
        }
    }

    return new ModuleNode(debug.file, statements, debug);
}