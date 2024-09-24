namespace rsl.Parser;

/// <summary>
///     #include <see cref="File" />
/// </summary>
public class IncludeNode : Node
{
    public string File;
    public string SourceFile;

    public IncludeNode(string sourceFile, string file) : base(ENodeType.Include)
    {
        SourceFile = sourceFile;
        File = file;
    }

    public override IEnumerable<Node> GetChildren() => [];
}