namespace rsl.Tokenizer;

public class TokenMatchResult(TokenList<RawToken> extra, TokenType type, RawToken token)
{
    public readonly TokenType Type = type;
    public TokenList<RawToken> Extra = extra;
    public RawToken Token = token;
}