namespace ashl.Tokenizer;

public class TokenMatchResult(TokenList<RawToken> extra, TokenType type, RawToken token)
{
    public TokenList<RawToken> Extra = extra;
    public readonly TokenType Type = type;
    public RawToken Token = token;
}