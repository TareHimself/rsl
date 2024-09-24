#include "rsl/TokenDebugInfo.hpp"

rsl::TokenDebugInfo::TokenDebugInfo()
{
}

rsl::TokenDebugInfo::TokenDebugInfo(const uint32_t& inLine, const uint32_t& inCol): TokenDebugInfo(
    inLine, inCol, inLine, inCol + 1)
{
}

rsl::TokenDebugInfo::TokenDebugInfo(const std::string& inFile, const uint32_t& inLine,
                                     const uint32_t& inCol) : TokenDebugInfo(inLine, inCol)
{
    file = inFile;
}

rsl::TokenDebugInfo::TokenDebugInfo(const uint32_t& inStartLine, const uint32_t& inStartCol, const uint32_t& inEndLine,
                                     const uint32_t& inEndCol)
{
    startLine = inStartLine;
    endLine = inEndLine;
    startCol = inStartCol;
    endCol = inEndCol;
}

rsl::TokenDebugInfo::TokenDebugInfo(const std::string& inFile, const uint32_t& inStartLine, const uint32_t& inStartCol,
                                     const uint32_t& inEndLine, const uint32_t& inEndCol) : TokenDebugInfo(
    inStartLine, inStartCol, inEndLine, inEndCol)
{
    file = inFile;
}

rsl::TokenDebugInfo rsl::TokenDebugInfo::operator+(const TokenDebugInfo& other) const
{
    const auto minStartLine = std::min(startLine, other.startLine);
    const auto minStartCol = startLine == minStartLine && other.startLine == minStartLine
                                 ? std::min(startCol, other.startCol)
                                 : (startLine == minStartLine ? startCol : other.startCol);

    const auto maxEndLine = std::max(endLine, other.endLine);
    const auto maxEndCol = startLine == maxEndLine && other.endLine == maxEndLine
                               ? std::max(endCol, other.endCol)
                               : (endLine == maxEndLine ? endCol : other.endCol);

    return TokenDebugInfo{file, minStartLine, minStartCol, maxEndLine, maxEndCol};
}

rsl::TokenDebugInfo& rsl::TokenDebugInfo::operator+=(const TokenDebugInfo& other)
{
    if (this == &other) return *this;

    *this = *this + other;

    return *this;
}
