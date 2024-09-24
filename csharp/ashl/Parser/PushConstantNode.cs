namespace rsl.Parser;

public class PushConstantNode : Node
{
    public StructNode Data;
    public string Name;

    public Dictionary<string, string> Tags;

    public PushConstantNode(string name, StructNode data, Dictionary<string, string> tags) : base(
        ENodeType.PushConstant)
    {
        Name = name;
        Data = data;
        Tags = tags;
    }

    public override IEnumerable<Node> GetChildren()
    {
        return [Data];
    }
}