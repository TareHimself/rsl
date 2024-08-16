#include "ashl/Token.hpp"

#include <ranges>


namespace ashl
{
    std::unordered_map<ETokenType,std::string> Token::TOKENS_TO_KEYWORDS = {
        {ETokenType::Assign, "="},
    {ETokenType::Access, "."},
    {ETokenType::OpAnd, "&&"},
    {ETokenType::OpOr, "||"},
    {ETokenType::OpNot, "!"},
    {ETokenType::OpIncrement, "++"},
    {ETokenType::OpDecrement, "--"},
    {ETokenType::OpAddAssign, "+="},
    {ETokenType::OpSubtractAssign, "-="},
    {ETokenType::OpDivideAssign, "/="},
    {ETokenType::OpMultiplyAssign, "*="},
    {ETokenType::OpAdd, "+"},
    {ETokenType::OpSubtract, "-"},
    {ETokenType::OpDivide, "/"},
    {ETokenType::OpMultiply, "*"},
    {ETokenType::OpMod, "%"},
    {ETokenType::OpEqual, "=="},
    {ETokenType::OpNotEqual, "!="},
    {ETokenType::OpLess, "<"},
    {ETokenType::OpGreater, ">"},
    {ETokenType::OpLessEqual, "<="},
    {ETokenType::OpGreaterEqual, ">="},
    {ETokenType::OpenBrace, "{"},
    {ETokenType::CloseBrace, "}"},
    {ETokenType::OpenParen, "("},
    {ETokenType::CloseParen, ")"},
    {ETokenType::OpenBracket, "["},
    {ETokenType::CloseBracket, "]"},
    {ETokenType::Return, "return"},
    {ETokenType::Comma, ","},
    {ETokenType::For, "for"},
    {ETokenType::Continue, "continue"},
    {ETokenType::Break, "break"},
    {ETokenType::StatementEnd, ";"},
    {ETokenType::TypeStruct, "struct"},
    {ETokenType::TypeFloat, "float"},
    {ETokenType::TypeFloat2, "float2"},
    {ETokenType::TypeFloat3, "float3"},
    {ETokenType::TypeFloat4, "float4"},
    {ETokenType::TypeInt, "int"},
    {ETokenType::TypeInt2, "int2"},
    {ETokenType::TypeInt3, "int3"},
    {ETokenType::TypeInt4, "int4"},
    {ETokenType::TypeMat3, "mat3"},
    {ETokenType::TypeMat4, "mat4"},
    {ETokenType::TypeBoolean, "bool"},
    {ETokenType::TypeVoid, "void"},
    {ETokenType::TypeSampler2D, "sampler2D"},
    {ETokenType::TypeBuffer, "buffer"},
    {ETokenType::DataIn, "in"},
    {ETokenType::DataOut, "out"},
    {ETokenType::Layout, "layout"},
    {ETokenType::Uniform, "uniform"},
    {ETokenType::ReadOnly, "readonly"},
    {ETokenType::Discard, "discard"},
    {ETokenType::Include, "#include"},
    {ETokenType::Define, "#define"},
    {ETokenType::Const, "const"},
    {ETokenType::PushConstant, "push"},
    {ETokenType::If, "if"},
    {ETokenType::Else, "else"},
    {ETokenType::Conditional, "?"},
    {ETokenType::Colon, ":"},
        {ETokenType::Arrow,"->"},
    {ETokenType::VertexScope, "@Vertex"},
    {ETokenType::FragmentScope, "@Fragment"},
    };

    std::unordered_map<std::string,ETokenType> Token::KEYWORDS_TO_TOKENS = []
    {

        std::unordered_map<std::string,ETokenType> m{};

        for (const auto& [fst, snd] : TOKENS_TO_KEYWORDS)
        {
            m.emplace(snd,fst);
        }

        return m;
    }();

    std::map<int,std::set<std::string>> Token::SIZES_TO_KEYWORDS = []
    {
        std::map<int,std::set<std::string>> m{};

        for (const auto& snd : TOKENS_TO_KEYWORDS | std::views::values)
        {
            auto id = static_cast<int>(snd.size());

            if(!m.contains(id)) m.emplace(snd.size(),std::set<std::string>{});

            m[id].emplace(snd);
        }

        return m;
    }();

    Token::Token(const ETokenType inType, const TokenDebugInfo& inDebugInfo) : Token(inType,TOKENS_TO_KEYWORDS.contains(inType) ? TOKENS_TO_KEYWORDS[inType] : "",inDebugInfo)
    {

    }

    Token::Token(const std::string& inValue, const TokenDebugInfo& inDebugInfo) : Token(KEYWORDS_TO_TOKENS.contains(inValue) ? KEYWORDS_TO_TOKENS[inValue] : ETokenType::Unknown,inValue,inDebugInfo)
    {

    }

    Token::Token(const ETokenType inType, const std::string& inValue, const TokenDebugInfo& inDebugInfo)
    {
        type = inType;
        value = inValue;
        debugInfo = inDebugInfo;
    }
}
