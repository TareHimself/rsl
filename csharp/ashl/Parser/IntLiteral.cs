namespace rsl.Parser;

/// <summary>
///     <see cref="Value" />
/// </summary>
public class IntLiteral : Node
{
    public int Value;

    public IntLiteral(int value) : base(ENodeType.IntLiteral)
    {
        Value = value;
    }

    public override IEnumerable<Node> GetChildren() => [];
}