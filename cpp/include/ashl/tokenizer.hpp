#pragma once
#include <set>
#include <string>
#include <optional>
#include "TokenList.hpp"


namespace ashl
{
    std::optional<Token> joinTokensTill(TokenList& tokens, const std::set<std::string>& search);
    bool isSplitToken(const Token& token);
    bool isSeparatorToken(const Token& token);
    TokenList preprocess(const std::string& fileName, const std::string& fileData);

    TokenList tokenize(const std::string& fileName, const std::string& fileData);

    TokenList tokenize(const std::string& fileName);
}
