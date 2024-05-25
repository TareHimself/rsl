using System.Collections;

namespace ashl.Tokenizer;

public class Tokenizer
{
    public static TokenList<RawToken>? SkipToNext(TokenList<RawToken> input, string search)
    {
        var extraTokens = input.CreateEmpty();
        while (input.NotEmpty())
        {
            var test = input.Front();


            if (test.Data == search)
            {
                return extraTokens;
            }

            input.RemoveFront();

            extraTokens.InsertBack(test);
        }

        input.InsertFront(extraTokens);

        return null;
    }

    public static TokenMatchResult? SkipToNextToken(TokenList<RawToken> input)
    {
        var extra = input.CreateEmpty();
        while (input.NotEmpty())
        {
            var test = input.Front();

            var tokType = Token.KeywordToTokenType(test.Data);
            if (tokType != null)
            {
                return new TokenMatchResult(extra, tokType.Value, test);
            }

            input.RemoveFront();

            extra.InsertBack(test);
        }

        input.InsertFront(extra);

        return null;
    }

    public static void ConsumeTill(TokenList<RawToken> result, TokenList<RawToken> input, string target, int scope)
    {
        var workingScope = scope;

        do
        {
            var tok = input.Front();

            input.RemoveFront();


            switch (tok.Type)
            {
                case TokenType.OpenParen or TokenType.OpenBrace or TokenType.OpenBracket:
                    workingScope++;
                    break;
                case TokenType.CloseParen or TokenType.CloseBrace or TokenType.CloseBracket:
                    workingScope--;
                    break;
            }

            result.InsertBack(tok);

            if (workingScope == 0 && tok.Data == target)
            {
                break;
            }
        } while (input.NotEmpty());
    }

    public static string ConsumeTill(TextStream data, params char[] tokens)
    {
        var tokenSet = tokens.ToHashSet();
        var pending = "";
        var escaped = false;

        while (!data.IsEmpty() && data.Get(out var tok))
        {
            pending += tok;

            if (tokenSet.Contains(tok) && !escaped)
            {
                return pending;
            }

            escaped = tok == '\\';
        }

        return pending;
    }

    public static string ConsumeTill(TextStream data, string token)
    {
        var window = "";
        var pending = "";
        var escaped = false;

        while (!data.IsEmpty() && data.Get(out var tok))
        {
            pending += tok;
            window += tok;

            if (window.Length > token.Length)
            {
                window = window.Substring(1, window.Length - 1);
            }

            if (window == token && !escaped)
            {
                return pending;
            }

            escaped = tok == '\\';
        }

        return pending;
    }

    public void PreProcess(TokenList<RawToken> tokens, TextStream data)
    {
        var pending = new RawToken();

        void StorePending()
        {
            if (pending.Data.Length == 0) return;

            tokens.InsertBack(new RawToken(pending.Data, pending.DebugInfo));
            pending.Data = "";
        }

        while (data.Get(out var tok))
        {
            var debugInfo = new TokenDebugInfo(data.SourcePath, data.LineNumber, data.ColumnNumber);

            switch (tok)
            {
                case ' ' or '\n' or '\r':
                    StorePending();
                    break;
                case '\"':
                    {
                        StorePending();
                        var consumed = pending.Data + ConsumeTill(data, '\"');
                        consumed = consumed[..^1];
                        pending.Data = "";
                        tokens.InsertBack(new RawToken($"\'{consumed}\'", debugInfo));
                        break;
                    }
                case '\'':
                    {
                        StorePending();
                        var consumed = pending.Data + ConsumeTill(data, '\'');
                        consumed = consumed[..^1];
                        pending.Data = "";
                        tokens.InsertBack(new RawToken($"\'{consumed}\'", debugInfo));
                        break;
                    }
                case '/':
                    {
                        if (data.Peak() == '/')
                        {
                            var comment = data.GetRemainingOnLine();
                            continue;
                        }

                        if (data.Peak() == '*')
                        {
                            var comment = ConsumeTill(data, "*/");
                            continue;
                        }

                        if (pending.Data.Length > 0)
                        {
                            pending.Data += tok;
                        }
                        else
                        {
                            pending = new RawToken(tok.ToString(), debugInfo);
                        }
                    }
                    break;
                default:
                    var tokType = Token.KeywordToTokenType(tok.ToString());
                    if (tokType is not null and not TokenType.Unknown){
                        if(tokType is TokenType.Access && int.TryParse(pending.Data, out var _))
                        {
                            pending.Data += tok;
                        }
                        else
                        {
                            StorePending();
                            tokens.InsertBack(new RawToken(tok.ToString(), debugInfo));
                        }
                    } 
                    else if (pending.Data.Length > 0)
                    {
                        pending.Data += tok;
                    }
                    else
                    {
                        pending = new RawToken(tok.ToString(), debugInfo);
                    }

                    break;
            }
        }

        if (pending.Data.Length > 0)
        {
            tokens.InsertBack(pending);
            pending.Data = "";
        }
    }

    public virtual TokenList<Token> Run(string[] data, string filePath)
    {
        var rawTokens = new TokenList<RawToken>();
        var textStream = new TextStream(data, filePath);
        PreProcess(rawTokens, textStream);
        var inTokens = new TokenList<Token>();
        TokenizeRoot(inTokens, rawTokens);
        return inTokens;
    }


    public virtual TokenList<Token> Run(string filePath)
    {
        var rawTokens = new TokenList<RawToken>();
        var textStream = new TextStream(filePath);
        PreProcess(rawTokens, textStream);
        var inTokens = new TokenList<Token>();
        TokenizeRoot(inTokens, rawTokens);
        return inTokens;
    }

    public virtual void TokenizeDeclaration(TokenList<Token> output, TokenList<RawToken> input)
    {
        
        if (input.Front().Type == TokenType.Const)
        {
            output.InsertBack(input.RemoveFront());
        }
        if (input.Front().Type is TokenType.DataIn or TokenType.DataOut)
        {
            output.InsertBack(input.RemoveFront());
        }
        var declarationType = input.RemoveFront();
        var declarationName = input.RemoveFront();
        var declarationTypeCount = "1";
        if (input.NotEmpty() && input.Front().Type == TokenType.OpenBracket)
        {
            input.ExpectFront(TokenType.OpenBracket).RemoveFront();
            if (input.Front().Type != TokenType.CloseBracket)
            {
                declarationTypeCount = input.RemoveFront().Data;
            }
            else
            {
                declarationTypeCount = "0";
            }
            input.ExpectFront(TokenType.CloseBracket).RemoveFront();
        }

        if (declarationType.Type == TokenType.Unknown) declarationType.Type = TokenType.Identifier;

        output
            .InsertBack(new Token(declarationType))
            .InsertBack(new Token(TokenType.Identifier, declarationName))
            .InsertBack(new Token(TokenType.DeclarationCount, declarationName.DebugInfo)
            {
                Value = declarationTypeCount
            });
    }
    
    public virtual void TokenizeFunctionArgument(TokenList<Token> output, TokenList<RawToken> input)
    {
        if (input.Front().Type is TokenType.DataIn or TokenType.DataOut)
        {
            output.InsertBack(input.RemoveFront());
        }
        
        var declarationType = input.RemoveFront();
        
        var declarationTypeCount = "1";
        
        if (input.Front().Type == TokenType.OpenBracket)
        {
            input.ExpectFront(TokenType.OpenBracket).RemoveFront();
            declarationTypeCount = input.RemoveFront().Data;
            input.ExpectFront(TokenType.CloseBracket).RemoveFront();
        }
        
        var declarationName = input.RemoveFront();
        
        if (declarationType.Type == TokenType.Unknown) declarationType.Type = TokenType.Identifier;

        output
            .InsertBack(new Token(declarationType))
            .InsertBack(new Token(TokenType.Identifier, declarationName))
            .InsertBack(new Token(TokenType.DeclarationCount, declarationName.DebugInfo)
            {
                Value = declarationTypeCount
            });
    }

    public virtual void TokenizeExpression(TokenList<Token> output, TokenList<RawToken> input)
    {
        while (input.NotEmpty())
        {
            if (input.Front().Type is TokenType.TypeFloat or TokenType.TypeInt or TokenType.TypeVec2f
                or TokenType.TypeVec2i
                or TokenType.TypeVec3f or TokenType.TypeVec3i or TokenType.TypeVec4f or TokenType.TypeVec4i
                or TokenType.TypeMat3 or TokenType.TypeMat4 or TokenType.Unknown)
            {
                var lastFront = input.RemoveFront();
                if (input.NotEmpty() && input.Front().Type is TokenType.Unknown or TokenType.Identifier)
                {
                    input.InsertFront(lastFront);
                    TokenizeDeclaration(output,input);
                    continue;
                }
                
                input.InsertFront(lastFront);
            }
            
            output.InsertBack(input.Front().Type is TokenType.Unknown
                ? new Token(TokenType.Identifier, input.RemoveFront())
                : input.RemoveFront());
        }
    }

    public virtual void TokenizeStatement(TokenList<Token> output, TokenList<RawToken> input)
    {
        var statement = input.CreateEmpty();
        ConsumeTill(statement, input, Token.TokenTypeToKeyword(TokenType.StatementEnd) ?? "", 0);
        var end = statement.ExpectBack(TokenType.StatementEnd).RemoveBack();
        TokenizeExpression(output, statement);
        output.InsertBack(end);
    }

    public virtual void TokenizeScope(TokenList<Token> output, TokenList<RawToken> input)
    {
        output.InsertBack(input.ExpectFront(TokenType.OpenBrace).RemoveFront());
        while (input.NotEmpty() && input.Front().Type != TokenType.CloseBrace)
        {
            var front = input.Front();

            switch (front.Type)
            {
                case TokenType.OpenBrace:
                    {
                        TokenizeScope(output, input);
                    }
                    break;
                case TokenType.Return:
                    {
                        output.InsertBack(input.ExpectFront(TokenType.Return).RemoveFront());
                    }
                    break;
                default:
                    {
                        TokenizeStatement(output, input);
                    }
                    break;
            }
        }
        output.InsertBack(input.ExpectFront(TokenType.CloseBrace).RemoveFront());
    }

    public virtual void TokenizeLayout(TokenList<Token> output, TokenList<RawToken> input)
    {
        output.InsertBack(input.ExpectFront(TokenType.Layout).RemoveFront());
        output.InsertBack(input.ExpectFront(TokenType.OpenParen).RemoveFront());
        while (input.Front().Type != TokenType.CloseParen)
        {
            var tag = input.RemoveFront();
            if(input.Front().Type != TokenType.Assign){
                output
                    .InsertBack(new Token(TokenType.Identifier, tag))
                    .InsertBack(new Token(TokenType.Identifier,tag.DebugInfo));
            }
            else
            {
                input.ExpectFront(TokenType.Assign).RemoveFront();
                var value = input.RemoveFront();
                output
                    .InsertBack(new Token(TokenType.Identifier, tag))
                    .InsertBack(new Token(TokenType.Identifier, value));
            }
            
            
            if (input.Front().Type == TokenType.Comma)
            {
                input.RemoveFront();
            }
        }
        output.InsertBack(input.ExpectFront(TokenType.CloseParen).RemoveFront());
        if (input.Front().Type is TokenType.DataOut or TokenType.DataIn or TokenType.Uniform)
        {
            output.InsertBack(input.RemoveFront());
        }
        TokenizeDeclaration(output, input);
        output.InsertBack(input.ExpectFront(TokenType.StatementEnd).RemoveFront());
    }


    public virtual void TokenizePushConstant(TokenList<Token> output, TokenList<RawToken> input)
    {
        output.InsertBack(input.ExpectFront(TokenType.PushConstant).RemoveFront());
        output.InsertBack(input.ExpectFront(TokenType.OpenParen).RemoveFront());
        while (input.Front().Type != TokenType.CloseParen)
        {
            var tag = input.RemoveFront();
            if(input.Front().Type != TokenType.Assign){
                output
                    .InsertBack(new Token(TokenType.Identifier, tag))
                    .InsertBack(new Token(TokenType.Identifier,tag.DebugInfo));
            }
            else
            {
                input.ExpectFront(TokenType.Assign).RemoveFront();
                var value = input.RemoveFront();
                output
                    .InsertBack(new Token(TokenType.Identifier, tag))
                    .InsertBack(new Token(TokenType.Identifier, value));
            }
            
            
            if (input.Front().Type == TokenType.Comma)
            {
                input.RemoveFront();
            }
        }
        output.InsertBack(input.ExpectFront(TokenType.CloseParen).RemoveFront());
        TokenizeStructScope(output,input);
        output.InsertBack(input.ExpectFront(TokenType.StatementEnd).RemoveFront());
    }

    public virtual void TokenizeNamedScope(TokenList<Token> output, TokenList<RawToken> input)
    {
        output.InsertBack(input.ExpectFront(TokenType.VertexScope,TokenType.FragmentScope).RemoveFront());
        output.InsertBack(input.ExpectFront(TokenType.OpenBrace).RemoveFront());
        while (input.NotEmpty() && input.Front().Type != TokenType.CloseBrace)
        {
            var front = input.Front();

            switch (front.Type)
            {
                case TokenType.Layout:
                    {
                        TokenizeLayout(output, input);
                    }
                    break;

                    case TokenType.PushConstant:
                    {
                        TokenizePushConstant(output, input);
                    }
                    break;

                case TokenType.Include:
                {
                    TokenizeInclude(output,input);
                    
                }
                    break;
                case TokenType.TypeStruct:
                {
                    TokenizeStruct(output, input);
                }
                    break;
                case TokenType.Const:
                {
                    TokenizeDeclaration(output,input);
                    output.InsertBack(input.ExpectFront(TokenType.Assign).RemoveFront());
                    TokenizeStatement(output,input);
                }
                    break;
                
                
                case TokenType.TypeFloat or TokenType.TypeVec2f or TokenType.TypeVec3f or TokenType.TypeVec4f
                    or TokenType.TypeInt or TokenType.TypeVec2i or TokenType.TypeVec3i or TokenType.TypeVec4i
                    or TokenType.TypeMat3 or TokenType.TypeMat4 or TokenType.TypeVoid or TokenType.TypeBoolean or TokenType.Unknown:
                {
                    TokenizeFunction(output, input);
                }
                    break;
                default:
                    throw input.CreateException("Unexpected Token", input.Front());
                    break;
            }
        }
        output.InsertBack(input.ExpectFront(TokenType.CloseBrace).RemoveFront());
    }

    public virtual void TokenizeFunction(TokenList<Token> output, TokenList<RawToken> input)
    {
        var type = input.RemoveFront();
        var typeSize = "1";
        var typeSizeDebug = type.DebugInfo;
        if (input.Front().Type == TokenType.OpenBracket)
        {
            input.ExpectFront(TokenType.OpenBracket).RemoveFront();
            var typeSizeToken = input.RemoveFront();
            typeSizeDebug = typeSizeToken.DebugInfo;
            typeSize = typeSizeToken.Data;
            
            input.ExpectFront(TokenType.CloseBracket).RemoveFront();
        }

        if (type.Type == TokenType.Unknown) type.Type = TokenType.Identifier;
        
        output
            .InsertBack(new Token(TokenType.Function, type))
            .InsertBack(new Token(TokenType.DeclarationCount, typeSizeDebug)
            {
                Value = typeSize
            });

        var name = input.RemoveFront();

        output.InsertBack(new Token(TokenType.Identifier, name));

        output.InsertBack(input.ExpectFront(TokenType.OpenParen).RemoveFront());

        while (input.NotEmpty() && input.Front().Type != TokenType.CloseParen)
        {
            TokenizeFunctionArgument(output, input);
            if (input.Front().Type == TokenType.Comma)
            {
                input.RemoveFront();
            }
        }

        output.InsertBack(input.ExpectFront(TokenType.CloseParen).RemoveFront());

        TokenizeScope(output, input);
    }

    public virtual void TokenizeStructScope(TokenList<Token> output, TokenList<RawToken> input)
    {
        output.InsertBack(input.ExpectFront(TokenType.OpenBrace).RemoveFront());

        while (input.Front().Type != TokenType.CloseBrace)
        {
            TokenizeDeclaration(output, input);

            output.InsertBack(input.ExpectFront(TokenType.StatementEnd)
                    .RemoveFront());
        }

        output.InsertBack(input.ExpectFront(TokenType.CloseBrace).RemoveFront());
    }

    public virtual void TokenizeStruct(TokenList<Token> output, TokenList<RawToken> input)
    {
        output.InsertBack(input.ExpectFront(TokenType.TypeStruct).RemoveFront());
        var id = input.ExpectFront(TokenType.Unknown).RemoveFront();
        output.InsertBack(new Token(TokenType.Identifier, id));
        TokenizeStructScope(output, input);
        input.ExpectFront(TokenType.StatementEnd).RemoveFront();
    }

    public virtual void TokenizeInclude(TokenList<Token> output, TokenList<RawToken> input)
    {
        output.InsertBack(input.ExpectFront(TokenType.Include).RemoveFront());
        var includeFile = input.ExpectFront(TokenType.Unknown).RemoveFront();
        includeFile.Data = includeFile.Data.Substring(1, includeFile.Data.Length - 2).Trim();
        output.InsertBack(new Token(TokenType.Identifier, includeFile));
        input.ExpectFront(TokenType.StatementEnd).RemoveFront();
    }

    public virtual void TokenizeRoot(TokenList<Token> output, TokenList<RawToken> input)
    {
        while (input.NotEmpty())
        {
            switch (input.Front().Type)
            {
                case TokenType.Include:
                {
                    TokenizeInclude(output,input);
                    
                }
                    break;
                case TokenType.Layout:
                    {
                        TokenizeLayout(output, input);
                    }
                    break;
                case TokenType.PushConstant:
                {
                    TokenizePushConstant(output, input);
                }
                break;
                case TokenType.TypeStruct:
                {
                    TokenizeStruct(output, input);
                }
                    break;
                case TokenType.Const:
                {
                    TokenizeDeclaration(output,input);
                    output.InsertBack(input.ExpectFront(TokenType.Assign).RemoveFront());
                    TokenizeStatement(output,input);
                }
                    break;
                
                case TokenType.TypeFloat or TokenType.TypeVec2f or TokenType.TypeVec3f or TokenType.TypeVec4f
                    or TokenType.TypeInt or TokenType.TypeVec2i or TokenType.TypeVec3i or TokenType.TypeVec4i
                    or TokenType.TypeMat3 or TokenType.TypeMat4 or TokenType.TypeVoid or TokenType.TypeBoolean or TokenType.Unknown:
                {
                    TokenizeFunction(output, input);
                }
                    break;
                case TokenType.VertexScope or TokenType.FragmentScope:
                {
                    TokenizeNamedScope(output, input);
                }
                    break;
                default:
                    throw input.CreateException("Unexpected token", input.Front());
            }
        }
    }
}