namespace ashl.Parser;

/// <summary>
///     <see cref="Identifier" />( <see cref="Arguments" /> )
/// </summary>
public class CallNode : Node
{
    public Node[] Arguments;
    public IdentifierNode Identifier;

    public CallNode(IdentifierNode identifier, IEnumerable<Node> arguments) : base(ENodeType.Call)
    {
        Identifier = identifier;
        Arguments = arguments.ToArray();
    }

    public override IEnumerable<Node> GetChildren()
    {
        return [Identifier, ..Arguments];
    }
}