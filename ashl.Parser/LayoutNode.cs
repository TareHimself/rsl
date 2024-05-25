namespace ashl.Parser;

/// <summary>
/// layout( <see cref="Tags"/> ) <see cref="LayoutType"/> <see cref="Declaration"/>
/// </summary>
public class LayoutNode : Node
{
    public Dictionary<string, string> Tags;
    public ELayoutType LayoutType;
    public DeclarationNode Declaration;

    public LayoutNode(Dictionary<string,string> tags,ELayoutType layoutType,DeclarationNode declaration) : base(ENodeType.Layout)
    {
        Tags = tags;
        LayoutType = layoutType;
        Declaration = declaration;
    }

    public override IEnumerable<Node> GetChildren() => [Declaration];
}