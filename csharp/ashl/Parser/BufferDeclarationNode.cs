namespace rsl.Parser;

public class BufferDeclarationNode : DeclarationNode
{
    public readonly DeclarationNode[] Declarations;
    
    public BufferDeclarationNode(string name, int count,IEnumerable<DeclarationNode> declarations) : base(EDeclarationType.Buffer, name, count)
    {
        Declarations = declarations.ToArray();
    }

    public override int SizeOf() => Declarations.Aggregate(0, (total, decl) => total + decl.SizeOf());

    public override IEnumerable<Node> GetChildren() => Declarations;

    public override string GetTypeName() => "";
}