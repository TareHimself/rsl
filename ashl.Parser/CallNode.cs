namespace ashl.Parser;

/// <summary>
/// <see cref="Identifier"/>( <see cref="Arguments"/> )
/// </summary>
public class CallNode : Node
{
    public IdentifierNode Identifier;
    public Node[] Arguments;

    public CallNode(IdentifierNode identifier, IEnumerable<Node> arguments) : base(ENodeType.Call)
    {
        Identifier = identifier;
        Arguments = arguments.ToArray();
    }

    public override IEnumerable<Node> GetChildren() => [Identifier,..Arguments];
}