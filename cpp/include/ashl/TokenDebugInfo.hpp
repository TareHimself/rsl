#pragma once
#include <string>

namespace ashl {
struct TokenDebugInfo {
    std::string file = "<unknown>";
    uint32_t startLine = 0;
    uint32_t startCol = 0;
    uint32_t endLine = 0;
    uint32_t endCol = 0;
    TokenDebugInfo();
    TokenDebugInfo(const uint32_t& inLine,const uint32_t& inCol);
    TokenDebugInfo(const std::string& inFile,const uint32_t& inLine,const uint32_t& inCol);

    TokenDebugInfo(const uint32_t& inStartLine,const uint32_t& inStartCol,const uint32_t& inEndLine,const uint32_t& inEndCol);
    TokenDebugInfo(const std::string& inFile,const uint32_t& inStartLine,const uint32_t& inStartCol,const uint32_t& inEndLine,const uint32_t& inEndCol);

    TokenDebugInfo operator +(const TokenDebugInfo& other) const;

    TokenDebugInfo& operator +=(const TokenDebugInfo& other);
};
}
