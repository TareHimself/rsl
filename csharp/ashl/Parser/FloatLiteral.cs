namespace ashl.Parser;

public class FloatLiteral : Node
{
    public float Value;

    public FloatLiteral(float value) : base(ENodeType.FloatLiteral)
    {
        Value = value;
    }

    public override IEnumerable<Node> GetChildren() => [];
}