namespace ashl.Parser;

public abstract class Node
{
    public ENodeType NodeType;

    public Node(ENodeType type)
    {
        NodeType = type;
    }


    public virtual IEnumerable<Node> GetChildren()
    {
        return [];
    }
}