namespace ashl.Parser;

public class IfNode : Node
{
    public Node Condition;
    public ScopeNode Scope;
    public Node Else;
    public IfNode(Node condition,ScopeNode scope,Node elseNode) : base(ENodeType.If)
    {
        Condition = condition;
        Scope = scope;
        Else = elseNode;
    }


    public override IEnumerable<Node> GetChildren() => [Condition, Scope, Else];
}