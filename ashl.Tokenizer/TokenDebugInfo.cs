namespace ashl.Tokenizer;

public struct TokenDebugInfo(string file, uint line, uint column)
{
    public string File = file;
    public uint Line = line;
    public uint Column = column;

    public TokenDebugInfo() : this("<unknown>",0,0)
    {
    }

    public TokenDebugInfo(uint line, uint column) : this("<unknown>",line,column)
    {
    }
}