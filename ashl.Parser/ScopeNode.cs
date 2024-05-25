namespace ashl.Parser;

/// <summary>
/// {
/// <see cref="Statements"/>
/// }
/// </summary>
public class ScopeNode : Node
{
    public Node[] Statements;
    public ScopeNode(IEnumerable<Node> statements) : base(ENodeType.Scope){
        Statements = statements.ToArray();
    }

    public override IEnumerable<Node> GetChildren() => Statements;
}
