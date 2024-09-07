#include "ashl/parser.hpp"

#include "ashl/utils.hpp"

namespace ashl
{
    TokenList consumeTokensTill(TokenList& input, const std::set<TokenType>& targets, const int& initialScope,
                                const bool& includeTarget)
    {
        TokenList result{};
        auto scope = initialScope;
        while (input.NotEmpty())
        {
            auto frontToken = input.Front();

            switch (frontToken.type)
            {
            case TokenType::OpenBrace:
            case TokenType::OpenParen:
            case TokenType::OpenBracket:
                scope++;
                break;

            case TokenType::CloseBrace:
            case TokenType::CloseParen:
            case TokenType::CloseBracket:
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
        input.ExpectFront(TokenType::OpenBrace).RemoveFront();

        auto allItemsTokens = consumeTokensTill(input, setOf(TokenType::CloseBrace), 1);

        input.ExpectFront(TokenType::CloseBrace).RemoveFront();

        std::vector<std::shared_ptr<Node>> nodes{};

        while (allItemsTokens.NotEmpty())
        {
            auto itemTokens = consumeTokensTill(allItemsTokens, setOf(TokenType::Comma));

            if (allItemsTokens.NotEmpty())
            {
                allItemsTokens.ExpectFront(TokenType::Comma).RemoveFront();
            }

            nodes.push_back(parseExpression(itemTokens));
        }

        return std::make_shared<ArrayLiteralNode>(nodes);
    }

    std::shared_ptr<Node> parsePrimary(TokenList& input)
    {
        switch (input.Front().type)
        {
        case TokenType::Const:
            {
                const auto a = input.RemoveFront();
                return std::make_shared<ConstNode>(parseDeclaration(input));
            }
        case TokenType::Numeric:
            return resolveTokenToLiteralOrIdentifier(input.RemoveFront());
        case TokenType::Identifier:
        case TokenType::Unknown:
            {
                const auto front = input.RemoveFront();
                if (input.NotEmpty() && input.Front().type == TokenType::Unknown)
                {
                    input.InsertFront(front);
                    return parseDeclaration(input);
                }
                return resolveTokenToLiteralOrIdentifier(front);
            }
        case TokenType::TypeFloat:
        case TokenType::TypeInt:
        case TokenType::TypeFloat2:
        case TokenType::TypeInt2:
        case TokenType::TypeFloat3:
        case TokenType::TypeInt3:
        case TokenType::TypeFloat4:
        case TokenType::TypeInt4:
        case TokenType::TypeMat3:
        case TokenType::TypeMat4:
            {
                const auto targetToken = input.RemoveFront();
                if (input.NotEmpty() && (input.Front().type == TokenType::Identifier || input.Front().type ==
                    TokenType::Unknown))
                {
                    input.InsertFront(targetToken);
                    return parseDeclaration(input);
                }
                return parseAccessors(input, resolveTokenToLiteralOrIdentifier(targetToken));
            }
        case TokenType::OpIncrement:
        case TokenType::OpDecrement:
            {
                const auto op = input.RemoveFront();
                const auto next = parseAccessors(input);
                if (op.type == TokenType::OpIncrement)
                {
                    return std::make_shared<IncrementNode>(true, next);
                }
                return std::make_shared<IncrementNode>(true, next);
            }
        case TokenType::OpenParen:
            {
                auto parenTokens = consumeTokensTill(input, setOf(TokenType::CloseParen));

                parenTokens.RemoveFront();

                input.ExpectFront(TokenType::CloseParen).RemoveFront();

                return std::make_shared<PrecedenceNode>(parseExpression(parenTokens));
            }
        case TokenType::OpenBrace:
            return parseArrayLiteral(input);
        case TokenType::OpSubtract:
            {
                input.RemoveFront();
                return std::make_shared<NegateNode>(parsePrimary(input));
            }
        case TokenType::PushConstant:
            {
                auto tok = input.RemoveFront();
                return std::make_shared<IdentifierNode>(tok.value);
            }
        case TokenType::Discard:
            return std::make_shared<DiscardNode>();
        default:
            throw std::exception("Unknown Primary Token");
        }
    }

    std::shared_ptr<Node> parseAccessors(TokenList& input, const std::shared_ptr<Node>& initialLeft)
    {
        auto left = initialLeft ? initialLeft : parsePrimary(input);

        while (input.NotEmpty() && (input.Front().type == TokenType::OpenParen || input.Front().type ==
            TokenType::Access || input.Front().type == TokenType::OpenBracket))
        {
            switch (input.Front().type)
            {
            case TokenType::OpenParen:
                {
                    if (left->nodeType == NodeType::Identifier)
                    {
                        auto identifier = std::dynamic_pointer_cast<IdentifierNode>(left);
                        input.RemoveFront();
                        auto allArgsTokens = consumeTokensTill(input, setOf(TokenType::CloseParen), 1);
                        input.ExpectFront(TokenType::CloseParen).RemoveFront();

                        std::vector<std::shared_ptr<Node>> args{};

                        while (allArgsTokens.NotEmpty())
                        {
                            auto argsTokens = consumeTokensTill(allArgsTokens, setOf(TokenType::Comma));

                            if (allArgsTokens.NotEmpty())
                            {
                                allArgsTokens.ExpectFront(TokenType::Comma).RemoveFront();
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
            case TokenType::Access:
                {
                    auto token = input.RemoveFront();
                    auto right = parsePrimary(input);
                    left = std::make_shared<AccessNode>(left, right);
                }
                break;
            case TokenType::OpenBracket:
                {
                    auto token = input.RemoveFront();
                    auto exprTokens = consumeTokensTill(input, setOf(TokenType::CloseBracket), 1);
                    left = std::make_shared<IndexNode>(left, parseExpression(exprTokens));
                }
            }
        }

        return left;
    }

    std::shared_ptr<Node> parseMultiplicativeExpression(TokenList& input)
    {
        auto left = parseAccessors(input);

        while (input.NotEmpty() && (input.Front().type == TokenType::OpDivide || input.Front().type ==
            TokenType::OpMultiply))
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

        while (input.NotEmpty() && (input.Front().type == TokenType::OpAdd || input.Front().type ==
            TokenType::OpSubtract))
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

        while (input.NotEmpty() && (input.Front().type == TokenType::OpEqual || input.Front().type ==
            TokenType::OpNotEqual ||
            input.Front().type == TokenType::OpLess || input.Front().type == TokenType::OpLessEqual || input.Front().
            type ==
            TokenType::OpGreater || input.Front().type == TokenType::OpGreaterEqual))
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

        while (input.NotEmpty() && (input.Front().type == TokenType::OpAnd || input.Front().type == TokenType::OpOr ||
            input.Front().type == TokenType::OpNot))
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

        while (input.NotEmpty() && (input.Front().type == TokenType::Conditional))
        {
            auto token = input.RemoveFront();
            auto leftTokens = consumeTokensTill(input, std::set{TokenType::Colon});
            left = std::make_shared<ConditionalNode>(left, parseExpression(leftTokens), parseExpression(input));
        }

        return left;
    }

    std::shared_ptr<Node> parseAssignmentExpression(TokenList& input)
    {
        auto left = parseLogicalExpression(input);

        while (input.NotEmpty() && input.Front().type == TokenType::Assign)
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
        input.ExpectFront(TokenType::OpenBrace).RemoveFront();
        while (input.Front().type != TokenType::CloseBrace)
        {
            auto declarationTokens = consumeTokensTill(input, setOf(TokenType::StatementEnd));
            input.ExpectFront(TokenType::StatementEnd).RemoveFront();
            result.push_back(parseDeclaration(declarationTokens));
        }

        input.RemoveFront();

        return result;
    }

    std::shared_ptr<StructNode> parseStruct(TokenList& input)
    {
        input.ExpectFront(TokenType::TypeStruct).RemoveFront();
        auto name = input.RemoveFront();
        auto declarations = parseStructScope(input);
        input.ExpectFront(TokenType::StatementEnd).RemoveFront();
        return std::make_shared<StructNode>(name.value, declarations);
    }

    std::shared_ptr<IfNode> parseIf(TokenList& input)
    {
        input.ExpectFront(TokenType::If).RemoveFront();
        auto condition = consumeTokensTill(input, setOf(TokenType::CloseParen));
        condition.ExpectFront(TokenType::OpenParen).RemoveFront();

        input.ExpectFront(TokenType::CloseParen).RemoveFront();

        auto cond = parseExpression(condition);

        auto scope = parseScope(input);

        if (input.NotEmpty() && input.Front().type == TokenType::Else)
        {
            input.RemoveFront();
            std::shared_ptr<Node> elseScope;
            if (input.Front().type == TokenType::If)
                elseScope = parseIf(input);
            else
                elseScope = parseScope(input);

            return std::make_shared<IfNode>(cond, scope, elseScope);
        }

        return std::make_shared<IfNode>(cond, scope);
    }

    std::shared_ptr<ForNode> parseFor(TokenList& input)
    {
        input.ExpectFront(TokenType::For).RemoveFront();

        auto withinParen = consumeTokensTill(input, setOf(TokenType::CloseParen));

        withinParen.ExpectFront(TokenType::OpenParen).RemoveFront();

        input.ExpectFront(TokenType::CloseParen).RemoveBack();

        auto initTokens = consumeTokensTill(withinParen, setOf(TokenType::Colon));

        withinParen.ExpectFront(TokenType::Colon).RemoveFront();

        auto condTokens = consumeTokensTill(withinParen, setOf(TokenType::Colon));

        withinParen.ExpectFront(TokenType::Colon).RemoveFront();

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
        input.ExpectFront(TokenType::Layout).RemoveFront();

        input.ExpectFront(TokenType::OpenParen).RemoveFront();

        auto tagTokens = consumeTokensTill(input, setOf(TokenType::CloseParen), 1);

        input.ExpectFront(TokenType::CloseParen).RemoveFront();
        std::unordered_map<std::string, std::string> tags{};
        while (tagTokens.NotEmpty())
        {
            auto id = tagTokens.RemoveFront();
            if (tagTokens.NotEmpty() && tagTokens.Front().type == TokenType::Assign)
            {
                tagTokens.RemoveFront();
                auto val = tagTokens.RemoveFront();
                tags.emplace(id.value, val.value);
            }
            else
            {
                tags.emplace(id.value, "");
            }

            if (tagTokens.NotEmpty() && tagTokens.Front().type == TokenType::Comma)
            {
                tagTokens.RemoveFront();
            }
        }

        const auto layoutTypeToken = input.RemoveFront();

        ELayoutType layoutType;

        switch (layoutTypeToken.type)
        {
        case TokenType::DataIn:
            layoutType = ELayoutType::Input;
            break;
        case TokenType::DataOut:
            layoutType = ELayoutType::Output;
            break;
        case TokenType::ReadOnly:
            layoutType = ELayoutType::Readonly;
            break;
        case TokenType::Uniform:
            layoutType = ELayoutType::Uniform;
            break;
        default:
            layoutType = ELayoutType::Input;
            break;
        }

        auto declaration = parseDeclaration(input);

        input.ExpectFront(TokenType::StatementEnd).RemoveFront();

        return std::make_shared<LayoutNode>(layoutType, declaration, tags);
    }

    std::shared_ptr<PushConstantNode> parsePushConstant(TokenList& input)
    {
        input.ExpectFront(TokenType::PushConstant).RemoveFront();

        input.ExpectFront(TokenType::OpenParen).RemoveFront();

        auto tagTokens = consumeTokensTill(input, setOf(TokenType::CloseParen), 1);

        input.ExpectFront(TokenType::CloseParen).RemoveFront();
        std::unordered_map<std::string, std::string> tags{};
        while (tagTokens.NotEmpty())
        {
            auto id = tagTokens.RemoveFront();
            if (tagTokens.NotEmpty() && tagTokens.Front().type == TokenType::Assign)
            {
                tagTokens.RemoveFront();
                auto val = tagTokens.RemoveFront();
                tags.emplace(id.value, val.value);
            }
            else
            {
                tags.emplace(id.value, "");
            }

            if (tagTokens.NotEmpty() && tagTokens.Front().type == TokenType::Comma)
            {
                tagTokens.RemoveFront();
            }
        }

        auto declarations = parseStructScope(input);

        input.ExpectFront(TokenType::StatementEnd).RemoveFront();

        return std::make_shared<PushConstantNode>(declarations, tags);
    }

    std::shared_ptr<DefineNode> parseDefine(TokenList& input)
    {
        input.ExpectFront(TokenType::Define).RemoveFront();
        auto identifier = input.RemoveFront();
        auto expr = consumeTokensTill(input, setOf(TokenType::StatementEnd));
        input.ExpectFront(TokenType::StatementEnd).RemoveFront();
        return std::make_shared<DefineNode>(identifier.value, parseExpression(expr));
    }

    std::shared_ptr<IncludeNode> parseInclude(TokenList& input)
    {
        input.ExpectFront(TokenType::Include).RemoveFront();
        auto token = input.ExpectFront(TokenType::StringLiteral).RemoveFront();
        return std::make_shared<IncludeNode>(token.debugInfo.file, token.value);
    }

    std::shared_ptr<ScopeNode> parseScope(TokenList& input)
    {
        std::vector<std::shared_ptr<Node>> statements{};

        input.ExpectFront(TokenType::OpenBrace).RemoveFront();

        while (input.Front().type != TokenType::CloseBrace)
        {
            switch (input.Front().type)
            {
            case TokenType::If:
                {
                    statements.push_back(parseIf(input));
                }
                break;
            case TokenType::For:
                {
                    statements.push_back(parseFor(input));
                }
                break;
            case TokenType::Return:
                {
                    input.RemoveFront();
                    auto statementTokens = consumeTokensTill(input, setOf(TokenType::StatementEnd));
                    input.ExpectFront(TokenType::StatementEnd).RemoveFront();

                    statements.push_back(std::make_shared<ReturnNode>(parseExpression(statementTokens)));
                }
                break;
            default:
                {
                    auto statementTokens = consumeTokensTill(input, setOf(TokenType::StatementEnd));

                    input.ExpectFront(TokenType::StatementEnd).RemoveFront();

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

        input.ExpectFront(TokenType::OpenBrace).RemoveFront();

        while (input.Front().type != TokenType::CloseBrace)
        {
            switch (input.Front().type)
            {
            case TokenType::Include:
                statements.push_back(parseInclude(input));
                break;
            case TokenType::Define:
                statements.push_back(parseDefine(input));
                break;
            case TokenType::Layout:
                statements.push_back(parseLayout(input));
                break;
            case TokenType::TypeStruct:
                statements.push_back(parseStruct(input));
                break;
            case TokenType::Const:
                {
                    auto tokens = consumeTokensTill(input, setOf(TokenType::StatementEnd));
                    input.ExpectFront(TokenType::StatementEnd).RemoveFront();
                    statements.push_back(parseExpression(tokens));
                }
                break;
            case TokenType::PushConstant:
                {
                    statements.push_back(parsePushConstant(input));
                }
                break;
            case TokenType::TypeVoid:
            case TokenType::TypeFloat:
            case TokenType::TypeFloat2:
            case TokenType::TypeFloat3:
            case TokenType::TypeFloat4:
            case TokenType::TypeInt:
            case TokenType::TypeInt2:
            case TokenType::TypeInt3:
            case TokenType::TypeInt4:
            case TokenType::TypeBoolean:
            case TokenType::TypeMat3:
            case TokenType::TypeMat4:
            case TokenType::Unknown:
                statements.push_back(parseFunction(input));
                break;
            default:
                {
                    auto statementTokens = consumeTokensTill(input, setOf(TokenType::StatementEnd));

                    input.ExpectFront(TokenType::StatementEnd).RemoveFront();

                    statements.push_back(parseExpression(statementTokens));
                }
            }
        }

        input.RemoveFront();

        EScopeType scopeType;

        switch (scopeTypeToken.type)
        {
        case TokenType::FragmentScope:
            scopeType = EScopeType::Fragment;
            break;
        case TokenType::VertexScope:
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

        if (type.type == TokenType::TypeBuffer)
        {
            auto name = input.ExpectFront(TokenType::Unknown).RemoveFront();
            auto declarations = parseStructScope(input);
            return std::make_shared<BufferDeclarationNode>(name.value, 0, declarations);
        }

        if (type.type == TokenType::Unknown && input.NotEmpty() && input.Front().type == TokenType::OpenBrace)
        {
            auto name = type;
            auto declarations = parseStructScope(input);
            return std::make_shared<BlockDeclarationNode>(name.value, 0, declarations);
        }

        auto name = input.Front().type == TokenType::Unknown
                        ? input.ExpectFront(TokenType::Unknown).RemoveFront().value
                        : "";

        auto returnCount = 1;

        if (input.NotEmpty() && input.Front().type == TokenType::OpenBracket)
        {
            input.RemoveFront();
            returnCount = input.Front().type == TokenType::Numeric ? parseInt(input.RemoveFront().value) : -1;
            input.ExpectFront(TokenType::CloseBracket).RemoveFront();
        }

        return type.type == TokenType::Unknown
                   ? std::make_shared<StructDeclarationNode>(type.value, name, returnCount)
                   : std::make_shared<DeclarationNode>(type, name, returnCount);
    }

    std::shared_ptr<FunctionArgumentNode> parseFunctionArgument(TokenList& input)
    {
        bool isInput = true;
        
        if (input.Front().type == TokenType::DataIn || input.Front().type == TokenType::DataOut)
        {
            const auto token = input.RemoveFront();
            isInput = token.type == TokenType::DataIn;
        }
        
        auto type = input.RemoveFront();
        
        auto returnCount = 1;

        if (input.NotEmpty() && input.Front().type == TokenType::OpenBracket)
        {
            input.RemoveFront();
            returnCount = input.Front().type == TokenType::Numeric ? parseInt(input.RemoveFront().value) : -1;
            input.ExpectFront(TokenType::CloseBracket).RemoveFront();
        }
        
        auto name = input.RemoveFront().value;
        
        return std::make_shared<FunctionArgumentNode>(isInput, std::make_shared<DeclarationNode>(type, name, returnCount));
    }

    std::shared_ptr<FunctionNode> parseFunction(TokenList& input)
    {
        auto type = input.RemoveFront();
        auto returnCount = 0;

        if (input.Front().type == TokenType::OpenBracket)
        {
            input.RemoveFront();
            returnCount = input.Front().type == TokenType::Numeric
                              ? parseInt(input.ExpectFront(TokenType::Numeric).RemoveFront().value)
                              : -1;
            input.ExpectFront(TokenType::CloseBracket).RemoveFront();
        }

        auto name = input.ExpectFront(TokenType::Unknown).RemoveFront();

        input.ExpectFront(TokenType::OpenParen).RemoveFront();
        auto allArgsTokens = consumeTokensTill(input, setOf(TokenType::CloseParen), 1);
        input.ExpectFront(TokenType::CloseParen).RemoveFront();

        std::vector<std::shared_ptr<FunctionArgumentNode>> args{};

        while (allArgsTokens.NotEmpty())
        {
            auto argsTokens = consumeTokensTill(allArgsTokens, setOf(TokenType::Comma));

            if (allArgsTokens.NotEmpty())
            {
                allArgsTokens.ExpectFront(TokenType::Comma).RemoveFront();
            }

            args.push_back(parseFunctionArgument(argsTokens));
        }

        auto returnDecl = type.type == TokenType::Unknown
                              ? std::make_shared<StructDeclarationNode>(type.value, "", returnCount)
                              : std::make_shared<DeclarationNode>(type, "", returnCount);

        if (input.Front().type == TokenType::Arrow)
        {
            input.RemoveFront();
            auto expr = consumeTokensTill(input, setOf(TokenType::StatementEnd));
            input.ExpectFront(TokenType::StatementEnd).RemoveFront();
            return std::make_shared<FunctionNode>(returnDecl, name.value,
                                                  args, std::make_shared<ScopeNode>(std::vector<std::shared_ptr<Node>>{
                                                      std::make_shared<ReturnNode>(parseExpression(expr))
                                                  }));
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
            case TokenType::FragmentScope:
            case TokenType::VertexScope:
                statements.push_back(parseNamedScope(input));
                break;
            case TokenType::Include:
                statements.push_back(parseInclude(input));
                break;
            case TokenType::Define:
                statements.push_back(parseDefine(input));
                break;
            case TokenType::Layout:
                statements.push_back(parseLayout(input));
                break;
            case TokenType::TypeStruct:
                statements.push_back(parseStruct(input));
                break;
            case TokenType::Const:
                {
                    auto tokens = consumeTokensTill(input, setOf(TokenType::StatementEnd));
                    input.ExpectFront(TokenType::StatementEnd).RemoveFront();
                    statements.push_back(parseExpression(tokens));
                }
                break;
            case TokenType::PushConstant:
                {
                    statements.push_back(parsePushConstant(input));
                }
                break;
            case TokenType::TypeVoid:
            case TokenType::TypeFloat:
            case TokenType::TypeFloat2:
            case TokenType::TypeFloat3:
            case TokenType::TypeFloat4:
            case TokenType::TypeInt:
            case TokenType::TypeInt2:
            case TokenType::TypeInt3:
            case TokenType::TypeInt4:
            case TokenType::TypeBoolean:
            case TokenType::TypeMat3:
            case TokenType::TypeMat4:
            case TokenType::Unknown:
                statements.push_back(parseFunction(input));
                break;
            default:
                throw std::exception("Unexpected Token type");
            }
        }

        return std::make_shared<ModuleNode>(statements);
    }
}
