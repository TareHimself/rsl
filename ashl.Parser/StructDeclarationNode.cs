namespace ashl.Parser;

/// <summary>
/// <see cref="StructName"/> <see cref="DeclarationNode.Name"/>
/// </summary>
public class StructDeclarationNode : DeclarationNode
{
    public StructNode? Struct;
    public string StructName;

    public StructDeclarationNode(string name, int count,string structName) : base(EDeclarationType.Struct,name,count)
    {
        StructName = structName;
    }
    
    public StructDeclarationNode(StructNode inStruct,string name, int count) : this(name,count,inStruct.Name)
    {
        Struct = inStruct;
        StructName = inStruct.Name;
    }

    public override int SizeOf()
    {
        if (Struct == null) throw new Exception("Struct has not been resolved");
        return Struct.Declarations.Aggregate(0,(total, node) => total + node.SizeOf());
    }
}
