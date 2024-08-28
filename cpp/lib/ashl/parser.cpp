#include "ashl/parser.hpp"

#include "ashl/utils.hpp"

namespace ashl
{
    TokenList consumeTokensTill(TokenList& input, const std::set<ETokenType>& targets, const int& initialScope,
                                const bool& includeTarget)
    {
        TokenList result{};
        auto scope = initialScope;
        while (input.NotEmpty())
        {
            auto frontToken = input.Front();

            switch (frontToken.type)
            {
            case ETokenType::OpenBrace:
            case ETokenType::OpenParen:
            case ETokenType::OpenBracket:
                scope++;
                break;

            case ETokenType::CloseBrace:
            case ETokenType::CloseParen:
            case ETokenType::CloseBracket:
                scope--;
                break;
            }

            if (targets.contains(frontToken.type) && scope == 0)
            {
                if (includeTarget)
                {
                    result.InsertBack(input.RemoveFront());
                }
                return result;
            }

            result.InsertBack(input.RemoveFront());
        }

        return result;
    }

    std::shared_ptr<Node> resolveTokenToLiteralOrIdentifier(const Token& input)
    {
        if (isInteger(input.value))
        {
            return std::make_shared<IntegerLiteralNode>(parseInt(input.value));
        }

        if (isBoolean(input.value))
        {
            return std::make_shared<BooleanLiteralNode>(parseBoolean(input.value));
        }

        if (isFloat(input.value))
        {
            return std::make_shared<FloatLiteralNode>(parseFloat(input.value));
        }

        return std::make_shared<IdentifierNode>(input.value);
    }

    std::shared_ptr<ArrayLiteralNode> parseArrayLiteral(TokenList& input)
    {
        input.ExpectFront(ETokenType::OpenBrace).RemoveFront();

        auto allItemsTokens = consumeTokensTill(input, setOf(ETokenType::CloseBrace), 1);

        input.ExpectFront(ETokenType::CloseBrace).RemoveFront();

        std::vector<std::shared_ptr<Node>> nodes{};

        while (allItemsTokens.NotEmpty())
        {
            auto itemTokens = consumeTokensTill(allItemsTokens, setOf(ETokenType::Comma));

            if (allItemsTokens.NotEmpty())
            {
                allItemsTokens.ExpectFront(ETokenType::Comma).RemoveFront();
            }

            nodes.push_back(parseExpression(itemTokens));
        }

        return std::make_shared<ArrayLiteralNode>(nodes);
    }

    std::shared_ptr<Node> parsePrimary(TokenList& input)
    {
        switch (input.Front().type)
        {
        case ETokenType::Const:
            {
                const auto a = input.RemoveFront();
                return std::make_shared<ConstNode>(parseDeclaration(input));
            }
        case ETokenType::Numeric:
            return resolveTokenToLiteralOrIdentifier(input.RemoveFront());
        case ETokenType::Identifier:
        case ETokenType::Unknown:
            {
                const auto front = input.RemoveFront();
                if (input.NotEmpty() && input.Front().type == ETokenType::Unknown)
                {
                    input.InsertFront(front);
                    return parseDeclaration(input);
                }
                return resolveTokenToLiteralOrIdentifier(front);
            }
        case ETokenType::TypeFloat:
        case ETokenType::TypeInt:
        case ETokenType::TypeFloat2:
        case ETokenType::TypeInt2:
        case ETokenType::TypeFloat3:
        case ETokenType::TypeInt3:
        case ETokenType::TypeFloat4:
        case ETokenType::TypeInt4:
        case ETokenType::TypeMat3:
        case ETokenType::TypeMat4:
            {
                const auto targetToken = input.RemoveFront();
                if (input.NotEmpty() && (input.Front().type == ETokenType::Identifier || input.Front().type ==
                    ETokenType::Unknown))
                {
                    input.InsertFront(targetToken);
                    return parseDeclaration(input);
                }
                return parseAccessors(input, resolveTokenToLiteralOrIdentifier(targetToken));
            }
        case ETokenType::OpIncrement:
        case ETokenType::OpDecrement:
            {
                const auto op = input.RemoveFront();
                const auto next = parseAccessors(input);
                if (op.type == ETokenType::OpIncrement)
                {
                    return std::make_shared<IncrementNode>(true, next);
                }
                return std::make_shared<IncrementNode>(true, next);
            }
        case ETokenType::OpenParen:
            {
                auto parenTokens = consumeTokensTill(input, setOf(ETokenType::CloseParen));

                parenTokens.RemoveFront();

                input.ExpectFront(ETokenType::CloseParen).RemoveFront();

                return std::make_shared<PrecedenceNode>(parseExpression(parenTokens));
            }
        case ETokenType::OpenBrace:
            return parseArrayLiteral(input);
        case ETokenType::OpSubtract:
            {
                input.RemoveFront();
                return std::make_shared<NegateNode>(parsePrimary(input));
            }
        case ETokenType::PushConstant:
            {
                auto tok = input.RemoveFront();
                return std::make_shared<IdentifierNode>(tok.value);
            }
        case ETokenType::Discard:
            return std::make_shared<DiscardNode>();
        default:
            throw std::exception("Unknown Primary Token");
        }
    }

    std::shared_ptr<Node> parseAccessors(TokenList& input, const std::shared_ptr<Node>& initialLeft)
    {
        auto left = initialLeft ? initialLeft : parsePrimary(input);

        while (input.NotEmpty() && (input.Front().type == ETokenType::OpenParen || input.Front().type ==
            ETokenType::Access || input.Front().type == ETokenType::OpenBracket))
        {
            switch (input.Front().type)
            {
            case ETokenType::OpenParen:
                {
                    if (left->nodeType == ENodeType::Identifier)
                    {
                        auto identifier = std::dynamic_pointer_cast<IdentifierNode>(left);
                        input.RemoveFront();
                        auto allArgsTokens = consumeTokensTill(input, setOf(ETokenType::CloseParen), 1);
                        input.ExpectFront(ETokenType::CloseParen).RemoveFront();

                        std::vector<std::shared_ptr<Node>> args{};

                        while (allArgsTokens.NotEmpty())
                        {
                            auto argsTokens = consumeTokensTill(allArgsTokens, setOf(ETokenType::Comma));

                            if (allArgsTokens.NotEmpty())
                            {
                                allArgsTokens.ExpectFront(ETokenType::Comma).RemoveFront();
                            }

                            args.push_back(parseExpression(argsTokens));
                        }
                        left = std::make_shared<CallNode>(identifier, args);
                    }
                    else
                    {
                        return left;
                    }
                }
                break;
            case ETokenType::Access:
                {
                    auto token = input.RemoveFront();
                    auto right = parsePrimary(input);
                    left = std::make_shared<AccessNode>(left, right);
                }
                break;
            case ETokenType::OpenBracket:
                {
                    auto token = input.RemoveFront();
                    auto exprTokens = consumeTokensTill(input, setOf(ETokenType::CloseBracket), 1);
                    left = std::make_shared<IndexNode>(left, parseExpression(exprTokens));
                }
            }
        }

        return left;
    }

    std::shared_ptr<Node> parseMultiplicativeExpression(TokenList& input)
    {
        auto left = parseAccessors(input);

        while (input.NotEmpty() && (input.Front().type == ETokenType::OpDivide || input.Front().type ==
            ETokenType::OpMultiply))
        {
            auto token = input.RemoveFront();
            auto right = parseAccessors(input);
            left = std::make_shared<BinaryOpNode>(left, right, token.type);
        }

        return left;
    }

    std::shared_ptr<Node> parseAdditiveExpression(TokenList& input)
    {
        auto left = parseMultiplicativeExpression(input);

        while (input.NotEmpty() && (input.Front().type == ETokenType::OpAdd || input.Front().type ==
            ETokenType::OpSubtract))
        {
            auto token = input.RemoveFront();
            auto right = parseMultiplicativeExpression(input);
            left = std::make_shared<BinaryOpNode>(left, right, token.type);
        }

        return left;
    }

    

    std::shared_ptr<Node> parseComparisonExpression(TokenList& input)
    {
        auto left = parseAdditiveExpression(input);

        while (input.NotEmpty() && (input.Front().type == ETokenType::OpEqual || input.Front().type ==
            ETokenType::OpNotEqual ||
            input.Front().type == ETokenType::OpLess || input.Front().type == ETokenType::OpLessEqual || input.Front().
            type ==
            ETokenType::OpGreater || input.Front().type == ETokenType::OpGreaterEqual))
        {
            auto token = input.RemoveFront();
            auto right = parseAdditiveExpression(input);
            left = std::make_shared<BinaryOpNode>(left, right, token.type);
        }

        return left;
    }

    std::shared_ptr<Node> parseLogicalExpression(TokenList& input)
    {
        auto left = parseComparisonExpression(input);

        while (input.NotEmpty() && (input.Front().type == ETokenType::OpAnd || input.Front().type == ETokenType::OpOr ||
            input.Front().type == ETokenType::OpNot))
        {
            auto token = input.RemoveFront();
            auto right = parseComparisonExpression(input);
            left = std::make_shared<BinaryOpNode>(left, right, token.type);
        }

        return left;
    }

    std::shared_ptr<Node> parseConditionalExpression(TokenList& input)
    {
        auto left = parseLogicalExpression(input);

        while (input.NotEmpty() && (input.Front().type == ETokenType::Conditional))
        {
            auto token = input.RemoveFront();
            auto leftTokens = consumeTokensTill(input,std::set{ETokenType::Colon});
            left = std::make_shared<ConditionalNode>(left, parseExpression(leftTokens), parseExpression(input));
        }

        return left;
    }

    std::shared_ptr<Node> parseAssignmentExpression(TokenList& input)
    {
        auto left = parseLogicalExpression(input);

        while (input.NotEmpty() && input.Front().type == ETokenType::Assign)
        {
            auto token = input.RemoveFront();
            auto right = parseLogicalExpression(input);
            left = std::make_shared<AssignNode>(left, right);
        }

        return left;
    }

    std::shared_ptr<Node> parseExpression(TokenList& input)
    {
        return parseAssignmentExpression(input);
    }

    std::vector<std::shared_ptr<DeclarationNode>> parseStructScope(TokenList& input)
    {
        std::vector<std::shared_ptr<DeclarationNode>> result{};
        input.ExpectFront(ETokenType::OpenBrace).RemoveFront();
        while (input.Front().type != ETokenType::CloseBrace)
        {
            auto declarationTokens = consumeTokensTill(input, setOf(ETokenType::StatementEnd));
            input.ExpectFront(ETokenType::StatementEnd).RemoveFront();
            result.push_back(parseDeclaration(declarationTokens));
        }

        input.RemoveFront();

        return result;
    }

    std::shared_ptr<StructNode> parseStruct(TokenList& input)
    {
        input.ExpectFront(ETokenType::TypeStruct).RemoveFront();
        auto name = input.RemoveFront();
        auto declarations = parseStructScope(input);
        input.ExpectFront(ETokenType::StatementEnd).RemoveFront();
        return std::make_shared<StructNode>(name.value, declarations);
    }

    std::shared_ptr<IfNode> parseIf(TokenList& input)
    {
        input.ExpectFront(ETokenType::If).RemoveFront();
        auto condition = consumeTokensTill(input, setOf(ETokenType::CloseParen));
        condition.ExpectFront(ETokenType::OpenParen).RemoveFront();

        input.ExpectFront(ETokenType::CloseParen).RemoveFront();

        auto cond = parseExpression(condition);

        auto scope = parseScope(input);

        if (input.NotEmpty() && input.Front().type == ETokenType::Else)
        {
            input.RemoveFront();
            std::shared_ptr<Node> elseScope;
            if (input.Front().type == ETokenType::If)
                elseScope = parseIf(input);
            else
                elseScope = parseScope(input);

            return std::make_shared<IfNode>(cond, scope, elseScope);
        }

        return std::make_shared<IfNode>(cond, scope);
    }

    std::shared_ptr<ForNode> parseFor(TokenList& input)
    {
        input.ExpectFront(ETokenType::For).RemoveFront();

        auto withinParen = consumeTokensTill(input, setOf(ETokenType::CloseParen));

        withinParen.ExpectFront(ETokenType::OpenParen).RemoveFront();

        input.ExpectFront(ETokenType::CloseParen).RemoveBack();

        auto initTokens = consumeTokensTill(withinParen, setOf(ETokenType::Colon));

        withinParen.ExpectFront(ETokenType::Colon).RemoveFront();

        auto condTokens = consumeTokensTill(withinParen, setOf(ETokenType::Colon));

        withinParen.ExpectFront(ETokenType::Colon).RemoveFront();

        auto updateTokens = withinParen;

        auto noop = std::make_shared<NoOpNode>();

        return std::make_shared<ForNode>(
            initTokens.Empty() ? noop : parseExpression(initTokens),
            condTokens.Empty() ? noop : parseExpression(condTokens),
            updateTokens.Empty() ? noop : parseExpression(updateTokens),
            parseScope(input)
        );
    }

    std::shared_ptr<LayoutNode> parseLayout(TokenList& input)
    {
        input.ExpectFront(ETokenType::Layout).RemoveFront();

        input.ExpectFront(ETokenType::OpenParen).RemoveFront();

        auto tagTokens = consumeTokensTill(input, setOf(ETokenType::CloseParen), 1);

        input.ExpectFront(ETokenType::CloseParen).RemoveFront();
        std::unordered_map<std::string, std::string> tags{};
        while (tagTokens.NotEmpty())
        {
            auto id = tagTokens.RemoveFront();
            if (tagTokens.NotEmpty() && tagTokens.Front().type == ETokenType::Assign)
            {
                tagTokens.RemoveFront();
                auto val = tagTokens.RemoveFront();
                tags.emplace(id.value, val.value);
            }
            else
            {
                tags.emplace(id.value, "");
            }

            if (tagTokens.NotEmpty() && tagTokens.Front().type == ETokenType::Comma)
            {
                tagTokens.RemoveFront();
            }
        }

        const auto layoutTypeToken = input.RemoveFront();

        ELayoutType layoutType;

        switch (layoutTypeToken.type)
        {
        case ETokenType::DataIn:
            layoutType = ELayoutType::Input;
            break;
        case ETokenType::DataOut:
            layoutType = ELayoutType::Output;
            break;
        case ETokenType::ReadOnly:
            layoutType = ELayoutType::Readonly;
            break;
        case ETokenType::Uniform:
            layoutType = ELayoutType::Uniform;
            break;
        default:
            layoutType = ELayoutType::Input;
            break;
        }

        auto declaration = parseDeclaration(input);

        input.ExpectFront(ETokenType::StatementEnd).RemoveFront();

        return std::make_shared<LayoutNode>(layoutType, declaration, tags);
    }

    std::shared_ptr<PushConstantNode> parsePushConstant(TokenList& input)
    {
        input.ExpectFront(ETokenType::PushConstant).RemoveFront();

        input.ExpectFront(ETokenType::OpenParen).RemoveFront();

        auto tagTokens = consumeTokensTill(input, setOf(ETokenType::CloseParen), 1);

        input.ExpectFront(ETokenType::CloseParen).RemoveFront();
        std::unordered_map<std::string, std::string> tags{};
        while (tagTokens.NotEmpty())
        {
            auto id = tagTokens.RemoveFront();
            if (tagTokens.NotEmpty() && tagTokens.Front().type == ETokenType::Assign)
            {
                tagTokens.RemoveFront();
                auto val = tagTokens.RemoveFront();
                tags.emplace(id.value, val.value);
            }
            else
            {
                tags.emplace(id.value, "");
            }

            if (tagTokens.NotEmpty() && tagTokens.Front().type == ETokenType::Comma)
            {
                tagTokens.RemoveFront();
            }
        }
        
        auto declarations = parseStructScope(input);

        input.ExpectFront(ETokenType::StatementEnd).RemoveFront();

        return std::make_shared<PushConstantNode>(declarations,tags);
    }

    std::shared_ptr<DefineNode> parseDefine(TokenList& input)
    {
        input.ExpectFront(ETokenType::Define).RemoveFront();
        auto identifier = input.RemoveFront();
        auto expr = consumeTokensTill(input, setOf(ETokenType::StatementEnd));
        input.ExpectFront(ETokenType::StatementEnd).RemoveFront();
        return std::make_shared<DefineNode>(identifier.value,parseExpression(expr));
    }

    std::shared_ptr<IncludeNode> parseInclude(TokenList& input)
    {
        input.ExpectFront(ETokenType::Include).RemoveFront();
        auto token = input.ExpectFront(ETokenType::StringLiteral).RemoveFront();
        return std::make_shared<IncludeNode>(token.debugInfo.file, token.value);
    }

    std::shared_ptr<ScopeNode> parseScope(TokenList& input)
    {
        std::vector<std::shared_ptr<Node>> statements{};

        input.ExpectFront(ETokenType::OpenBrace).RemoveFront();

        while (input.Front().type != ETokenType::CloseBrace)
        {
            switch (input.Front().type)
            {
            case ETokenType::If:
                {
                    statements.push_back(parseIf(input));
                }
                break;
            case ETokenType::For:
                {
                    statements.push_back(parseFor(input));
                }
                break;
            case ETokenType::Return:
                {
                    input.RemoveFront();
                    auto statementTokens = consumeTokensTill(input, setOf(ETokenType::StatementEnd));
                    input.ExpectFront(ETokenType::StatementEnd).RemoveFront();

                    statements.push_back(std::make_shared<ReturnNode>(parseExpression(statementTokens)));
                }
                break;
            default:
                {
                    auto statementTokens = consumeTokensTill(input, setOf(ETokenType::StatementEnd));

                    input.ExpectFront(ETokenType::StatementEnd).RemoveFront();

                    statements.push_back(parseExpression(statementTokens));
                }
            }
        }

        input.RemoveFront();

        return std::make_shared<ScopeNode>(statements);
    }

    std::shared_ptr<NamedScopeNode> parseNamedScope(TokenList& input)
    {
        std::vector<std::shared_ptr<Node>> statements{};

        auto scopeTypeToken = input.RemoveFront();

        input.ExpectFront(ETokenType::OpenBrace).RemoveFront();

        while (input.Front().type != ETokenType::CloseBrace)
        {
            switch (input.Front().type)
            {
            case ETokenType::Include:
                statements.push_back(parseInclude(input));
                break;
            case ETokenType::Define:
                statements.push_back(parseDefine(input));
                break;
            case ETokenType::Layout:
                statements.push_back(parseLayout(input));
                break;
            case ETokenType::TypeStruct:
                statements.push_back(parseStruct(input));
                break;
            case ETokenType::Const:
                {
                    auto tokens = consumeTokensTill(input, setOf(ETokenType::StatementEnd));
                    input.ExpectFront(ETokenType::StatementEnd).RemoveFront();
                    statements.push_back(parseExpression(tokens));
                }
                break;
            case ETokenType::PushConstant:
                {
                    statements.push_back(parsePushConstant(input));
                }
                break;
            case ETokenType::TypeVoid:
            case ETokenType::TypeFloat:
            case ETokenType::TypeFloat2:
            case ETokenType::TypeFloat3:
            case ETokenType::TypeFloat4:
            case ETokenType::TypeInt:
            case ETokenType::TypeInt2:
            case ETokenType::TypeInt3:
            case ETokenType::TypeInt4:
            case ETokenType::TypeBoolean:
            case ETokenType::TypeMat3:
            case ETokenType::TypeMat4:
            case ETokenType::Unknown:
                statements.push_back(parseFunction(input));
                break;
            default:
                {
                    auto statementTokens = consumeTokensTill(input, setOf(ETokenType::StatementEnd));

                    input.ExpectFront(ETokenType::StatementEnd).RemoveFront();

                    statements.push_back(parseExpression(statementTokens));
                }
            }
        }

        input.RemoveFront();

        EScopeType scopeType;

        switch (scopeTypeToken.type)
        {
        case ETokenType::FragmentScope:
            scopeType = EScopeType::Fragment;
            break;
        case ETokenType::VertexScope:
            scopeType = EScopeType::Vertex;
            break;
        default:
            scopeType = EScopeType::Fragment;
        }

        return std::make_shared<NamedScopeNode>(scopeType, std::make_shared<ScopeNode>(statements));
    }

    std::shared_ptr<DeclarationNode> parseDeclaration(TokenList& input)
    {
        auto type = input.RemoveFront();

        if(type.type == ETokenType::TypeBuffer)
        {
            auto name = input.ExpectFront(ETokenType::Unknown).RemoveFront();
            auto declarations = parseStructScope(input);
            return std::make_shared<BufferDeclarationNode>(name.value, 0, declarations);
        }
        
        if (type.type == ETokenType::Unknown && input.NotEmpty() && input.Front().type == ETokenType::OpenBrace)
        {
            auto name = type;
            auto declarations = parseStructScope(input);
            return std::make_shared<BlockDeclarationNode>(name.value, 0, declarations); 
        }

        auto name = input.Front().type == ETokenType::Unknown ? input.ExpectFront(ETokenType::Unknown).RemoveFront().value : "";

        auto returnCount = 1;

        if (input.NotEmpty() && input.Front().type == ETokenType::OpenBracket)
        {
            input.RemoveFront();
            returnCount = input.Front().type == ETokenType::Numeric ? parseInt(input.RemoveFront().value) : -1;
            input.ExpectFront(ETokenType::CloseBracket).RemoveFront();
        }

        return type.type == ETokenType::Unknown ? std::make_shared<StructDeclarationNode>(type.value, name, returnCount) : std::make_shared<DeclarationNode>(type, name, returnCount);
    }

    std::shared_ptr<FunctionArgumentNode> parseFunctionArgument(TokenList& input)
    {
        bool isInput = true;
        if (input.Front().type == ETokenType::DataIn || input.Front().type == ETokenType::DataOut)
        {
            const auto token = input.RemoveFront();
            isInput = token.type == ETokenType::DataIn;
        }

        return std::make_shared<FunctionArgumentNode>(isInput, parseDeclaration(input));
    }

    std::shared_ptr<FunctionNode> parseFunction(TokenList& input)
    {
        auto type = input.RemoveFront();
        auto returnCount = 0;

        if (input.Front().type == ETokenType::OpenBracket)
        {
            input.RemoveFront();
            returnCount = input.Front().type == ETokenType::Numeric ? parseInt(input.ExpectFront(ETokenType::Numeric).RemoveFront().value) : -1;
            input.ExpectFront(ETokenType::CloseBracket).RemoveFront();
        }

        auto name = input.ExpectFront(ETokenType::Unknown).RemoveFront();

        input.ExpectFront(ETokenType::OpenParen).RemoveFront();
        auto allArgsTokens = consumeTokensTill(input, setOf(ETokenType::CloseParen), 1);
        input.ExpectFront(ETokenType::CloseParen).RemoveFront();

        std::vector<std::shared_ptr<FunctionArgumentNode>> args{};

        while (allArgsTokens.NotEmpty())
        {
            auto argsTokens = consumeTokensTill(allArgsTokens, setOf(ETokenType::Comma));

            if (allArgsTokens.NotEmpty())
            {
                allArgsTokens.ExpectFront(ETokenType::Comma).RemoveFront();
            }

            args.push_back(parseFunctionArgument(argsTokens));
        }

        auto returnDecl = type.type == ETokenType::Unknown ? std::make_shared<StructDeclarationNode>(type.value,"", returnCount) : std::make_shared<DeclarationNode>(type, "", returnCount);
        
        if(input.Front().type ==  ETokenType::Arrow)
        {
            input.RemoveFront();
            auto expr = consumeTokensTill(input,setOf(ETokenType::StatementEnd));
            input.ExpectFront(ETokenType::StatementEnd).RemoveFront();
            return std::make_shared<FunctionNode>(returnDecl, name.value,
                                              args, std::make_shared<ScopeNode>(std::vector<std::shared_ptr<Node>>{std::make_shared<ReturnNode>(parseExpression(expr))}));
        }
        
        return std::make_shared<FunctionNode>(returnDecl, name.value,
                                              args, parseScope(input));
    }

    std::shared_ptr<ModuleNode> parse(TokenList& input)
    {
        if (input.Empty()) return std::make_shared<ModuleNode>(std::vector<std::shared_ptr<Node>>{});

        auto file = input.Front().value;

        std::vector<std::shared_ptr<Node>> statements{};
        while (input.NotEmpty())
        {
            switch (input.Front().type)
            {
            case ETokenType::FragmentScope:
            case ETokenType::VertexScope:
                statements.push_back(parseNamedScope(input));
                break;
            case ETokenType::Include:
                statements.push_back(parseInclude(input));
                break;
            case ETokenType::Define:
                statements.push_back(parseDefine(input));
                break;
            case ETokenType::Layout:
                statements.push_back(parseLayout(input));
                break;
            case ETokenType::TypeStruct:
                statements.push_back(parseStruct(input));
                break;
            case ETokenType::Const:
                {
                    auto tokens = consumeTokensTill(input, setOf(ETokenType::StatementEnd));
                    input.ExpectFront(ETokenType::StatementEnd).RemoveFront();
                    statements.push_back(parseExpression(tokens));
                }
                break;
            case ETokenType::PushConstant:
                {
                    statements.push_back(parsePushConstant(input));
                }
                break;
            case ETokenType::TypeVoid:
            case ETokenType::TypeFloat:
            case ETokenType::TypeFloat2:
            case ETokenType::TypeFloat3:
            case ETokenType::TypeFloat4:
            case ETokenType::TypeInt:
            case ETokenType::TypeInt2:
            case ETokenType::TypeInt3:
            case ETokenType::TypeInt4:
            case ETokenType::TypeBoolean:
            case ETokenType::TypeMat3:
            case ETokenType::TypeMat4:
            case ETokenType::Unknown:
                statements.push_back(parseFunction(input));
                break;
            default:
                throw std::exception("Unexpected Token type");
            }
        }

        return std::make_shared<ModuleNode>(statements);
    }
}
