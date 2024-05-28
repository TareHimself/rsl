using System.Reflection;

namespace ashl.Tokenizer;

public class Token : TokenBase
{
    private static readonly Dictionary<string, TokenType> _keywordToTokenTypeMap = BuildTokenMap();

    public string Value;


    public Token(TokenType type, uint line, uint col) : this(type, new TokenDebugInfo(line, col))
    {
    }

    public Token(TokenType type, TokenDebugInfo debugInfo)
    {
        Type = type;
        Value = "";
        DebugInfo = debugInfo;
    }

    public Token(RawToken rawToken)
    {
        Type = rawToken.Type;
        Value = rawToken.Data;
        DebugInfo = rawToken.DebugInfo;
    }

    public Token(TokenType type, RawToken rawToken)
    {
        Type = type;
        Value = rawToken.Data;
        DebugInfo = rawToken.DebugInfo;
    }


    public static string? TokenTypeToKeyword(TokenType type)
    {
        var enumType = typeof(TokenType);
        var memberInfos =
            enumType.GetMember(type.ToString());
        var enumValueMemberInfo = memberInfos.FirstOrDefault(m =>
            m.DeclaringType == enumType);
        var valueAttribute =
            (KeywordTokenAttribute?)enumValueMemberInfo?
                .GetCustomAttribute(typeof(KeywordTokenAttribute), false);
        return valueAttribute?.Keyword;
    }

    public static Dictionary<string, TokenType> BuildTokenMap()
    {
        var tokenMap = new Dictionary<string, TokenType>();

        foreach (var token in (TokenType[])Enum.GetValues(typeof(TokenType)))
        {
            var tok = TokenTypeToKeyword(token);
            if (tok == null) continue;
            tokenMap.Add(tok, token);
        }

        return tokenMap;
    }

    public static TokenType? KeywordToTokenType(string keyword)
    {
        return _keywordToTokenTypeMap.TryGetValue(keyword, out var tokType) ? tokType : null;
    }

    public static implicit operator Token(RawToken raw)
    {
        return new Token(raw.Type, raw);
    }
}