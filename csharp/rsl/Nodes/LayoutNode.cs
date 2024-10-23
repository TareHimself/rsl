namespace rsl.Nodes;

/// <summary>
///     layout( <see cref="Tags" /> ) <see cref="LayoutType" /> <see cref="Declaration" />
/// </summary>
public class LayoutNode : Node
{
    public readonly DeclarationNode Declaration;
    public readonly LayoutType LayoutType;
    public readonly Dictionary<string, string> Tags;

    public LayoutNode(LayoutType layoutType, DeclarationNode declaration,Dictionary<string, string> tags ) : base(
        NodeType.Layout)
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