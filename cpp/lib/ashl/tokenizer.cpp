
#include "ashl/tokenizer.hpp"

#include <fstream>
#include <ranges>
#include <sstream>
#include "ashl/utils.hpp"

namespace ashl
{
    std::optional<Token> joinTokensTill(TokenList& tokens, const std::set<std::string>& search)
    {
        if(tokens.Empty()) return {};

        if(search.contains(tokens.Front().value)) return {};

        auto pending = tokens.Front();
        tokens.RemoveFront();
        while(tokens.NotEmpty() && !search.contains(tokens.Front().value))
        {
            auto front = tokens.Front();
            pending = Token{pending.value + front.value,pending.debugInfo + front.debugInfo};
            tokens.RemoveFront();
        }

        return pending;
    }

    bool isSplitToken(const Token& token)
    {
        switch (token.type)
        {
        case ETokenType::OpenBrace:
        case ETokenType::OpenParen:
        case ETokenType::OpenBracket:
        case ETokenType::CloseBrace:
        case ETokenType::CloseParen:
        case ETokenType::CloseBracket:
        case ETokenType::Comma:
        case ETokenType::StatementEnd:
        case ETokenType::Access:
            return true;
        case ETokenType::Unknown:
        case ETokenType::Assign:
        case ETokenType::OpAnd:
        case ETokenType::OpOr:
        case ETokenType::OpNot:
        case ETokenType::OpIncrement:
        case ETokenType::OpDecrement:
        case ETokenType::OpAddAssign:
        case ETokenType::OpSubtractAssign:
        case ETokenType::OpDivideAssign:
        case ETokenType::OpMultiplyAssign:
        case ETokenType::OpAdd:
        case ETokenType::OpSubtract:
        case ETokenType::OpDivide:
        case ETokenType::OpMultiply:
        case ETokenType::OpMod:
        case ETokenType::OpEqual:
        case ETokenType::OpNotEqual:
        case ETokenType::OpLess:
        case ETokenType::OpGreater:
        case ETokenType::OpLessEqual:
        case ETokenType::OpGreaterEqual:
        case ETokenType::Identifier:
        case ETokenType::Function:
        case ETokenType::Return:
        case ETokenType::BooleanLiteral:
        case ETokenType::For:
        case ETokenType::Continue:
        case ETokenType::Break:
        case ETokenType::DeclarationCount:
        case ETokenType::TypeStruct:
        case ETokenType::TypeFloat:
        case ETokenType::TypeFloat2:
        case ETokenType::TypeFloat3:
        case ETokenType::TypeFloat4:
        case ETokenType::TypeInt:
        case ETokenType::TypeInt2:
        case ETokenType::TypeInt3:
        case ETokenType::TypeInt4:
        case ETokenType::TypeMat3:
        case ETokenType::TypeMat4:
        case ETokenType::TypeBoolean:
        case ETokenType::TypeVoid:
        case ETokenType::TypeSampler2D:
        case ETokenType::TypeBuffer:
        case ETokenType::DataIn:
        case ETokenType::DataOut:
        case ETokenType::Layout:
        case ETokenType::Uniform:
        case ETokenType::ReadOnly:
        case ETokenType::Discard:
        case ETokenType::Include:
        case ETokenType::Define:
        case ETokenType::Const:
        case ETokenType::PushConstant:
        case ETokenType::If:
        case ETokenType::Else:
        case ETokenType::Conditional:
        case ETokenType::Colon:
        case ETokenType::Numeric:
        case ETokenType::VertexScope:
        case ETokenType::FragmentScope:
        default:
            return false;
        }
    }

    bool isSeparatorToken(const Token& token)
    {
        return isSplitToken(token) || token.value == " " || token.value == "\n" || token.value == "\r";
    }

    TokenList preprocess(const std::string& fileName, const std::string& fileData)
    {
        TokenList rawTokens{};

        {
            std::stringstream dStream{fileData};
            uint32_t lineNo = 1;
            std::string line{};
            while(std::getline(dStream,line))
            {
                uint32_t colNo = 1;
                for (const char& a : line)
                {
                    rawTokens.InsertBack({std::string{a},TokenDebugInfo{fileName,lineNo,colNo}});
                    colNo++;
                }
                lineNo++;
            }
        }

        TokenList result{};
        
        while(rawTokens.NotEmpty())
        {
            auto curToken = rawTokens.Front();
            
            if(curToken.value == " " || curToken.value == "\n" || curToken.value == "\r")
            {
                rawTokens.RemoveFront();
                continue;
            }

            if(curToken.value == "\"" || curToken.value == "\'")
            {
                auto tok = rawTokens.RemoveFront();
                auto consumed = joinTokensTill(rawTokens,std::set{tok.value});
                rawTokens.RemoveFront();
                if(consumed.has_value())
                {
                    auto consumedTok = consumed.value();
                    
                    result.InsertBack({ETokenType::StringLiteral,consumedTok.value,consumedTok.debugInfo});
                }
                continue;
            }

            // if(isSplitToken(curToken))
            // {
            //     rawTokens.RemoveFront();
            //     result.InsertBack(curToken);
            //     continue;
            // }

            if(curToken.value == "/")
            {
                rawTokens.RemoveFront();
                
                if(rawTokens.NotEmpty())
                {
                    auto nextToken = rawTokens.Front();
                    if(nextToken.value == "/")
                    {
                        rawTokens.RemoveFront();
                        auto startLine = nextToken.debugInfo.startLine;
                        auto combined = rawTokens.RemoveFront();
                        while(rawTokens.NotEmpty() && rawTokens.Front().debugInfo.startLine == startLine)
                        {
                            auto next = rawTokens.RemoveFront();
                            combined = Token{combined.value + next.value,combined.debugInfo + next.debugInfo};
                        }
                        continue;
                    }

                    if(nextToken.value == "*")
                    {
                        rawTokens.RemoveFront();
                        joinTokensTill(rawTokens,std::set<std::string>{"*/"});
                        continue;
                    }
                }

                rawTokens.InsertFront(curToken);
            }

            auto maxSize = std::ranges::reverse_view(Token::SIZES_TO_KEYWORDS).begin()->first;

            std::string combinedStr{};
            std::list<Token> searchTokens{};

            while(combinedStr.size() < static_cast<std::string::size_type>(maxSize) && rawTokens.NotEmpty())
            {
                auto front = rawTokens.Front();
                searchTokens.push_back(rawTokens.RemoveFront());
                combinedStr += front.value;
            }

            auto matchedSize = false;

            for(const auto& [size,matches] : std::ranges::reverse_view(Token::SIZES_TO_KEYWORDS))
            {
                while(static_cast<int>(combinedStr.size()) > size)
                {
                    rawTokens.InsertFront(searchTokens.back());
                    combinedStr = combinedStr.substr(0,combinedStr.size() - searchTokens.back().value.size());
                    searchTokens.pop_back();
                }

                if(static_cast<int>(combinedStr.size()) < size) continue;

                if(matches.contains(combinedStr) && (isSeparatorToken(Token{combinedStr,{}}) ? true :  (rawTokens.Empty() ? true : isSeparatorToken(rawTokens.Front()))))
                {
                    matchedSize = true;
                    break;
                }
            }

             if(matchedSize)
             {
                 auto debugSpan = searchTokens.front().debugInfo;
                 searchTokens.pop_front();
                 while(!searchTokens.empty())
                 {
                     debugSpan += searchTokens.front().debugInfo;
                     searchTokens.pop_front();
                 }

                 result.InsertBack({combinedStr,debugSpan});
             }
             else
             {
                 if(!searchTokens.empty())
                {
                    while(!searchTokens.empty())
                    {
                        rawTokens.InsertFront(searchTokens.back());
                        searchTokens.pop_back();
                    }
                }
                
                combinedStr.clear();

                searchTokens.push_back(rawTokens.RemoveFront());
                combinedStr += searchTokens.back().value;
                auto debugSpan = searchTokens.front().debugInfo;

                if(isInteger(searchTokens.front().value))
                {
                    while(rawTokens.NotEmpty() && isInteger(rawTokens.Front().value))
                    {
                        auto tok = rawTokens.RemoveFront();
                        combinedStr += tok.value;
                        debugSpan += tok.debugInfo;
                        searchTokens.push_back(tok);
                    }

                    if(rawTokens.NotEmpty() && rawTokens.Front().value == ".")
                    {
                        auto tok = rawTokens.RemoveFront();
                        combinedStr += tok.value;
                        debugSpan += tok.debugInfo;
                        searchTokens.emplace_back(tok);
                        
                        while(rawTokens.NotEmpty() && isInteger(rawTokens.Front().value))
                        {
                            tok = rawTokens.RemoveFront();
                            combinedStr += tok.value;
                            debugSpan += tok.debugInfo;
                            searchTokens.push_back(tok);
                        }
                    }

                    result.InsertBack(Token{ETokenType::Numeric,combinedStr,debugSpan});
                    continue;
                }

                while(rawTokens.NotEmpty() && !isSeparatorToken(rawTokens.Front()))
                {
                    auto tok = rawTokens.RemoveFront();
                    combinedStr += tok.value;
                    debugSpan += tok.debugInfo;
                    
                    searchTokens.push_back(tok);
                }
                
                result.InsertBack(Token{combinedStr,debugSpan});
                searchTokens.pop_front();
            }
        }
        
        return result;
    }

    TokenList tokenize(const std::string& fileName, const std::string& fileData)
    {
        return preprocess(fileName,fileData);
    }

    TokenList tokenize(const std::string& fileName)
    {
        std::ifstream fileStream(fileName, std::ios::binary);
        const std::string fileContent((std::istreambuf_iterator<char>(fileStream)),
                                      std::istreambuf_iterator<char>());
        return tokenize(fileName,fileContent);
    }
}
