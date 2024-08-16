namespace ashl.Parser;

public abstract class Node
{
    public ENodeType NodeType;

    public Node(ENodeType type)
    {
        NodeType = type;
    }


    public abstract IEnumerable<Node> GetChildren();
}