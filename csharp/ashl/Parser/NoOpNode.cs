namespace rsl.Parser;

public class NoOpNode : Node
{
    public NoOpNode() : base(ENodeType.NoOp)
    {
    }

    public override IEnumerable<Node> GetChildren() => [];
}