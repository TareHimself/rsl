import Token, { ETokenType, isType, TOKEN_SIZES, TOKEN_TO_KEYWORDS, TokenDebugInfo } from "./token";
import TokenList from "./TokenList";

export function joinTokensTill(data: TokenList, ...tokens: string[]): Token | undefined {

    if (data.empty()) return undefined;

    const tokensSet = new Set<string>(tokens)
    if (tokensSet.has(data.front().value)) return undefined;

    const pending = data[0];

    data.removeFront();
    while (data.notEmpty() && !tokensSet.has(data[0].value)) {
        pending.value += data[0].value;
        pending.debug = pending.debug.combine(data[0].debug);
        data.removeFront();
    }

    return pending;
}

export function isSplitToken(token: Token)
    {
        switch (token.type)
        {
        case ETokenType.OpenBrace:
        case ETokenType.OpenParen:
        case ETokenType.OpenBracket:
        case ETokenType.CloseBrace:
        case ETokenType.CloseParen:
        case ETokenType.CloseBracket:
        case ETokenType.Comma:
        case ETokenType.StatementEnd:
        case ETokenType.Access:
            return true;
        default:
            return false;
        }
    }

export function isSeparatorToken(token: Token)
{
    return isSplitToken(token) || token.value == " " || token.value == "\n" || token.value == "\r";
}

function isInteger(data: string){
    return Number.isNaN(parseInt(data))
}

export function tokenize(data: string, filePath: string): TokenList {

    const rawTokens = new TokenList(data.split('\n').map((c, idx) => {
        const lineNo = idx + 1;
        return c.trim().split('').map((c, colIdx) => {
            const colNo = colIdx + 1;
            return Token.FromData(c, new TokenDebugInfo(filePath, lineNo, colNo, lineNo, colNo + c.length));
        })
    }).flat());

    const result = new TokenList();

    while(rawTokens.notEmpty())
        {
            const curToken = rawTokens.front();
            
            if(curToken.value == " " || curToken.value == "\n" || curToken.value == "\r")
            {
                rawTokens.removeFront();
                continue;
            }

            if(curToken.value == "\"" || curToken.value == "\'")
            {
                const tok = rawTokens.removeFront();
                const consumed = joinTokensTill(rawTokens,tok.value);
                rawTokens.removeFront();
                if(consumed !== undefined)
                {
                    result.insertBack(new Token(ETokenType.StringLiteral,consumed.value,consumed.debug));
                }
                continue;
            }

            if(curToken.value == "/")
            {
                rawTokens.removeFront();
                
                if(rawTokens.notEmpty())
                {
                    const nextToken = rawTokens.front();
                    if(nextToken.value == "/")
                    {
                        rawTokens.removeFront();
                        const startLine = nextToken.debug.startLine;
                        let combined = rawTokens.removeFront();
                        while(rawTokens.notEmpty() && rawTokens.front().debug.startLine == startLine)
                        {
                            const next = rawTokens.removeFront();
                            combined = Token.FromData(combined.value + next.value,combined.debug.combine(next.debug));
                        }
                        continue;
                    }

                    if(nextToken.value == "*")
                    {
                        rawTokens.removeFront();
                        joinTokensTill(rawTokens,"*/");
                        continue;
                    }
                }

                rawTokens.insertFront(curToken);
            }

            const maxSize = TOKEN_SIZES[0][0];

            let combinedStr = "";
            const searchTokens = new TokenList()

            while(combinedStr.length < maxSize && rawTokens.notEmpty())
            {
                const front = rawTokens.front();
                searchTokens.insertBack(rawTokens.removeFront());
                combinedStr += front.value;
            }

            let matchedSize = false;

            for(const entry of TOKEN_SIZES)
            {
                const [size,matches] = entry;

                while(combinedStr.length > size)
                {
                    rawTokens.insertFront(searchTokens.back());
                    combinedStr = combinedStr.substr(0,combinedStr.length - searchTokens.back().value.length);
                    searchTokens.removeBack();
                }

                if(combinedStr.length < size) continue;

                if(matches.has(combinedStr) && (isSeparatorToken(Token.FromData(combinedStr,TokenDebugInfo.empty())) ? true :  (rawTokens.empty() ? true : isSeparatorToken(rawTokens.front()))))
                {
                    matchedSize = true;
                    break;
                }
            }

             if(matchedSize)
             {
                 let debugSpan = searchTokens.front().debug;
                 searchTokens.removeFront();
                 while(!searchTokens.empty())
                 {
                     debugSpan = debugSpan.combine(searchTokens.front().debug);
                     searchTokens.removeFront();
                 }

                 result.insertBack(Token.FromData(combinedStr,debugSpan));
             }
             else
             {
                 if(!searchTokens.empty())
                {
                    while(!searchTokens.empty())
                    {
                        rawTokens.insertFront(searchTokens.back());
                        searchTokens.removeBack();
                    }
                }
                
                combinedStr = "";

                searchTokens.insertBack(rawTokens.removeFront());
                combinedStr += searchTokens.back().value;
                let debugSpan = searchTokens.front().debug;

                if(isInteger(searchTokens.front().value))
                {
                    while(rawTokens.notEmpty() && isInteger(rawTokens.front().value))
                    {
                        const tok = rawTokens.removeFront();
                        combinedStr += tok.value;
                        debugSpan = debugSpan.combine(tok.debug);
                        searchTokens.insertBack(tok);
                    }

                    if(rawTokens.notEmpty() && rawTokens.front().value == ".")
                    {
                        let tok = rawTokens.removeFront();
                        combinedStr += tok.value;
                        debugSpan = debugSpan.combine(tok.debug);
                        searchTokens.insertBack(tok);
                        
                        while(rawTokens.notEmpty() && isInteger(rawTokens.front().value))
                        {
                            tok = rawTokens.removeFront();
                            combinedStr += tok.value;
                            debugSpan = debugSpan.combine(tok.debug);
                            searchTokens.insertBack(tok);
                        }
                    }

                    result.insertBack(new Token(ETokenType.Numeric,combinedStr,debugSpan));
                    continue;
                }

                while(rawTokens.notEmpty() && !isSeparatorToken(rawTokens.front()))
                {
                    const tok = rawTokens.removeFront();
                    combinedStr += tok.value;
                    debugSpan = debugSpan.combine(tok.debug);
                    
                    searchTokens.insertBack(tok);
                }
                
                result.insertBack(Token.FromData(combinedStr,debugSpan));
                searchTokens.removeFront();
            }
        }
        
        return result;
}