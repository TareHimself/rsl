namespace rsl.Parser;

public class BlockDeclarationNode : DeclarationNode
{
    public string BlockName;
    public readonly DeclarationNode[] Declarations;
    
    public BlockDeclarationNode(string blockName,string name, int count,IEnumerable<DeclarationNode> declarations) : base(EDeclarationType.Block, name, count)
    {
        BlockName = blockName;
        Declarations = declarations.ToArray();
    }

    public override int SizeOf() => Declarations.Aggregate(0, (total, decl) => total + decl.SizeOf());

    public override IEnumerable<Node> GetChildren() => Declarations;

    public override string GetTypeName() => BlockName;
}