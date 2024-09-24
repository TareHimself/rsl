namespace rsl.Parser;

/// <summary>
///     <see cref="Identifier" />( <see cref="Arguments" /> )
/// </summary>
public class CallNode : Node
{
    public Node[] Arguments;
    public string Identifier;

    public CallNode(string identifier, IEnumerable<Node> arguments) : base(ENodeType.Call)
    {
        Identifier = identifier;
        Arguments = arguments.ToArray();
    }

    public override IEnumerable<Node> GetChildren()
    {
        return Arguments;
    }
}