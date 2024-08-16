#pragma once
#include <list>
#include "Token.hpp"
namespace ashl {

    class TokenList {
        std::list<Token> _tokens{};
    public:

        TokenList& ExpectFront(ETokenType tokenType);
        TokenList& ExpectBack(ETokenType tokenType);
        TokenList& ExpectFront(const std::vector<ETokenType>& tokenTypes);
        TokenList& ExpectBack(const std::vector<ETokenType>& tokenTypes);
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