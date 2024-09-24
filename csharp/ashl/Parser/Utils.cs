using rsl.Tokenizer;

namespace rsl.Parser;

public static class Utils
{
    public static EDeclarationType? TokenTypeToDeclarationType(TokenType tokenType)
    {
        return tokenType switch
        {
            TokenType.TypeFloat => EDeclarationType.Float,
            TokenType.TypeFloat2 => EDeclarationType.Float2,
            TokenType.TypeFloat3 => EDeclarationType.Float3,
            TokenType.TypeFloat4 => EDeclarationType.Float4,
            TokenType.TypeInt => EDeclarationType.Int,
            TokenType.TypeInt2 => EDeclarationType.Int2,
            TokenType.TypeInt3 => EDeclarationType.Int3,
            TokenType.TypeInt4 => EDeclarationType.Int4,
            TokenType.TypeMat3 => EDeclarationType.Mat3,
            TokenType.TypeMat4 => EDeclarationType.Mat4,
            TokenType.TypeVoid => EDeclarationType.Void,
            TokenType.TypeSampler2D => EDeclarationType.Sampler2D,
            _ => null
        };
    }
}