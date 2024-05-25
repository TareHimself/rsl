namespace ashl.Parser;

public enum ENodeType
{
    Unknown,
    BinaryOp,
    Module,
    Function,
    FunctionArgument,
    Return,
    Assign,
    Layout,
    Call,
    Access,
    Index,
    Scope,
    NamedScope,
    Identifier,
    Struct,
    Declaration,
    Include,
    FloatLiteral,
    IntLiteral,
    Const,
    ArrayLiteral,
    Negate,
    Precedence,
    PushConstant
}