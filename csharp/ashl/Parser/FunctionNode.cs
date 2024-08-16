namespace ashl.Parser;

/// <summary>
///     <see cref="DeclarationNode" /> <see cref="Name">FunctionName</see>( <see cref="Arguments" /> ) <see cref="Scope" />
/// </summary>
public class FunctionNode : Node
{
    public FunctionArgumentNode[] Arguments;
    public string Name;
    public DeclarationNode ReturnDeclaration;
    public ScopeNode Scope;

    public FunctionNode(string name, DeclarationNode returnDeclaration, IEnumerable<FunctionArgumentNode> arguments,
        ScopeNode scope) : base(ENodeType.Function)
    {
        Name = name;
        ReturnDeclaration = returnDeclaration;
        Arguments = arguments.ToArray();
        Scope = scope;
    }

    public override IEnumerable<Node> GetChildren()
    {
        return [ReturnDeclaration, .. Arguments, Scope];
    }
}