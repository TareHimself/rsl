export enum ETokenType {
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
    Function,
    Return,
    Comma,
    For,
    Continue,
    Break,
    StatementEnd,
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
    Numeric,
    StringLiteral,
    If,
    Else,
    Conditional,
    Colon,
    Arrow,
    VertexScope,
    FragmentScope
}

export const TOKEN_TO_KEYWORDS = new Map<ETokenType, string>([
    [ETokenType.Assign, "="],
    [ETokenType.Access, "."],
    [ETokenType.OpAnd, "&&"],
    [ETokenType.OpOr, "||"],
    [ETokenType.OpNot, "!"],
    [ETokenType.OpIncrement, "++"],
    [ETokenType.OpDecrement, "--"],
    [ETokenType.OpAddAssign, "+="],
    [ETokenType.OpSubtractAssign, "-="],
    [ETokenType.OpDivideAssign, "/="],
    [ETokenType.OpMultiplyAssign, "*="],
    [ETokenType.OpAdd, "+"],
    [ETokenType.OpSubtract, "-"],
    [ETokenType.OpDivide, "/"],
    [ETokenType.OpMultiply, "*"],
    [ETokenType.OpMod, "%"],
    [ETokenType.OpEqual, "=="],
    [ETokenType.OpNotEqual, "!="],
    [ETokenType.OpLess, "<"],
    [ETokenType.OpGreater, ">"],
    [ETokenType.OpLessEqual, "<="],
    [ETokenType.OpGreaterEqual, ">="],
    [ETokenType.OpenBrace, "{"],
    [ETokenType.CloseBrace, "}"],
    [ETokenType.OpenParen, "("],
    [ETokenType.CloseParen, ")"],
    [ETokenType.OpenBracket, "["],
    [ETokenType.CloseBracket, "]"],
    [ETokenType.Return, "return"],
    [ETokenType.Comma, ","],
    [ETokenType.For, "for"],
    [ETokenType.Continue, "continue"],
    [ETokenType.Break, "break"],
    [ETokenType.StatementEnd, ";"],
    [ETokenType.TypeStruct, "struct"],
    [ETokenType.TypeFloat, "float"],
    [ETokenType.TypeFloat2, "float2"],
    [ETokenType.TypeFloat3, "float3"],
    [ETokenType.TypeFloat4, "float4"],
    [ETokenType.TypeInt, "int"],
    [ETokenType.TypeInt2, "int2"],
    [ETokenType.TypeInt3, "int3"],
    [ETokenType.TypeInt4, "int4"],
    [ETokenType.TypeMat3, "mat3"],
    [ETokenType.TypeMat4, "mat4"],
    [ETokenType.TypeBoolean, "bool"],
    [ETokenType.TypeVoid, "void"],
    [ETokenType.TypeSampler2D, "sampler2D"],
    [ETokenType.TypeBuffer, "buffer"],
    [ETokenType.DataIn, "in"],
    [ETokenType.DataOut, "out"],
    [ETokenType.Layout, "layout"],
    [ETokenType.Uniform, "uniform"],
    [ETokenType.ReadOnly, "readonly"],
    [ETokenType.Discard, "discard"],
    [ETokenType.Include, "#include"],
    [ETokenType.Define, "#define"],
    [ETokenType.Const, "const"],
    [ETokenType.PushConstant, "push"],
    [ETokenType.If, "if"],
    [ETokenType.Else, "else"],
    [ETokenType.Conditional, "?"],
    [ETokenType.Colon, ":"],
    [ETokenType.Arrow, "=>"],
    [ETokenType.VertexScope, "@Vertex"],
    [ETokenType.FragmentScope, "@Fragment"],
]);

export const KEYWORD_TO_TOKENS = new Map<string, ETokenType>(Array.from(TOKEN_TO_KEYWORDS.entries()).map(c => [c[1],c[0]]));

export const TOKEN_SIZES = Array.from(Array.from(TOKEN_TO_KEYWORDS.values()).reduce<Map<number,Set<string>>>((m,keyword)=> {
    if(m.has(keyword.length)){
        m.get(keyword.length)?.add(keyword);
    }
    else
    {
        m.set(keyword.length,new Set([keyword]));
    }
    return m;
},new Map()).entries()).sort((c, d) => d[0] - c[0]);

export class TokenDebugInfo {
    file: string;
    startLine: number;
    startCol: number;
    endLine: number;
    endCol: number;
    constructor(file: string, startLine: number, startCol: number, endLine: number, endCol: number) {
        this.file = file;
        this.startLine = startLine;
        this.startCol = startCol;
        this.endLine = endLine;
        this.endCol = endCol;
    }

    combine(other: TokenDebugInfo): TokenDebugInfo {
        const minStartLine = Math.min(this.startLine, other.startLine);
        const minStartCol = this.startLine == minStartLine && other.startLine == minStartLine ? Math.min(this.startCol, other.startCol) : (this.startLine == minStartLine ? this.startCol : other.startCol);

        const maxEndLine = Math.max(this.endLine, other.endLine);
        const maxEndCol = this.startLine == maxEndLine && other.endLine == maxEndLine ? Math.max(this.endCol, other.endCol) : (this.endLine == maxEndLine ? this.endCol : other.endCol);

        return new TokenDebugInfo(this.file, minStartLine, minStartCol, maxEndLine, maxEndCol);
    }

    static empty(): TokenDebugInfo {
        return new TokenDebugInfo("",0,0,0,0);
    }
}

export default class Token {

    type: ETokenType;
    debug: TokenDebugInfo;
    value: string;

    constructor(type: ETokenType, value: string, debug: TokenDebugInfo) {
        this.type = type;
        this.value = value;
        this.debug = debug;

    }

    static FromType(type: ETokenType, debug: TokenDebugInfo): Token {
        return new Token(type, TOKEN_TO_KEYWORDS[type] ?? "", debug);
    }

    static FromData(data: string, debug: TokenDebugInfo): Token {
        return new Token(KEYWORD_TO_TOKENS[data] ?? ETokenType.Unknown, data, debug);
    }
}

export function isType(tokenType: ETokenType) {
    switch (tokenType) {
        case ETokenType.TypeFloat:
        case ETokenType.TypeFloat2:
        case ETokenType.TypeFloat3:
        case ETokenType.TypeFloat4:
        case ETokenType.TypeInt:
        case ETokenType.TypeInt2:
        case ETokenType.TypeInt3:
        case ETokenType.TypeInt4:
        case ETokenType.TypeMat3:
        case ETokenType.TypeMat4:
        case ETokenType.TypeVoid:
        case ETokenType.TypeBoolean:
            return true;
            break;
        default:
            return false;
    }
}

