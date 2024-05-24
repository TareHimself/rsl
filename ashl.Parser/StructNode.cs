namespace ashl.Parser;

/// <summary>
/// struct <see cref="Name"/> {
/// <see cref="Declarations"/>
/// }
/// </summary>
public class StructNode : Node
{
    public string Name;
    public DeclarationNode[] Declarations;
    public StructNode(string name,IEnumerable<DeclarationNode> declarations) : base(ENodeType.Struct)
    {
        Name = name;
        Declarations = declarations.ToArray();
    }
}
