namespace ashl.Parser;
/// <summary>
/// { <see cref="Expressions"/> }
/// </summary>
public class ArrayLiteralNode : Node
{
    public Node[] Expressions;
    public ArrayLiteralNode(IEnumerable<Node> expressions) : base(ENodeType.ArrayLiteral)
    {
        Expressions = expressions.ToArray();
    }

    public override IEnumerable<Node> GetChildren() => Expressions;
}