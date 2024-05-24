namespace ashl.Parser;

/// <summary>
/// <see cref="DeclarationNode"/> <see cref="Name">FunctionName</see>( <see cref="Arguments"/> ) <see cref="Scope"/>
/// </summary>
public class FunctionNode : Node
{
    public string Name;
    public DeclarationNode ReturnDeclaration;
    public FunctionArgumentNode[] Arguments;
    public ScopeNode Scope;
    public FunctionNode(string name,DeclarationNode returnDeclaration,IEnumerable<FunctionArgumentNode> arguments,ScopeNode scope) : base(ENodeType.Function)
    {
        Name = name;
        ReturnDeclaration = returnDeclaration;
        Arguments = arguments.ToArray();
        Scope = scope;
    }
}
