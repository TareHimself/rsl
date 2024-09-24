namespace rsl.Parser;

public class DefineNode : Node
{
    public string Identifier;
    public Node Expression;
    public DefineNode(string identifier,Node expression) : base(ENodeType.Define)
    {
        Identifier = identifier;
        Expression = expression;
    }

    public override IEnumerable<Node> GetChildren() => [Expression];
}