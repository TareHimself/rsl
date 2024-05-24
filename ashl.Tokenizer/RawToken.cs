namespace ashl.Tokenizer;

public partial class RawToken : TokenBase
{
    public string Data;
    
    public RawToken() : this("",new TokenDebugInfo(0,0))
    {
    }
    
    public RawToken(string data, TokenDebugInfo debugInfo)
    {
        Data = data;
        DebugInfo = debugInfo;
        Type = Token.KeywordToTokenType(data) ?? TokenType.Unknown;
    }
}