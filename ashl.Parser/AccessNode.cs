namespace ashl.Parser;

/// <summary>
/// <see cref="HasLeftNode.Left"/>.<see cref="Right"/>
/// </summary>
public class AccessNode : HasLeftNode
{
    public Node Right;

    public AccessNode(Node left, Node right) : base(left, ENodeType.Access)
    {
        Right = right;
    }
}