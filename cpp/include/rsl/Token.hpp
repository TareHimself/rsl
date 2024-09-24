#pragma once
#include <map>
#include <set>
#include <unordered_map>
#include <string>
#include "TokenDebugInfo.hpp"

namespace rsl
{
    enum class TokenType
    {
        Unknown,
        Assign,
        Access,
        OpAnd,
        OpOr,
        OpNot,
        OpIncrement,
        OpDecrement,
        OpAddAssign,
        OpSubtractAssign,
        OpDivideAssign,
        OpMultiplyAssign,
        OpAdd,
        OpSubtract,
        OpDivide,
        OpMultiply,
        OpMod,
        OpEqual,
        OpNotEqual,
        OpLess,
        OpGreater,
        OpLessEqual,
        OpGreaterEqual,
        OpenBrace,
        CloseBrace,
        OpenParen,
        CloseParen,
        OpenBracket,
        CloseBracket,
        Identifier,
        Function,
        Return,
        Comma,
        BooleanLiteral,
        For,
        Continue,
        Break,
        StringLiteral,
        StatementEnd,
        DeclarationCount,
        TypeStruct,
        TypeFloat,
        TypeFloat2,
        TypeFloat3,
        TypeFloat4,
        TypeInt,
        TypeInt2,
        TypeInt3,
        TypeInt4,
        TypeMat3,
        TypeMat4,
        TypeBoolean,
        TypeVoid,
        TypeSampler,
        TypeTexture2D,
        TypeSampler2D,
        TypeBuffer,
        DataIn,
        DataOut,
        Layout,
        Uniform,
        ReadOnly,
        Discard,
        Include,
        Define,
        Const,
        PushConstant,
        If,
        Else,
        Conditional,
        Colon,
        Numeric,
        Arrow,
        VertexScope,
        FragmentScope
    };

    class Token
    {
    public:
        static std::unordered_map<TokenType, std::string> TOKENS_TO_KEYWORDS;
        static std::unordered_map<std::string, TokenType> KEYWORDS_TO_TOKENS;
        static std::map<int, std::set<std::string>> SIZES_TO_KEYWORDS;

        TokenType type = TokenType::Unknown;
        std::string value{};
        TokenDebugInfo debugInfo{};

        Token(TokenType inType, const TokenDebugInfo& inDebugInfo);
        Token(const std::string& inValue, const TokenDebugInfo& inDebugInfo);

        Token(TokenType inType, const std::string& inValue, const TokenDebugInfo& inDebugInfo);
    };
}
