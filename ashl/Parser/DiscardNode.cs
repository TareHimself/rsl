namespace ashl.Parser;

public class DiscardNode : Node
{
    public DiscardNode() : base(ENodeType.Discard)
    {
    }
    
    public override IEnumerable<Node> GetChildren() => [];
}