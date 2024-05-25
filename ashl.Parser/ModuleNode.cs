
namespace ashl.Parser;

/// <summary>
/// <see cref="Statements"/>
/// </summary>
public class ModuleNode : Node
{
    public string FilePath;
    public Node[] Statements;

    public ModuleNode(string filePath,IEnumerable<Node> statements) : base(ENodeType.Module)
    {
        FilePath = filePath;
        Statements = statements.ToArray();
    }


    public void ResolveIncludes(Func<IncludeNode,ModuleNode,string> resolver,Tokenizer.Tokenizer tokenizer,Parser parser)
    {
        var newStatements = Statements.ToList();
        var includedFiles = new HashSet<string>();
        for(var i = 0; i < newStatements.Count; i++)
        {
            if (newStatements[i] is not IncludeNode includeNode) continue;
            
            var absPath = resolver(includeNode, this);
            
            if (!includedFiles.Add(absPath))
            {
                newStatements.RemoveAt(i);
                i--;
                continue;
            }

            var tokens = tokenizer.Run(absPath);
            var ast = parser.Run(tokens);
            newStatements.RemoveAt(i);
            newStatements.InsertRange(i,ast.Statements);
            i--;
        }

        Statements = newStatements.ToArray();
    }

    public override IEnumerable<Node> GetChildren() => Statements;
}
