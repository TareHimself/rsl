#pragma once

#include <string>
#include <memory>
#include "nodes.hpp"

namespace rsl::glsl
{
    std::string typeNameToGlslTypeName(const std::string& typeName);
    std::string tabs(int depth);
    std::string generateDeclaration(const std::shared_ptr<DeclarationNode>& node, int depth = 0);
    std::string generateFunctionArgument(const std::shared_ptr<FunctionArgumentNode>& node);
    std::string generateScope(const std::shared_ptr<ScopeNode>& node, int depth = 0);
    std::string generateFunction(const std::shared_ptr<FunctionNode>& node, int depth = 0);
    std::string generateTags(const std::unordered_map<std::string,std::string>& tags);
    std::string generateLayout(const std::shared_ptr<LayoutNode>& node, int depth = 0);
    std::string generateDefine(const std::shared_ptr<DefineNode>& node, int depth = 0);
    std::string generateInclude(const std::shared_ptr<DefineNode>& node, int depth = 0);
    std::string generateIf(const std::shared_ptr<IfNode>& node, int depth = 0);
    std::string generateElse(const std::shared_ptr<Node>& node, int depth = 0);
    std::string generateFor(const std::shared_ptr<ForNode>& node, int depth = 0);
    std::string generatePushConstant(const std::shared_ptr<PushConstantNode>& node, int depth = 0);
    std::string generateStruct(const std::shared_ptr<StructNode>& node, int depth = 0);
    std::string generateStatement(const std::shared_ptr<Node>& node, int depth = 0);
    std::string generateExpression(const std::shared_ptr<Node>& node, int depth = 0);
    std::string generate(const std::shared_ptr<ModuleNode>& node, int depth = 0);
}
