namespace ashl.Parser;

/// <summary>
///     ( <see cref="Expression" /> )
/// </summary>
public class PrecedenceNode : Node
{
    public Node Expression;

    public PrecedenceNode(Node expression) : base(ENodeType.Precedence)
    {
        Expression = expression;
    }

    public override IEnumerable<Node> GetChildren()
    {
        return [Expression];
    }
}