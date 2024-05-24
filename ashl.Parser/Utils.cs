using ashl.Tokenizer;

namespace ashl.Parser;

public static class Utils
{
    public static EDeclarationType? TokenTypeToDeclarationType(TokenType tokenType) => tokenType switch
    {
        TokenType.TypeFloat => EDeclarationType.Float,
        TokenType.TypeVec2f => EDeclarationType.Vec2f,
        TokenType.TypeVec3f => EDeclarationType.Vec3f,
        TokenType.TypeVec4f => EDeclarationType.Vec4f,
        TokenType.TypeInt => EDeclarationType.Int,
        TokenType.TypeVec2i => EDeclarationType.Vec2i,
        TokenType.TypeVec3i => EDeclarationType.Vec3i,
        TokenType.TypeVec4i => EDeclarationType.Vec4i,
        TokenType.TypeMat3 => EDeclarationType.Mat3,
        TokenType.TypeMat4 => EDeclarationType.Mat4,
        TokenType.TypeVoid => EDeclarationType.Void,
        _ => null
    };
}