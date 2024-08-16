namespace ashl.Parser;

public class HasLeftNode : Node
{
    public Node Left;

    public HasLeftNode(Node left, ENodeType nodeType) : base(nodeType)
    {
        Left = left;
    }

    public override IEnumerable<Node> GetChildren() => [Left];
}