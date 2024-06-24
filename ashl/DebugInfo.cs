namespace ashl;
public struct DebugInfo(string file, uint line, uint column)
{
    public string File = file;
    public uint Line = line;
    public uint Column = column;

    public DebugInfo() : this("<unknown>", 0, 0)
    {
    }

    public DebugInfo(uint line, uint column) : this("<unknown>", line, column)
    {
    }
}