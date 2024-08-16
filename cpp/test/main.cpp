#include <iostream>

#include "ashl/glsl.hpp"
#include "ashl/parser.hpp"
#include "ashl/tokenizer.hpp"
#include "ashl/utils.hpp"

int main() {
    auto tokens = ashl::tokenize("<test>",R"(float2 applyTransform3(float2 pos, mat3 projection){
    return (projection * float3(pos, 1.0)).xy;
}

float2 applyTransform4(float2 pos, mat4 projection){
    return (projection * float4(pos, 0.0, 1.0)).xy;
})");
    auto ast = ashl::parse(tokens);
    ashl::resolveIncludes(ast);
    ashl::resolveReferences(ast);
    auto str = ashl::glsl::generate(ashl::extractScope(ast,ashl::EScopeType::Fragment));
    std::cout  << str << std::endl;
}
