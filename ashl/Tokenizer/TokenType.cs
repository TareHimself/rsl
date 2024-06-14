﻿namespace ashl.Tokenizer;

public enum TokenType
{
    Unknown,
    [KeywordToken("=")] Assign,
    [KeywordToken(".")] Access,
    [KeywordToken("&&")] OpAnd,
    [KeywordToken("||")] OpOr,
    [KeywordToken("!")] OpNot,
    [KeywordToken("++")] OpIncrement,
    [KeywordToken("--")] OpDecrement,
    [KeywordToken("+=")] OpAddAssign,
    [KeywordToken("-=")] OpSubtractAssign,
    [KeywordToken("/=")] OpDivideAssign,
    [KeywordToken("*=")] OpMultiplyAssign,
    [KeywordToken("+")] OpAdd,
    [KeywordToken("-")] OpSubtract,
    [KeywordToken("/")] OpDivide,
    [KeywordToken("*")] OpMultiply,
    [KeywordToken("%")] OpMod,
    [KeywordToken("==")] OpEqual,
    [KeywordToken("!=")] OpNotEqual,
    [KeywordToken("<")] OpLess,
    [KeywordToken(">")] OpGreater,
    [KeywordToken("<=")] OpLessEqual,
    [KeywordToken(">=")] OpGreaterEqual,
    [KeywordToken("{")] OpenBrace,
    [KeywordToken("}")] CloseBrace,
    [KeywordToken("(")] OpenParen,
    [KeywordToken(")")] CloseParen,
    [KeywordToken("[")] OpenBracket,
    [KeywordToken("]")] CloseBracket,
    Identifier,
    Function,
    [KeywordToken("return")] Return,
    [KeywordToken(",")] Comma,
    BooleanLiteral,
    [KeywordToken("for")] For,
    [KeywordToken("continue")] Continue,
    [KeywordToken("break")] Break,
    [KeywordToken(";")] StatementEnd,
    DeclarationCount,
    [KeywordToken("struct")] TypeStruct,
    [KeywordToken("float")] TypeFloat,
    [KeywordToken("float2")] TypeFloat2,
    [KeywordToken("float3")] TypeFloat3,
    [KeywordToken("float4")] TypeFloat4,
    [KeywordToken("int")] TypeInt,
    [KeywordToken("int2")] TypeInt2,
    [KeywordToken("int3")] TypeInt3,
    [KeywordToken("int4")] TypeInt4,
    [KeywordToken("mat3")] TypeMat3,
    [KeywordToken("mat4")] TypeMat4,
    [KeywordToken("bool")] TypeBoolean,
    [KeywordToken("void")] TypeVoid,
    [KeywordToken("sampler2D")] TypeSampler2D,
    [KeywordToken("buffer")] TypeBuffer,
    [KeywordToken("in")] DataIn,
    [KeywordToken("out")] DataOut,
    [KeywordToken("layout")] Layout,
    [KeywordToken("uniform")] Uniform,
    [KeywordToken("readonly")] ReadOnly,
    [KeywordToken("discard")] Discard,
    [KeywordToken("#include")] Include,
    [KeywordToken("#define")] Define,
    [KeywordToken("const")] Const,
    [KeywordToken("push")] PushConstant,
    [KeywordToken("if")] If,
    [KeywordToken("else")] Else,
    [KeywordToken("?")] Conditional,
    [KeywordToken(":")] Colon,
    [KeywordToken("@Vertex")] VertexScope,
    [KeywordToken("@Fragment")] FragmentScope
}