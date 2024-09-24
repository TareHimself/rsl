#pragma once
#include "nodes.hpp"
#include "TokenList.hpp"

namespace rsl
{
    // void getStatementTokens(TokenList& statement, TokenList& input);
    // // Gets tokens till end (scope aware)
    // void getTokensTill(TokenList& result,TokenList& input,const std::function<bool(const Token&,int)>& evaluator,int initialScope = 0,bool popEnd = true);
    // void getTokensTill(TokenList& result,TokenList& input,const std::set<TokenType>& ends,int initialScope = 0,bool popEnd = true);
    // std::shared_ptr<ScopeNode> parseScope(TokenList& input);


    TokenList consumeTokensTill(TokenList& input, const std::set<TokenType>& targets, const int& initialScope = 0,
                                const bool& includeTarget = false);

    std::shared_ptr<Node> resolveTokenToLiteralOrIdentifier(const Token& input);

    std::shared_ptr<Node> parseParen(TokenList& input);

    std::shared_ptr<ArrayLiteralNode> parseArrayLiteral(TokenList& input);

    std::shared_ptr<Node> parsePrimary(TokenList& input);

    std::shared_ptr<Node> parseAccessors(TokenList& input, const std::shared_ptr<Node>& initialLeft = {});

    std::shared_ptr<Node> parseMultiplicativeExpression(TokenList& input);

    std::shared_ptr<Node> parseAdditiveExpression(TokenList& input);

    std::shared_ptr<Node> parseComparisonExpression(TokenList& input);

    std::shared_ptr<Node> parseLogicalExpression(TokenList& input);

    std::shared_ptr<Node> parseConditionalExpression(TokenList& input);

    std::shared_ptr<Node> parseAssignmentExpression(TokenList& input);

    std::shared_ptr<Node> parseExpression(TokenList& input);

    std::vector<std::shared_ptr<DeclarationNode>> parseStructScope(TokenList& input);

    std::shared_ptr<StructNode> parseStruct(TokenList& input);

    std::shared_ptr<IfNode> parseIf(TokenList& input);

    std::shared_ptr<ForNode> parseFor(TokenList& input);

    std::shared_ptr<LayoutNode> parseLayout(TokenList& input);

    std::shared_ptr<PushConstantNode> parsePushConstant(TokenList& input);

    std::shared_ptr<DefineNode> parseDefine(TokenList& input);

    std::shared_ptr<IncludeNode> parseInclude(TokenList& input);

    std::shared_ptr<ScopeNode> parseScope(TokenList& input);

    std::shared_ptr<NamedScopeNode> parseNamedScope(TokenList& input);

    std::shared_ptr<DeclarationNode> parseDeclaration(TokenList& input);

    std::shared_ptr<FunctionArgumentNode> parseFunctionArgument(TokenList& input);

    std::shared_ptr<FunctionNode> parseFunction(TokenList& input);

    std::shared_ptr<ModuleNode> parse(TokenList& input);
}
