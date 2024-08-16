namespace ashl.Parser;

public class NoOpNode : Node
{
    public NoOpNode() : base(ENodeType.NoOp)
    {
    }

    public override IEnumerable<Node> GetChildren() => [];
}