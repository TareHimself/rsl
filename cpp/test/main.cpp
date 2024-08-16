#include <iostream>

#include "ashl/glsl.hpp"
#include "ashl/parser.hpp"
#include "ashl/tokenizer.hpp"
#include "ashl/utils.hpp"

int main() {
    auto tokens = ashl::tokenize(R"(C:\Github\aerox\aerox.Runtime.Widgets\shaders\widgets\image.ash)");
    auto ast = ashl::parse(tokens);
    ashl::resolveIncludes(ast);
    ashl::resolveReferences(ast);
    auto str = ashl::glsl::generate(ashl::extractScope(ast,ashl::EScopeType::Fragment));
    std::cout  << str << std::endl;
}
