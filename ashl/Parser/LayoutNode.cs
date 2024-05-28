namespace ashl.Parser;

/// <summary>
///     layout( <see cref="Tags" /> ) <see cref="LayoutType" /> <see cref="Declaration" />
/// </summary>
public class LayoutNode : Node
{
    public DeclarationNode Declaration;
    public ELayoutType LayoutType;
    public Dictionary<string, string> Tags;

    public LayoutNode(Dictionary<string, string> tags, ELayoutType layoutType, DeclarationNode declaration) : base(
        ENodeType.Layout)
    {
        Tags = tags;
        LayoutType = layoutType;
        Declaration = declaration;
    }

    public override IEnumerable<Node> GetChildren()
    {
        return [Declaration];
    }
}