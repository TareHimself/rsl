namespace rsl.Tokenizer;

public class RawToken : TokenBase
{
    public string Data;

    public RawToken() : this("", new DebugInfo(0, 0))
    {
    }

    public RawToken(string data, DebugInfo debugInfo)
    {
        Data = data;
        DebugInfo = debugInfo;
        Type = Token.KeywordToTokenType(data) ?? TokenType.Unknown;
    }
}