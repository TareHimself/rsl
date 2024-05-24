namespace ashl.Parser;

/// <summary>
/// -( <see cref="Expression"/> )
/// </summary>
public class NegateNode : Node
{
    public Node Expression;

    public NegateNode(Node expression) : base(ENodeType.Negate)
    {
        Expression = expression;
    }
}