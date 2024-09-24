#pragma once
#include <list>
#include "Token.hpp"

namespace rsl
{
    class TokenList
    {
        std::list<Token> _tokens{};

    public:
        TokenList& ExpectFront(TokenType tokenType);
        TokenList& ExpectBack(TokenType tokenType);
        TokenList& ExpectFront(const std::vector<TokenType>& tokenTypes);
        TokenList& ExpectBack(const std::vector<TokenType>& tokenTypes);
        Token RemoveFront();
        Token RemoveBack();
        Token& Front();
        Token& Back();
        TokenList& InsertFront(const Token& token);
        TokenList& InsertBack(const Token& token);

        bool Empty() const;
        bool NotEmpty() const;
    };
}
