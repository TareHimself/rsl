namespace ashl.Parser;

/// <summary>
///     <see cref="EDeclarationType">Type</see> <see cref="Name" />[<see cref="Count" />]
/// </summary>
public class DeclarationNode : Node
{
    public readonly int Count;
    public readonly EDeclarationType DeclarationType;
    public string Name;

    public DeclarationNode(EDeclarationType declarationType, string name, int count) : base(ENodeType.Declaration)
    {
        DeclarationType = declarationType;
        Name = name;
        Count = count;
    }

    public virtual int SizeOf()
    {
        return DeclarationType switch
        {
            EDeclarationType.Struct => throw new Exception(
                "Node with type 'Struct' must use class 'StructDeclarationNode'"),
            EDeclarationType.Float => 4 * Count,
            EDeclarationType.Int => 4 * Count,
            EDeclarationType.Float2 => 8 * Count,
            EDeclarationType.Int2 => 8 * Count,
            EDeclarationType.Float3 => 12 * Count,
            EDeclarationType.Int3 => 12 * Count,
            EDeclarationType.Float4 => 16 * Count,
            EDeclarationType.Int4 => 16 * Count,
            EDeclarationType.Mat3 => 12 * 3 * Count,
            EDeclarationType.Mat4 => 16 * 4 * Count,
            _ => throw new ArgumentOutOfRangeException()
        };
    }

    public virtual string GetTypeName()
    {
        return DeclarationType switch
        {
            EDeclarationType.Float => "float",
            EDeclarationType.Int => "int",
            EDeclarationType.Float2 => "float2",
            EDeclarationType.Int2 => "int2",
            EDeclarationType.Float3 =>  "float3",
            EDeclarationType.Int3 =>  "int3",
            EDeclarationType.Float4 =>  "float4",
            EDeclarationType.Int4 => "int4",
            EDeclarationType.Mat3 => "mat3",
            EDeclarationType.Mat4 => "mat4",
            EDeclarationType.Void => "void",
            EDeclarationType.Sampler2D => "sampler2D",
            EDeclarationType.Buffer => "buffer",
            _ => throw new ArgumentOutOfRangeException()
        };
    }

    public override IEnumerable<Node> GetChildren() => [];
}