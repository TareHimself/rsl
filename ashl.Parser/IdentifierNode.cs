namespace ashl.Parser;

/// <summary>
/// <see cref="Identity"/>
/// </summary>
public class IdentifierNode : Node
{
    public string Identity;
    public IdentifierNode(string identity) : base(ENodeType.Identifier)
    {
        Identity = identity;
    }
}