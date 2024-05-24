namespace ashl.Parser;

/// <summary>
/// <see cref="EDeclarationType">Type</see> <see cref="Name"/>[<see cref="Count"/>]
/// </summary>
public class DeclarationNode : Node
{
    public readonly EDeclarationType DeclarationType;
    public string Name;
    public readonly int Count;
    public DeclarationNode(EDeclarationType declarationType,string name, int count) : base(ENodeType.Declaration) {
        DeclarationType  =declarationType;
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
            EDeclarationType.Vec2f => 8 * Count,
            EDeclarationType.Vec2i => 8 * Count,
            EDeclarationType.Vec3f => 12 * Count,
            EDeclarationType.Vec3i => 12 * Count,
            EDeclarationType.Vec4f => 16 * Count,
            EDeclarationType.Vec4i => 16 * Count,
            EDeclarationType.Mat3 => 12 * 3 * Count,
            EDeclarationType.Mat4 => 16 * 4 * Count,
            _ => throw new ArgumentOutOfRangeException()
        };
    }

}
