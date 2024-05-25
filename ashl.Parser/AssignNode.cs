namespace ashl.Parser;

/// <summary>
/// <see cref="HasLeftNode.Left"/> = <see cref="Right"/>
/// </summary>
public class AssignNode : HasLeftNode
{
    public Node Right;
    public AssignNode(Node left,Node right) : base(left, ENodeType.Assign)
    {
        Right = right;
    }

    public override IEnumerable<Node> GetChildren() => [Right];
}