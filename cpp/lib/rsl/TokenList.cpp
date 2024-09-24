#include "rsl/TokenList.hpp"

namespace rsl
{
    TokenList& TokenList::ExpectFront(const TokenType tokenType)
    {
        if (const auto& a = Front(); a.type != tokenType)
        {
            throw std::exception("Unexpected token");
        }

        return *this;
    }

    TokenList& TokenList::ExpectBack(const TokenType tokenType)
    {
        if (const auto& a = Back(); a.type != tokenType)
        {
            throw std::exception("Unexpected token");
        }

        return *this;
    }

    TokenList& TokenList::ExpectFront(const std::vector<TokenType>& tokenTypes)
    {
        if (const auto& a = Front(); std::find(tokenTypes.begin(), tokenTypes.end(), a.type) != tokenTypes.end())
        {
            throw std::exception("Unexpected token");
        }

        return *this;
    }

    TokenList& TokenList::ExpectBack(const std::vector<TokenType>& tokenTypes)
    {
        if (const auto& a = Back(); std::find(tokenTypes.begin(), tokenTypes.end(), a.type) != tokenTypes.end())
        {
            throw std::exception("Unexpected Token");
        }

        return *this;
    }

    Token TokenList::RemoveFront()
    {
        // Call for checks
        auto a = Front();
        _tokens.pop_front();
        return a;
    }

    Token TokenList::RemoveBack()
    {
        // Call for checks
        auto a = Back();
        _tokens.pop_back();
        return a;
    }

    Token& TokenList::Front()
    {
        if (Empty())
        {
            throw std::exception("Expected Input");
        }
        return _tokens.front();
    }

    Token& TokenList::Back()
    {
        if (Empty())
        {
            throw std::exception("Expected Input");
        }
        return _tokens.back();
    }

    TokenList& TokenList::InsertFront(const Token& token)
    {
        _tokens.push_front(token);
        return *this;
    }

    TokenList& TokenList::InsertBack(const Token& token)
    {
        _tokens.push_back(token);
        return *this;
    }

    bool TokenList::Empty() const
    {
        return _tokens.empty();
    }

    bool TokenList::NotEmpty() const
    {
        return !Empty();
    }
}
