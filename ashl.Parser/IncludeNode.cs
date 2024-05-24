namespace ashl.Parser;
/// <summary>
/// #include <see cref="File"/>
/// </summary>
public class IncludeNode : Node
{
    public string SourceFile;
    public string File;
    public IncludeNode(string sourceFile,string file) : base(ENodeType.Include)
    {
        SourceFile = sourceFile;
        File = file;
    }
}