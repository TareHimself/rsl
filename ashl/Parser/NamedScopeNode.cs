using ashl.Tokenizer;

namespace ashl.Parser;

/// <summary>
///     @<see cref="Name">ScopeName</see> {
///     <see cref="ScopeNode.Statements" />
///     }
/// </summary>
public class NamedScopeNode : ScopeNode
{
    public EScopeType ScopeType;

    public NamedScopeNode(TokenType scopeType, IEnumerable<Node> nodes) : base(nodes)
    {
        NodeType = ENodeType.NamedScope;
        ScopeType = scopeType switch
        {
            TokenType.VertexScope => EScopeType.Vertex,
            TokenType.FragmentScope => EScopeType.Fragment,
            _ => throw new ArgumentOutOfRangeException(nameof(scopeType), scopeType, null)
        };
    }
}