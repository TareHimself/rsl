using System.Collections.Frozen;
using ashl.Tokenizer;

namespace ashl.Parser;

public class Parser
{
    public virtual TokenList<Token> ConsumeTill(TokenList<Token> input, int initialScope, params TokenType[] stopTokens)
    {
        var result = input.CreateEmpty();
        var scope = initialScope;
        var stopTokensSet = stopTokens.ToFrozenSet();
        while (input.NotEmpty())
        {
            switch (input.Front().Type)
            {
                case TokenType.OpenBrace or TokenType.OpenParen or TokenType.OpenBracket:
                    if (stopTokensSet.Contains(input.Front().Type) && scope == 0) return result;

                    scope++;
                    break;
                case TokenType.CloseBrace or TokenType.CloseParen or TokenType.CloseBracket:
                    if (stopTokensSet.Contains(input.Front().Type) && scope == 0) return result;

                    scope--;
                    break;
            }

            if (stopTokensSet.Contains(input.Front().Type) && scope == 0) return result;

            result.InsertBack(input.RemoveFront());
        }

        return result;
    }


    public virtual IEnumerable<Node> ParseCallArguments(TokenList<Token> input)
    {
        var callTokens = ConsumeTill(input, 0, TokenType.CloseParen);
        input.ExpectFront(TokenType.CloseParen).RemoveFront();
        callTokens.ExpectFront(TokenType.OpenParen).RemoveFront();

        List<Node> arguments = new();

        while (callTokens.NotEmpty())
        {
            var argumentTokens = ConsumeTill(callTokens, 0, TokenType.Comma);

            if (callTokens.NotEmpty() && callTokens.Front().Type == TokenType.Comma) callTokens.RemoveFront();

            arguments.Add(ParseExpression(argumentTokens));
        }

        return arguments;
    }

    public virtual Node ResolveTokenToLiteralOrIdentifier(Token token)
    {
        var val = token.Value;
        if (int.TryParse(val, out var asInt)) return new IntLiteral(asInt);

        if (float.TryParse(val, out var asFloat)) return new FloatLiteral(asFloat);

        return new IdentifierNode(token.Value);
    }

    public virtual ArrayLiteralNode ParseArrayLiteral(TokenList<Token> input)
    {
        input.ExpectFront(TokenType.OpenBrace);
        var literalTokens = ConsumeTill(input, 0, TokenType.CloseParen);
        literalTokens.ExpectFront(TokenType.OpenBrace).RemoveFront();
        literalTokens.ExpectBack(TokenType.CloseBrace).RemoveBack();
        List<Node> expressions = new();
        while (literalTokens.NotEmpty())
        {
            var expression = ConsumeTill(literalTokens, 0, TokenType.Comma);
            expressions.Add(ParseExpression(expression));
            if (literalTokens.NotEmpty() && literalTokens.Front().Type == TokenType.Comma) literalTokens.RemoveFront();
        }

        return new ArrayLiteralNode(expressions);
    }

    public virtual Node ParsePrimary(TokenList<Token> input)
    {
        if (input.Empty()) input.ThrowExpectedInput();

        switch (input.Front().Type)
        {
            case TokenType.Const:
                input.RemoveFront();
                return new ConstNode(ParseDeclaration(input));
            case TokenType.Identifier:
            {
                var front = input.RemoveFront();
                if (input.NotEmpty() && input.Front().Type is TokenType.Identifier)
                {
                    input.InsertFront(front);
                    goto case TokenType.Unknown;
                }
                return ResolveTokenToLiteralOrIdentifier(front);
            }
            case TokenType.Unknown:
            case TokenType.TypeFloat or TokenType.TypeInt or TokenType.TypeFloat2 or TokenType.TypeInt2
                or TokenType.TypeFloat3 or TokenType.TypeInt3 or TokenType.TypeFloat4 or TokenType.TypeInt4
                or TokenType.TypeMat3 or TokenType.TypeMat4:
            {
                var targetToken = input.RemoveFront();

                if (input.NotEmpty() && input.Front().Type is TokenType.Unknown or TokenType.Identifier)
                {
                    input.InsertFront(targetToken);
                    return
                        ParseDeclaration(
                            input); // Good chance it is a declaration if the format is <unknown/type> <unknown/identifier>
                }
                
                return ParseAccess(input,ResolveTokenToLiteralOrIdentifier(targetToken));
            }
            case TokenType.OpIncrement or TokenType.OpDecrement:
            {
                var op = input.RemoveFront();
                if (input.Empty()) input.ThrowExpectedInput();
                var next = ParseAccess(input);
                if (op.Type is TokenType.OpIncrement) return new IncrementNode(next, true);

                return new DecrementNode(next, true);
            }
            case TokenType.OpenParen:
            {
                var parenTokens = ConsumeTill(input, 0, TokenType.CloseParen);
                parenTokens.ExpectFront(TokenType.OpenParen).RemoveFront();
                input.ExpectFront(TokenType.CloseParen).RemoveFront();
                return new PrecedenceNode(ParseExpression(parenTokens));
            }
            case TokenType.OpenBrace:
            {
                return ParseArrayLiteral(input);
            }
            case TokenType.OpSubtract:
            {
                input.RemoveFront();
                return new NegateNode(ParsePrimary(input));
            }
            case TokenType.PushConstant:
            {
                return new IdentifierNode(input.RemoveFront().Value);
            }
            case TokenType.Discard:
            {
                input.RemoveFront();
                return new DiscardNode();
            }
            default:
                throw new Exception("Unknown Primary Token");
        }
    }

    public virtual Node ParseAccess(TokenList<Token> input,Node? initialLeft = null)
    {
        var left = initialLeft ?? ParsePrimary(input);

        while (input.NotEmpty() && (input.Front().Type is TokenType.Access or TokenType.OpenBracket ||
                                    (input.Front().Type is TokenType.OpenParen && left is IdentifierNode)))
            switch (input.Front().Type)
            {
                case TokenType.OpenParen:
                {
                    if (left is IdentifierNode id) left = new CallNode(id.Identity, ParseCallArguments(input));
                }
                    break;
                case TokenType.Access:
                {
                    input.RemoveFront();
                    left = new AccessNode(left, ParsePrimary(input));
                }
                    break;
                case TokenType.OpenBracket:
                {
                    input.RemoveFront();
                    var within = ConsumeTill(input, 0, TokenType.CloseBracket);
                                                                                                input.ExpectFront(TokenType.CloseBracket).RemoveFront();
                    left = new IndexNode(left, ParseExpression(within));
                }
                    break;
            }

        return left;
    }

    public virtual Node ParseMultiplicative(TokenList<Token> input)
    {
        var left = ParseAccess(input);
        while (input.NotEmpty() && input.Front().Type is TokenType.OpMultiply or TokenType.OpDivide or TokenType.OpMod)
        {
            var token = input.RemoveFront();
            var right = ParseAccess(input);
            left = new BinaryOpNode(left, right, token.Type);
        }

        return left;
    }

    public virtual Node ParseAdditive(TokenList<Token> input)
    {
        var left = ParseMultiplicative(input);
        while (input.NotEmpty() && input.Front().Type is TokenType.OpAdd or TokenType.OpSubtract)
        {
            var token = input.RemoveFront();
            var right = ParseMultiplicative(input);
            left = new BinaryOpNode(left, right, token.Type);
        }

        return left;
    }


    public virtual Node ParseComparison(TokenList<Token> input)
    {
        var left = ParseAdditive(input);
        while (input.NotEmpty() && input.Front().Type is TokenType.OpEqual or TokenType.OpNotEqual or TokenType.OpLess
                   or TokenType.OpLessEqual or TokenType.OpGreater or TokenType.OpGreaterEqual)
        {
            var token = input.RemoveFront();
            var right = ParseAdditive(input);
            left = new BinaryOpNode(left, right, token.Type);
        }

        return left;
    }


    public virtual Node ParseLogical(TokenList<Token> input)
    {
        var left = ParseComparison(input);
        while (input.NotEmpty() && input.Front().Type is TokenType.OpAnd or TokenType.OpOr or TokenType.OpNot)
        {
            var token = input.RemoveFront();
            var right = ParseComparison(input);
            left = new BinaryOpNode(left, right, token.Type);
        }

        return left;
    }

    public virtual Node ParseIncrementDecrement(TokenList<Token> input)
    {
        var left = ParseLogical(input);
        if (input.NotEmpty() && left is IdentifierNode or AccessNode &&
            input.Front().Type is TokenType.OpIncrement or TokenType.OpDecrement)
        {
            var op = input.RemoveFront();
            if (op.Type is TokenType.OpIncrement) return new IncrementNode(left, false);

            return new DecrementNode(left, false);
        }

        return left;
    }
    
    

    public virtual Node ParseConditional(TokenList<Token> input)
    {
        var left = ParseIncrementDecrement(input);
        if (input.NotEmpty() && input.Front().Type == TokenType.Conditional)
        {
            input.RemoveFront();
            var condition = left;
            var leftTokens = ConsumeTill(input, 0, TokenType.Colon);
            input.ExpectFront(TokenType.Colon).RemoveFront();
            return new ConditionalNode(condition, ParseExpression(leftTokens), ParseExpression(input));
        }

        return left;
    }
    
    public virtual Node ParseAssignment(TokenList<Token> input)
    {
        var left = ParseConditional(input);
        while (input.NotEmpty() && input.Front().Type is TokenType.Assign or TokenType.OpAddAssign or TokenType.OpDivideAssign or TokenType.OpMultiplyAssign or TokenType.OpSubtractAssign)
        {
            var tok = input.RemoveFront();
            if (tok.Type == TokenType.Assign)
            {
                left = new AssignNode(left, ParseConditional(input));
            }
            else
            {
                left = new BinaryOpAndAssignNode(left, ParseConditional(input), tok.Type switch
                {
                    TokenType.OpAddAssign => EBinaryOp.Add,
                    TokenType.OpDivideAssign => EBinaryOp.Divide,
                    TokenType.OpMultiplyAssign => EBinaryOp.Multiply,
                    TokenType.OpSubtractAssign => EBinaryOp.Subtract,
                    _ => throw new ArgumentOutOfRangeException()
                });
            }
        }

        return left;
    }

    public virtual Node ParseExpression(TokenList<Token> input)
    {
        return ParseAssignment(input);
    }

    public virtual Node ParseStatement(TokenList<Token> input)
    {
        var statementTokens = ConsumeTill(input, 0, TokenType.StatementEnd);
        input.ExpectFront(TokenType.StatementEnd).RemoveFront();

        switch (statementTokens.Front().Type)
        {
            case TokenType.Return:
            {
                statementTokens.RemoveFront();

                return new ReturnNode(ParseExpression(statementTokens));
            }
            default:
                return ParseExpression(statementTokens);
        }
    }

    public virtual DeclarationNode ParseDeclaration(TokenList<Token> input)
    {
        var type = input.RemoveFront();
        if (input.Front().Type == TokenType.OpenBrace)
        {
            var structBlock = ParseStructScope(input);
            var identifier = input.ExpectFront(TokenType.Identifier).RemoveFront();
            var count = input.ExpectFront(TokenType.DeclarationCount).RemoveFront();
            if (type.Type == TokenType.TypeBuffer)
            {
                return new BufferDeclarationNode(identifier.Value, int.Parse(count.Value), structBlock);
            }

            return new BlockDeclarationNode(type.Value,identifier.Value, int.Parse(count.Value), structBlock);
        }
        else
        {
            var identifier = input.ExpectFront(TokenType.Identifier).RemoveFront();
            var count = input.ExpectFront(TokenType.DeclarationCount).RemoveFront();
            if (type.Type == TokenType.Identifier)
                return new StructDeclarationNode(identifier.Value, int.Parse(count.Value), type.Value);
            return new DeclarationNode(
                Utils.TokenTypeToDeclarationType(type.Type) ?? throw input.CreateException("Unexpected token", type),
                identifier.Value, int.Parse(count.Value));
        }
    }

    public virtual List<DeclarationNode> ParseStructScope(TokenList<Token> input)
    {
        List<DeclarationNode> declarations = new();
        input.ExpectFront(TokenType.OpenBrace).RemoveFront();
        while (input.Front() is not { Type: TokenType.CloseBrace })
        {
            declarations.Add(ParseDeclaration(input));
            input.ExpectFront(TokenType.StatementEnd).RemoveFront();
        }

        input.ExpectFront(TokenType.CloseBrace).RemoveFront();
        return declarations;
    }

    public virtual StructNode ParseStruct(TokenList<Token> input)
    {
        input.ExpectFront(TokenType.TypeStruct).RemoveFront();
        var structIdentifier = input.ExpectFront(TokenType.Identifier).RemoveFront();

        return new StructNode(structIdentifier.Value, ParseStructScope(input));
    }


    public virtual LayoutNode ParseLayout(TokenList<Token> input)
    {
        input.ExpectFront(TokenType.Layout).RemoveFront();
        input.ExpectFront(TokenType.OpenParen).RemoveFront();

        var tags = new Dictionary<string, string>();

        while (input.Front().Type != TokenType.CloseParen)
            tags.Add(input.ExpectFront(TokenType.Identifier).RemoveFront().Value,
                input.ExpectFront(TokenType.Identifier).RemoveFront().Value);

        input.ExpectFront(TokenType.CloseParen).RemoveFront();

        var layoutType = input.RemoveFront().Type switch
        {
            TokenType.DataIn => ELayoutType.In,
            TokenType.DataOut => ELayoutType.Out,
            TokenType.Uniform => ELayoutType.Uniform,
            TokenType.ReadOnly => ELayoutType.ReadOnly,
            _ => throw new ArgumentOutOfRangeException()
        };

        var declaration = ParseDeclaration(input);
        input.ExpectFront(TokenType.StatementEnd).RemoveFront();

        return new LayoutNode(tags, layoutType, declaration);
    }
    
    public virtual DefineNode ParseDefine(TokenList<Token> input)
    {
        input.ExpectFront(TokenType.Define).RemoveFront();
        var id = input.ExpectFront(TokenType.Identifier).RemoveFront();
        var expr = ParseExpression(input);
        return new DefineNode(id.Value, expr);
    }

    public virtual PushConstantNode ParsePushConstant(TokenList<Token> input)
    {
        input.ExpectFront(TokenType.PushConstant).RemoveFront();
        input.ExpectFront(TokenType.OpenParen).RemoveFront();

        var tags = new Dictionary<string, string>();

        while (input.Front().Type != TokenType.CloseParen)
            tags.Add(input.ExpectFront(TokenType.Identifier).RemoveFront().Value,
                input.ExpectFront(TokenType.Identifier).RemoveFront().Value);

        input.ExpectFront(TokenType.CloseParen).RemoveFront();

        var push = new PushConstantNode("push", new StructNode("", ParseStructScope(input)), tags);
        input.ExpectFront(TokenType.StatementEnd).RemoveFront();
        return push;
    }


    public virtual NamedScopeNode ParseNamedScope(TokenList<Token> input)
    {
        var scopeType = input.ExpectFront(TokenType.VertexScope, TokenType.FragmentScope).RemoveFront().Type;
        input.ExpectFront(TokenType.OpenBrace).RemoveFront();

        List<Node> statements = new();
        while (input.Front() is not { Type: TokenType.CloseBrace })
            switch (input.Front().Type)
            {
                case TokenType.Layout:
                    statements.Add(ParseLayout(input));
                    break;
                case TokenType.Define:
                    statements.Add(ParseDefine(input));
                    break;
                case TokenType.PushConstant:
                    statements.Add(ParsePushConstant(input));
                    break;
                case TokenType.Function:
                    statements.Add(ParseFunction(input));
                    break;
                case TokenType.Const:
                {
                    statements.Add(ParseStatement(input));
                }
                    break;
                case TokenType.Include:
                {
                    statements.Add(ParseInclude(input));
                }
                    break;
                case TokenType.TypeStruct:
                {
                    statements.Add(ParseStruct(input));
                }
                    break;
                default:
                    throw input.CreateException("Unexpected Token", input.Front());
            }

        input.RemoveFront();

        return new NamedScopeNode(scopeType, statements);
    }

    public virtual ForNode ParseFor(TokenList<Token> input)
    {
        input.ExpectFront(TokenType.For).RemoveFront();
        input.ExpectFront(TokenType.OpenParen).RemoveFront();
        var predicate = ConsumeTill(input, 0, TokenType.StatementEnd);
        input.ExpectFront(TokenType.StatementEnd).RemoveFront();
        var condition = ConsumeTill(input, 0, TokenType.StatementEnd);
        input.ExpectFront(TokenType.StatementEnd).RemoveFront();
        var after = ConsumeTill(input, 0, TokenType.StatementEnd);
        input.ExpectFront(TokenType.StatementEnd).RemoveFront();

        var initialNode = predicate.Empty() ? new NoOpNode() : ParseExpression(predicate);
        var conditionNode = condition.Empty() ? new NoOpNode() : ParseExpression(condition);
        var updateNode = after.Empty() ? new NoOpNode() : ParseExpression(after);

        return new ForNode(initialNode, conditionNode, updateNode, ParseScope(input));
    }

    public virtual Node ParseIf(TokenList<Token> input)
    {
        input.ExpectFront(TokenType.If).RemoveFront();
        input.ExpectFront(TokenType.OpenParen).RemoveFront();
        var conditionTokens = ConsumeTill(input, 1, TokenType.CloseParen);
        input.ExpectFront(TokenType.CloseParen).RemoveFront();
        var condition = ParseExpression(conditionTokens);
        var scope = ParseScope(input);
        if (input.NotEmpty() && input.Front().Type == TokenType.Else)
        {
            input.RemoveFront();
            return input.Front().Type == TokenType.If ? new IfNode(condition, scope, ParseIf(input)) : new IfNode(condition, scope, ParseScope(input));
        }
        else
        {
            return new IfNode(condition, scope, new NoOpNode());
        }
    }
    public virtual ScopeNode ParseScope(TokenList<Token> input)
    {
        input.ExpectFront(TokenType.OpenBrace).RemoveFront();
        List<Node> statements = new();
        while (input.Front() is not { Type: TokenType.CloseBrace })
            switch (input.Front().Type)
            {
                case TokenType.OpenParen:
                    statements.Add(ParseScope(input));
                    break;
                case TokenType.For:
                    statements.Add(ParseFor(input));
                    break;
                case TokenType.If:
                    statements.Add(ParseIf(input));
                    break;
                default:
                    statements.Add(ParseStatement(input));
                    break;
            }

        input.RemoveFront();

        return new ScopeNode(statements);
    }

    public virtual FunctionNode ParseFunction(TokenList<Token> input)
    {
        var fnToken = input.ExpectFront(TokenType.Function).RemoveFront();
        var returnTokenType = Token.KeywordToTokenType(fnToken.Value) ?? TokenType.Identifier;
        var returnCount = input.ExpectFront(TokenType.DeclarationCount).RemoveFront();
        var fnName = input.ExpectFront(TokenType.Identifier).RemoveFront();
        input.ExpectFront(TokenType.OpenParen).RemoveFront();
        List<FunctionArgumentNode> arguments = new();

        while (input.Front() is not { Type: TokenType.CloseParen })
        {
            var isInput = true;
            if (input.Front().Type is TokenType.DataIn or TokenType.DataOut)
            {
                isInput = input.Front().Type == TokenType.DataIn;
                input.RemoveFront();
            }

            arguments.Add(new FunctionArgumentNode(isInput, ParseDeclaration(input)));
        }

        input.ExpectFront(TokenType.CloseParen).RemoveFront();

        var returnCountInt = int.Parse(returnCount.Value);

        var returnDeclaration = Utils.TokenTypeToDeclarationType(returnTokenType) is { } declType
            ? new DeclarationNode(declType, "", returnCountInt)
            : new StructDeclarationNode("", int.Parse(returnCount.Value), fnToken.Value);

        return new FunctionNode(fnName.Value, returnDeclaration, arguments, ParseScope(input));
    }

    public virtual IncludeNode ParseInclude(TokenList<Token> input)
    {
        var includeTok = input.ExpectFront(TokenType.Include).RemoveFront();
        var identifier = input.ExpectFront(TokenType.Identifier).RemoveFront();
        return new IncludeNode(includeTok.DebugInfo.File, identifier.Value);
    }

    public virtual ModuleNode Run(TokenList<Token> input)
    {
        if (input.Empty()) return new ModuleNode("", []);
        List<Node> statements = new();
        var filePath = input.Front().DebugInfo.File;
        while (input.NotEmpty())
            switch (input.Front().Type)
            {
                case TokenType.Include:
                {
                    statements.Add(ParseInclude(input));
                }
                    break;
                case TokenType.Define:
                    statements.Add(ParseDefine(input));
                    break;
                case TokenType.TypeStruct:
                {
                    statements.Add(ParseStruct(input));
                }
                    break;
                case TokenType.VertexScope or TokenType.FragmentScope:
                {
                    statements.Add(ParseNamedScope(input));
                }
                    break;
                case TokenType.Const:
                {
                    statements.Add(ParseStatement(input));
                }
                    break;
                case TokenType.Layout:
                {
                    statements.Add(ParseLayout(input));
                }
                    break;
                case TokenType.PushConstant:
                    statements.Add(ParsePushConstant(input));
                    break;
                case TokenType.Function:
                {
                    statements.Add(ParseFunction(input));
                }
                    break;
                default:
                    throw input.CreateException("Unexpected token", input.Front());
            }

        return new ModuleNode(filePath, statements);
    }
}