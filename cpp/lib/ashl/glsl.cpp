#include "ashl/glsl.hpp"

#include <stdexcept>

namespace ashl::glsl
{
    std::string typeNameToGlslTypeName(const std::string& typeName)
    {
        switch (DeclarationNode::TokenTypeToDeclarationType(Token(typeName, {}).type))
        {
        case EDeclarationType::Boolean:
            return "bool";
        case EDeclarationType::Float:
            return "float";
        case EDeclarationType::Int:
            return "int";
        case EDeclarationType::Float2:
            return "vec2";
        case EDeclarationType::Int2:
            return "ivec2";
        case EDeclarationType::Float3:
            return "vec3";
        case EDeclarationType::Int3:
            return "ivec3";
        case EDeclarationType::Float4:
            return "vec4";
        case EDeclarationType::Int4:
            return "ivec4";
        case EDeclarationType::Mat3:
            return "mat3";
        case EDeclarationType::Mat4:
            return "mat4";
        case EDeclarationType::Void:
            return "void";
        case EDeclarationType::Sampler2D:
            return "sampler2D";
        case EDeclarationType::Buffer:
            return "buffer";
        case EDeclarationType::Block:
        case EDeclarationType::Struct:
            return typeName;
        }

        return "";
    }

    std::string tabs(int depth)
    {
        if (depth == 0) return "";
        std::string r{};
        r.resize(depth, '\t');
        return r;
    }

    std::string generateDeclaration(const std::shared_ptr<DeclarationNode>& node, int depth)
    {
        if (node->declarationType == EDeclarationType::Block)
        {
            if (auto asBlock = std::dynamic_pointer_cast<BlockDeclarationNode>(node))
            {
                auto result = asBlock->GetTypeName() + " " + " {\n";
                for (auto& declarationNode : asBlock->declarations)
                {
                    result += tabs(depth + 1) + generateDeclaration(declarationNode, depth + 1) + ";\n";
                }

                result += tabs(depth) + "} " + asBlock->declarationName;
                return result;
            }
        }

        if (node->declarationType == EDeclarationType::Buffer)
        {
            if (auto asBuffer = std::dynamic_pointer_cast<BufferDeclarationNode>(node))
            {
                auto result = asBuffer->GetTypeName() + " " + asBuffer->declarationName + " {\n";
                for (auto& declarationNode : asBuffer->declarations)
                {
                    result += tabs(depth + 1) + generateDeclaration(declarationNode, depth + 1) + ";\n";
                }

                result += tabs(depth) + "}";
            }
        }

        auto result = typeNameToGlslTypeName(node->GetTypeName()) + (node->declarationName.empty()
                                                                         ? ""
                                                                         : " " + node->declarationName);
        if (node->declarationCount == -1)
        {
            result += "[]";
        }
        else if (node->declarationCount > 1)
        {
            result += "[" + std::to_string(node->declarationCount) + "]";
        }
        return result;
    }

    std::string generateFunctionArgument(const std::shared_ptr<FunctionArgumentNode>& node)
    {
        std::string result{};
        if (node->isInput)
        {
            result += "in ";
        }
        else
        {
            result += "out ";
        }

        result += typeNameToGlslTypeName(node->declaration->GetTypeName());

        if (node->declaration->declarationCount == -1)
        {
            result += "[]";
        }
        else if (node->declaration->declarationCount > 1)
        {
            result += "[" + std::to_string(node->declaration->declarationCount) + "]";
        }

        result += " " + node->declaration->declarationName;
        return result;
    }

    std::string generateScope(const std::shared_ptr<ScopeNode>& node, int depth)
    {
        std::string result = "{\n";

        for (auto& statement : node->statements)
        {
            result += tabs(depth + 1) + generateStatement(statement, depth + 1);
        }

        result += tabs(depth) + "}\n";
        return result;
    }

    std::string generateFunction(const std::shared_ptr<FunctionNode>& node, int depth)
    {
        std::string result = tabs(depth) + generateDeclaration(node->returnDeclaration, depth) + " " + node->name + "(";

        for (size_t i = 0; i < node->arguments.size(); i++)
        {
            result += generateFunctionArgument(node->arguments[i]);
            if (i != node->arguments.size() - 1) result += " , ";
        }

        result += ")\n";

        return result + tabs(depth) + generateScope(node->scope, depth);
    }

    std::string generateTags(const std::unordered_map<std::string, std::string>& tags)
    {
        auto isFirst = true;
        std::string result{};
        
        for (auto& [tag,val] : tags)
        {
            if(tag.starts_with("$")) continue;
            
            result += (isFirst ? "" : " , ") + tag;
            isFirst = false;
            if(!val.empty())
            {
                result += " = " + val;
            }
        }

        return result;
    }

    std::string generateLayout(const std::shared_ptr<LayoutNode>& node, int depth)
    {
        std::string result = tabs(depth) + "layout(" + generateTags(node->tags) + ")";
        
        if (node->tags.contains("$flat"))
        {
            result += " flat";
        }

        switch (node->layoutType)
        {
        case ELayoutType::Uniform:
            result += " uniform ";
            break;
        case ELayoutType::Readonly:
            result += " readonly ";
            break;
        case ELayoutType::Input:
            result += " in ";
            break;
        case ELayoutType::Output:
            result += " out ";
            break;
        }

        result += generateDeclaration(node->declaration, depth) + ";\n";
        return result;
    }

    std::string generateDefine(const std::shared_ptr<DefineNode>& node, int depth)
    {
        return tabs(depth) + "#define " + node->id + " " + generateExpression(node->expression);
    }

    std::string generateInclude(const std::shared_ptr<IncludeNode>& node, int depth)
    {
        return tabs(depth) + "#include \"" + node->targetFile + "\"";
    }

    std::string generateIf(const std::shared_ptr<IfNode>& node, int depth)
    {
        std::string result = "if(" + generateExpression(node->condition, 0) + ")\n";
        result += tabs(depth) + generateScope(node->scope, depth) + generateElse(node->elseNode, depth);
        return result;
    }

    std::string generateElse(const std::shared_ptr<Node>& node, int depth)
    {
        if (node)
        {
            std::string result = tabs(depth) + "else ";
            if (node->nodeType == NodeType::If)
            {
                return result + generateIf(std::dynamic_pointer_cast<IfNode>(node), depth);
            }
            if (node->nodeType == NodeType::Scope)
            {
                return result + "\n" + tabs(depth) + generateScope(std::dynamic_pointer_cast<ScopeNode>(node), depth);
            }
        }

        return "";
    }

    std::string generateFor(const std::shared_ptr<ForNode>& node, int depth)
    {
        std::string result = "for(" + generateExpression(node->init, 0) + ";" + generateExpression(node->condition, 0) +
            ";" + generateExpression(node->update) + ")\n";
        result += tabs(depth) + generateScope(node->scope, depth);
        return result;
    }

    std::string generatePushConstant(const std::shared_ptr<PushConstantNode>& node, int depth)
    {
        std::string result = tabs(depth) + "layout(push_constant" + (node->tags.empty() ? ")" : " , " + generateTags(node->tags)) + ")";

        result += " uniform constant {\n";
        for (auto& declarationNode : node->declarations)
        {
            result += tabs(depth + 1) + generateDeclaration(declarationNode) + ";\n";
        }
        result += tabs(depth) + "} push;\n";
        return result;
    }

    std::string generateStruct(const std::shared_ptr<StructNode>& node, int depth)
    {
        std::string result = tabs(depth) + "struct " + node->name + " {\n";
        for (auto& declarationNode : node->declarations)
        {
            result += tabs(depth + 1) + generateDeclaration(declarationNode, depth + 1) + ";\n";
        }
        result += tabs(depth) + "};\n";
        return result;
    }

    std::string generateStatement(const std::shared_ptr<Node>& node, int depth)
    {
        switch (node->nodeType)
        {
        case NodeType::If:
            if (auto casted = std::dynamic_pointer_cast<IfNode>(node))
            {
                return generateIf(casted, depth);
            }
            break;
        case NodeType::For:
            if (auto casted = std::dynamic_pointer_cast<ForNode>(node))
            {
                return generateFor(casted, depth);
            }
            break;
        default:
            return generateExpression(node, depth) + ";\n";
        }
        return "";
    }

    std::string generateExpression(const std::shared_ptr<Node>& node, int depth)
    {
        switch (node->nodeType)
        {
        case NodeType::Unknown:
            throw std::runtime_error("Unknown node");
        case NodeType::NoOp:
            return "";
        case NodeType::BinaryOp:
            if (auto casted = std::dynamic_pointer_cast<BinaryOpNode>(node))
            {
                std::string op{};
                switch (casted->op)
                {
                case EBinaryOp::Multiply:
                    op = " * ";
                    break;
                case EBinaryOp::Divide:
                    op = " / ";
                    break;
                case EBinaryOp::Add:
                    op = " + ";
                    break;
                case EBinaryOp::Subtract:
                    op = " - ";
                    break;
                case EBinaryOp::Mod:
                    op = " % ";
                    break;
                case EBinaryOp::And:
                    op = " && ";
                    break;
                case EBinaryOp::Or:
                    op = " || ";
                    break;
                case EBinaryOp::Not:
                    op = "!";
                    break;
                case EBinaryOp::Equal:
                    op = " == ";
                    break;
                case EBinaryOp::NotEqual:
                    op = " != ";
                    break;
                case EBinaryOp::Less:
                    op = " < ";
                    break;
                case EBinaryOp::LessEqual:
                    op = " <= ";
                    break;
                case EBinaryOp::Greater:
                    op = " > ";
                    break;
                case EBinaryOp::GreaterEqual:
                    op = " >= ";
                    break;
                }

                if (casted->op == EBinaryOp::Not)
                {
                    throw std::runtime_error("! not supported");
                }

                return generateExpression(casted->left) + op + generateExpression(casted->right);
            }
            break;
        case NodeType::Return:
            if (auto casted = std::dynamic_pointer_cast<ReturnNode>(node))
            {
                return "return " + generateExpression(casted->expression);
            }
            break;
        case NodeType::Assign:
            if (auto casted = std::dynamic_pointer_cast<AssignNode>(node))
            {
                return generateExpression(casted->target) + " = " + generateExpression(casted->value);
            }
            break;
        case NodeType::BinaryOpAndAssign:
            break;
        case NodeType::Call:
            if (auto casted = std::dynamic_pointer_cast<CallNode>(node))
            {
                std::string args{};

                for (size_t i = 0; i < casted->args.size(); i++)
                {
                    args += generateExpression(casted->args[i], 0);
                    if (i != casted->args.size() - 1)
                    {
                        args += " , ";
                    }
                }

                return generateExpression(casted->identifier) + (args.empty() ? "()" : "( " + args + " )");
            }
            break;
        case NodeType::Access:
            if (auto casted = std::dynamic_pointer_cast<AccessNode>(node))
            {
                return generateExpression(casted->left) + "." + generateExpression(casted->right);
            }
            break;
        case NodeType::Index:
            if (auto casted = std::dynamic_pointer_cast<IndexNode>(node))
            {
                return generateExpression(casted->left) + "[" + generateExpression(casted->indexExpression) + "]";
            }
            break;
        case NodeType::Scope:
            if (auto casted = std::dynamic_pointer_cast<ScopeNode>(node))
            {
                return generateScope(casted, depth);
            }
            break;
        case NodeType::Identifier:
            if (auto casted = std::dynamic_pointer_cast<IdentifierNode>(node))
            {
                return typeNameToGlslTypeName(casted->id);
            }
            break;
        case NodeType::Declaration:
            if (auto casted = std::dynamic_pointer_cast<DeclarationNode>(node))
            {
                return generateDeclaration(casted, depth);
            }
            break;
        case NodeType::FloatLiteral:
            if (auto casted = std::dynamic_pointer_cast<FloatLiteralNode>(node))
            {
                auto str = std::to_string(casted->data);
                while (!str.empty() && str[str.length() - 1] == '0' && str[str.length() - 2] != '.')
                {
                    str = str.substr(0, str.length() - 1);
                }
                return str;
            }
            break;
        case NodeType::IntLiteral:
            if (auto casted = std::dynamic_pointer_cast<IntegerLiteralNode>(node))
            {
                return std::to_string(casted->data);
            }
            break;
        case NodeType::Const:
            if (auto casted = std::dynamic_pointer_cast<ConstNode>(node))
            {
                return "const " + generateExpression(casted->declaration);
            }
            break;
        case NodeType::ArrayLiteral:
            if (auto casted = std::dynamic_pointer_cast<ArrayLiteralNode>(node))
            {
                std::string result = "{ ";

                for (size_t i = 0; i < casted->nodes.size(); i++)
                {
                    result += generateExpression(casted->nodes[i]);
                    if (i != casted->nodes.size() - 1)
                    {
                        result += " , ";
                    }
                }

                result += " }";

                return result;
            }
            break;
            break;
        case NodeType::Negate:
            if (auto casted = std::dynamic_pointer_cast<NegateNode>(node))
            {
                return "-" + generateExpression(casted->target);
            }
            break;
        case NodeType::Precedence:
            if (auto casted = std::dynamic_pointer_cast<PrecedenceNode>(node))
            {
                return "( " + generateExpression(casted->target) + " )";
            }
            break;
        case NodeType::Increment:
            if (auto casted = std::dynamic_pointer_cast<IncrementNode>(node))
            {
                return (casted->isPrefix
                            ? "++" + generateExpression(casted->target)
                            : generateExpression(casted->target) + "++");
            }
            break;
        case NodeType::Decrement:
            if (auto casted = std::dynamic_pointer_cast<DecrementNode>(node))
            {
                return (casted->isPrefix
                            ? "--" + generateExpression(casted->target)
                            : generateExpression(casted->target) + "--");
            }
            break;
        case NodeType::Discard:
            if (auto casted = std::dynamic_pointer_cast<DiscardNode>(node))
            {
                return "discard";
            }
            break;
        case NodeType::Conditional:
            if (auto casted = std::dynamic_pointer_cast<ConditionalNode>(node))
            {
                return generateExpression(casted->condition) + " ? " + generateExpression(casted->left) + " : " +
                    generateExpression(casted->right);
            }
            break;
        case NodeType::BooleanLiteral:
            if (auto casted = std::dynamic_pointer_cast<BooleanLiteralNode>(node))
            {
                return (casted->data ? "true" : "false");
            }
            break;
        }

        return "";
    }

    std::string generate(const std::shared_ptr<ModuleNode>& node, int depth)
    {
        std::string result{};

        for (auto& statement : node->statements)
        {
            switch (statement->nodeType)
            {
            case NodeType::Include:
                {
                    result += generateInclude(std::dynamic_pointer_cast<IncludeNode>(statement), depth);
                }
                break;
            case NodeType::Function:
                {
                    result += generateFunction(std::dynamic_pointer_cast<FunctionNode>(statement), depth);
                }
                break;
            case NodeType::Layout:
                {
                    result += generateLayout(std::dynamic_pointer_cast<LayoutNode>(statement), depth);
                }
                break;
            case NodeType::Define:
                {
                    result += generateDefine(std::dynamic_pointer_cast<DefineNode>(statement), depth);
                }
                break;
            case NodeType::PushConstant:
                {
                    result += generatePushConstant(std::dynamic_pointer_cast<PushConstantNode>(statement), depth);
                }
                break;
            case NodeType::Struct:
                {
                    result += generateStruct(std::dynamic_pointer_cast<StructNode>(statement), depth);
                }
                break;
            default:
                {
                    if (auto expr = generateExpression(statement, depth); !expr.empty())
                    {
                        result += tabs(depth) + generateExpression(statement, depth) + ";\n";
                    }
                }
                break;
            }
        }

        return result;
    }
}
