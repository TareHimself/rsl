#include "rsl/Token.hpp"

#include <ranges>


namespace rsl
{
    std::unordered_map<TokenType, std::string> Token::TOKENS_TO_KEYWORDS = {
        {TokenType::Assign, "="},
        {TokenType::Access, "."},
        {TokenType::OpAnd, "&&"},
        {TokenType::OpOr, "||"},
        {TokenType::OpNot, "!"},
        {TokenType::OpIncrement, "++"},
        {TokenType::OpDecrement, "--"},
        {TokenType::OpAddAssign, "+="},
        {TokenType::OpSubtractAssign, "-="},
        {TokenType::OpDivideAssign, "/="},
        {TokenType::OpMultiplyAssign, "*="},
        {TokenType::OpAdd, "+"},
        {TokenType::OpSubtract, "-"},
        {TokenType::OpDivide, "/"},
        {TokenType::OpMultiply, "*"},
        {TokenType::OpMod, "%"},
        {TokenType::OpEqual, "=="},
        {TokenType::OpNotEqual, "!="},
        {TokenType::OpLess, "<"},
        {TokenType::OpGreater, ">"},
        {TokenType::OpLessEqual, "<="},
        {TokenType::OpGreaterEqual, ">="},
        {TokenType::OpenBrace, "{"},
        {TokenType::CloseBrace, "}"},
        {TokenType::OpenParen, "("},
        {TokenType::CloseParen, ")"},
        {TokenType::OpenBracket, "["},
        {TokenType::CloseBracket, "]"},
        {TokenType::Return, "return"},
        {TokenType::Comma, ","},
        {TokenType::For, "for"},
        {TokenType::Continue, "continue"},
        {TokenType::Break, "break"},
        {TokenType::StatementEnd, ";"},
        {TokenType::TypeStruct, "struct"},
        {TokenType::TypeFloat, "float"},
        {TokenType::TypeFloat2, "float2"},
        {TokenType::TypeFloat3, "float3"},
        {TokenType::TypeFloat4, "float4"},
        {TokenType::TypeInt, "int"},
        {TokenType::TypeInt2, "int2"},
        {TokenType::TypeInt3, "int3"},
        {TokenType::TypeInt4, "int4"},
        {TokenType::TypeMat3, "mat3"},
        {TokenType::TypeMat4, "mat4"},
        {TokenType::TypeBoolean, "bool"},
        {TokenType::TypeVoid, "void"},
        {TokenType::TypeSampler,"sampler"},
        {TokenType::TypeTexture2D,"texture2D"},
        {TokenType::TypeSampler2D, "sampler2D"},
        {TokenType::TypeBuffer, "buffer"},
        {TokenType::DataIn, "in"},
        {TokenType::DataOut, "out"},
        {TokenType::Layout, "layout"},
        {TokenType::Uniform, "uniform"},
        {TokenType::ReadOnly, "readonly"},
        {TokenType::Discard, "discard"},
        {TokenType::Include, "#include"},
        {TokenType::Define, "#define"},
        {TokenType::Const, "const"},
        {TokenType::PushConstant, "push"},
        {TokenType::If, "if"},
        {TokenType::Else, "else"},
        {TokenType::Conditional, "?"},
        {TokenType::Colon, ":"},
        {TokenType::Arrow, "->"},
        {TokenType::VertexScope, "@Vertex"},
        {TokenType::FragmentScope, "@Fragment"},
    };

    std::unordered_map<std::string, TokenType> Token::KEYWORDS_TO_TOKENS = []
    {
        std::unordered_map<std::string, TokenType> m{};

        for (const auto& [fst, snd] : TOKENS_TO_KEYWORDS)
        {
            m.emplace(snd, fst);
        }

        return m;
    }();

    std::map<int, std::set<std::string>> Token::SIZES_TO_KEYWORDS = []
    {
        std::map<int, std::set<std::string>> m{};

        for (const auto& snd : TOKENS_TO_KEYWORDS | std::views::values)
        {
            auto id = static_cast<int>(snd.size());

            if (!m.contains(id)) m.emplace(snd.size(), std::set<std::string>{});

            m[id].emplace(snd);
        }

        return m;
    }();

    Token::Token(const TokenType inType, const TokenDebugInfo& inDebugInfo) : Token(
        inType, TOKENS_TO_KEYWORDS.contains(inType) ? TOKENS_TO_KEYWORDS[inType] : "", inDebugInfo)
    {
    }

    Token::Token(const std::string& inValue, const TokenDebugInfo& inDebugInfo) : Token(
        KEYWORDS_TO_TOKENS.contains(inValue) ? KEYWORDS_TO_TOKENS[inValue] : TokenType::Unknown, inValue, inDebugInfo)
    {
    }

    Token::Token(const TokenType inType, const std::string& inValue, const TokenDebugInfo& inDebugInfo)
    {
        type = inType;
        value = inValue;
        debugInfo = inDebugInfo;
    }
}
