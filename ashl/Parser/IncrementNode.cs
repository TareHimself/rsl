namespace ashl.Parser;

public class IncrementNode : Node
{
    public bool IsPre;
    public Node Target;

    public IncrementNode(Node target, bool isPre) : base(ENodeType.Increment)
    {
        Target = target;
        IsPre = isPre;
    }
}