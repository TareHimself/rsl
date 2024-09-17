#include "ashl/nodes.hpp"

#include <stdexcept>

#include "ashl/utils.hpp"

namespace ashl
{
    size_t Node::ComputeSelfHash() const
    {
        size_t seed = static_cast<size_t>(nodeType);
        for (const auto& child : GetChildren())
        {
            seed = hashCombine(seed, child->ComputeHash());
        }
        return seed;
    }

    Node::Node(const NodeType& inNodeType)
    {
        nodeType = inNodeType;
    }

    size_t Node::ComputeHash() const
    {
        return ComputeSelfHash();
    }

    ModuleNode::ModuleNode(const std::vector<std::shared_ptr<Node>>& inStatements) : Node(NodeType::Module)
    {
        statements = inStatements;
    }

    std::vector<std::shared_ptr<Node>> ModuleNode::GetChildren() const
    {
        return statements;
    }

    ScopeNode::ScopeNode(const std::vector<std::shared_ptr<Node>>& inStatements) : Node(NodeType::Scope)
    {
        statements = inStatements;
    }

    std::vector<std::shared_ptr<Node>> ScopeNode::GetChildren() const
    {
        return statements;
    }

    EDeclarationType DeclarationNode::TokenTypeToDeclarationType(TokenType tokenType)
    {
        switch (tokenType)
        {
        case TokenType::TypeBoolean:
            return EDeclarationType::Boolean;
        case TokenType::TypeVoid:
            return EDeclarationType::Void;
        case TokenType::TypeFloat:
            return EDeclarationType::Float;
        case TokenType::TypeFloat2:
            return EDeclarationType::Float2;
        case TokenType::TypeFloat3:
            return EDeclarationType::Float3;
        case TokenType::TypeFloat4:
            return EDeclarationType::Float4;
        case TokenType::TypeInt:
            return EDeclarationType::Int;
        case TokenType::TypeInt2:
            return EDeclarationType::Int2;
        case TokenType::TypeInt3:
            return EDeclarationType::Int3;
        case TokenType::TypeInt4:
            return EDeclarationType::Int4;
        case TokenType::TypeMat3:
            return EDeclarationType::Mat3;
        case TokenType::TypeMat4:
            return EDeclarationType::Mat4;
        case TokenType::TypeBuffer:
            return EDeclarationType::Buffer;
        case TokenType::TypeSampler:
            return EDeclarationType::Sampler;
        case TokenType::TypeTexture2D:
            return EDeclarationType::Texture2D;
        case TokenType::TypeSampler2D:
            return EDeclarationType::Sampler2D;
        default:
            return EDeclarationType::Struct;
        }
    }

    DeclarationNode::DeclarationNode(const EDeclarationType& inDeclarationType, const std::string& inDeclarationName,
                                     const int& inDeclarationCount) : Node(NodeType::Declaration)
    {
        declarationType = inDeclarationType;
        declarationName = inDeclarationName;
        declarationCount = inDeclarationCount;
    }

    DeclarationNode::DeclarationNode(const Token& typeToken, const std::string& inDeclarationName,
                                     const int& inDeclarationCount): Node(NodeType::Declaration)
    {
        declarationType = TokenTypeToDeclarationType(typeToken.type);
        declarationName = inDeclarationName;
        declarationCount = inDeclarationCount;
    }

    uint64_t DeclarationNode::GetSize() const
    {
        switch (declarationType)
        {
        case EDeclarationType::Struct:
            throw std::runtime_error("Node with type 'Struct' must use class 'StructDeclarationNode'");
        case EDeclarationType::Float:
        case EDeclarationType::Int:
            return static_cast<uint64_t>(4) * declarationCount;
        case EDeclarationType::Float2:
        case EDeclarationType::Int2:
            return static_cast<uint64_t>(8) * declarationCount;
        case EDeclarationType::Float3:
        case EDeclarationType::Int3:
            return static_cast<uint64_t>(12) * declarationCount;
        case EDeclarationType::Float4:
        case EDeclarationType::Int4:
            return static_cast<uint64_t>(16) * declarationCount;
        case EDeclarationType::Mat3:
            return static_cast<uint64_t>(12) * 3 * declarationCount;
        case EDeclarationType::Mat4:
            return static_cast<uint64_t>(16) * 4 * declarationCount;
        default:
            throw std::runtime_error("Unknown Declaration Size");
        }
    }

    std::string DeclarationNode::GetTypeName()
    {
        switch (declarationType)
        {
        case EDeclarationType::Float:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeFloat];
        case EDeclarationType::Int:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeInt];
        case EDeclarationType::Float2:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeFloat2];
        case EDeclarationType::Int2:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeInt2];
        case EDeclarationType::Float3:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeFloat3];
        case EDeclarationType::Int3:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeInt3];
        case EDeclarationType::Float4:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeFloat4];
        case EDeclarationType::Int4:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeInt4];
        case EDeclarationType::Mat3:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeMat3];
        case EDeclarationType::Mat4:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeMat4];
        case EDeclarationType::Void:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeVoid];
        case EDeclarationType::Sampler2D:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeSampler2D];
        case EDeclarationType::Sampler:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeSampler];
        case EDeclarationType::Texture2D:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeTexture2D];
        case EDeclarationType::Buffer:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeBuffer];
        case EDeclarationType::Boolean:
            return Token::TOKENS_TO_KEYWORDS[TokenType::TypeBoolean];
        default:
            throw std::runtime_error("Unknown declaration type");
        }
    }

    std::vector<std::shared_ptr<Node>> DeclarationNode::GetChildren() const
    {
        return {};
    }

    size_t DeclarationNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(), declarationType, declarationName, declarationCount);
    }

    uint64_t StructNode::GetSize() const
    {
        uint64_t size = 0;
        for (auto& declarationNode : declarations)
        {
            size += declarationNode->GetSize();
        }
        return size;
    }

    StructNode::StructNode(const std::string& inName,
                           const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations) : Node(NodeType::Struct)
    {
        name = inName;
        declarations = inDeclarations;
    }

    std::vector<std::shared_ptr<Node>> StructNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> r{};
        r.reserve(declarations.size());
        for (auto& declarationNode : declarations)
        {
            r.push_back(declarationNode);
        }
        return r;
    }

    size_t StructNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(), name);
    }

    uint64_t StructDeclarationNode::GetSize() const
    {
        if (!structNode) throw std::runtime_error("Struct Reference Is Invalid");
        return structNode->GetSize() * declarationCount;
    }

    std::string StructDeclarationNode::GetTypeName()
    {
        return structName;
    }

    std::vector<std::shared_ptr<Node>> StructDeclarationNode::GetChildren() const
    {
        auto d = DeclarationNode::GetChildren();
        if (structNode)
        {
            d.push_back(structNode);
        }
        return d;
    }

    StructDeclarationNode::StructDeclarationNode(const std::string& inStructName, const std::string& inDeclarationName,
                                                 const int& inCount) : DeclarationNode(
        EDeclarationType::Struct, inDeclarationName, inCount)
    {
        structName = inStructName;
    }

    StructDeclarationNode::StructDeclarationNode(const std::shared_ptr<StructNode>& inStruct,
                                                 const std::string& inDeclarationName,
                                                 const int& inCount) : DeclarationNode(
        EDeclarationType::Struct, inDeclarationName, inCount)
    {
        structNode = inStruct;
        structName = inStruct->name;
    }

    size_t StructDeclarationNode::ComputeSelfHash() const
    {
        return hashCombine(DeclarationNode::ComputeSelfHash(), structName);
    }

    BufferDeclarationNode::BufferDeclarationNode(const std::string& inName, const int& inCount,
                                                 const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations):
        DeclarationNode(EDeclarationType::Block, inName, inCount)
    {
        declarations = inDeclarations;
    }

    std::vector<std::shared_ptr<Node>> BufferDeclarationNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> r{};
        r.reserve(declarations.size());
        for (auto& declarationNode : declarations)
        {
            r.push_back(declarationNode);
        }
        return r;
    }

    std::string BufferDeclarationNode::GetTypeName()
    {
        return "buffer";
    }

    uint64_t BufferDeclarationNode::GetSize() const
    {
        uint64_t size = 0;
        for (auto& declarationNode : declarations)
        {
            size += declarationNode->GetSize();
        }
        return size * declarationCount;
    }

    uint64_t BlockDeclarationNode::GetSize() const
    {
        uint64_t size = 0;
        for (auto& declarationNode : declarations)
        {
            size += declarationNode->GetSize();
        }
        return size * declarationCount;
    }

    std::string BlockDeclarationNode::GetTypeName()
    {
        return "_block_" + declarationName;
    }

    BlockDeclarationNode::BlockDeclarationNode(const std::string& inDeclarationName,
                                               const int& inCount,
                                               const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations):
        DeclarationNode(EDeclarationType::Block, inDeclarationName, inCount)
    {
        declarations = inDeclarations;
    }

    std::vector<std::shared_ptr<Node>> BlockDeclarationNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> r{};
        r.reserve(declarations.size());
        for (auto& declarationNode : declarations)
        {
            r.push_back(declarationNode);
        }
        return r;
    }

    AssignNode::AssignNode(const std::shared_ptr<Node>& inTarget, const std::shared_ptr<Node>& inValue) : Node(
        NodeType::Assign)
    {
        target = inTarget;
        value = inValue;
    }

    std::vector<std::shared_ptr<Node>> AssignNode::GetChildren() const
    {
        return {target, value};
    }


    BinaryOpNode::BinaryOpNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inRight,
                               const EBinaryOp& inOp) : Node(NodeType::BinaryOp)
    {
        left = inLeft;
        right = inRight;
        op = inOp;
    }

    BinaryOpNode::BinaryOpNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inRight,
                               const TokenType& inOp) : Node(NodeType::BinaryOp)
    {
        left = inLeft;
        right = inRight;

        switch (inOp)
        {
        case TokenType::OpLess:
            op = EBinaryOp::Less;
            break;
        case TokenType::OpGreater:
            op = EBinaryOp::Greater;
            break;
        case TokenType::OpEqual:
            op = EBinaryOp::Equal;
            break;
        case TokenType::OpNotEqual:
            op = EBinaryOp::NotEqual;
            break;
        case TokenType::OpLessEqual:
            op = EBinaryOp::LessEqual;
            break;
        case TokenType::OpGreaterEqual:
            op = EBinaryOp::GreaterEqual;
            break;
        case TokenType::OpAnd:
            op = EBinaryOp::And;
            break;
        case TokenType::OpOr:
            op = EBinaryOp::Or;
            break;
        case TokenType::OpNot:
            op = EBinaryOp::Not;
            break;
        case TokenType::OpAdd:
            op = EBinaryOp::Add;
            break;
        case TokenType::OpSubtract:
            op = EBinaryOp::Subtract;
            break;
        case TokenType::OpDivide:
            op = EBinaryOp::Divide;
            break;
        case TokenType::OpMultiply:
            op = EBinaryOp::Multiply;
            break;
        case TokenType::OpMod:
            op = EBinaryOp::Mod;
            break;
        }
    }

    std::vector<std::shared_ptr<Node>> BinaryOpNode::GetChildren() const
    {
        return {left, right};
    }

    size_t BinaryOpNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(), static_cast<int>(op));
    }

    FunctionArgumentNode::FunctionArgumentNode(bool inIsInput,
                                               const std::shared_ptr<DeclarationNode>& inDeclaration) : Node(
        NodeType::FunctionArgument)
    {
        isInput = inIsInput;
        declaration = inDeclaration;
    }

    std::vector<std::shared_ptr<Node>> FunctionArgumentNode::GetChildren() const
    {
        return {declaration};
    }


    FunctionNode::FunctionNode(const std::shared_ptr<DeclarationNode>& inReturnDeclaration, const std::string& inName,
                               const std::vector<std::shared_ptr<FunctionArgumentNode>>& inArguments,
                               const std::shared_ptr<ScopeNode>& inScope) : Node(NodeType::Function)
    {
        returnDeclaration = inReturnDeclaration;
        name = inName;
        arguments = inArguments;
        scope = inScope;
    }

    std::vector<std::shared_ptr<Node>> FunctionNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> children{};
        children.reserve(2 + arguments.size());
        children.push_back(returnDeclaration);
        children.insert(children.end(), arguments.begin(), arguments.end());
        children.push_back(scope);
        return children;
    }

    IdentifierNode::IdentifierNode(const std::string& inId) : Node(NodeType::Identifier)
    {
        id = inId;
    }

    std::vector<std::shared_ptr<Node>> IdentifierNode::GetChildren() const
    {
        return {};
    }

    size_t IdentifierNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(), id);
    }

    AccessNode::AccessNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inRight) : Node(
        NodeType::Access)
    {
        left = inLeft;
        right = inRight;
    }

    std::vector<std::shared_ptr<Node>> AccessNode::GetChildren() const
    {
        return {left, right};
    }

    IndexNode::IndexNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inIndexExpression) : Node(
        NodeType::Index)
    {
        left = inLeft;
        indexExpression = inIndexExpression;
    }

    std::vector<std::shared_ptr<Node>> IndexNode::GetChildren() const
    {
        return {left, indexExpression};
    }

    ConstNode::ConstNode(const std::shared_ptr<DeclarationNode>& inDeclaration) : Node(NodeType::Const)
    {
        declaration = inDeclaration;
    }

    std::vector<std::shared_ptr<Node>> ConstNode::GetChildren() const
    {
        return {declaration};
    }

    IntegerLiteralNode::IntegerLiteralNode(const int& inData) : Node(NodeType::IntLiteral)
    {
        data = inData;
    }

    std::vector<std::shared_ptr<Node>> IntegerLiteralNode::GetChildren() const
    {
        return {};
    }

    BooleanLiteralNode::BooleanLiteralNode(const bool& inData) : Node(NodeType::BooleanLiteral)
    {
        data = inData;
    }

    std::vector<std::shared_ptr<Node>> BooleanLiteralNode::GetChildren() const
    {
        return {};
    }

    FloatLiteralNode::FloatLiteralNode(const float& inData) : Node(NodeType::FloatLiteral)
    {
        data = inData;
    }

    std::vector<std::shared_ptr<Node>> FloatLiteralNode::GetChildren() const
    {
        return {};
    }

    CallNode::CallNode(const std::shared_ptr<IdentifierNode>& inIdentifier,
                       const std::vector<std::shared_ptr<Node>>& inArgs) : Node(NodeType::Call)
    {
        identifier = inIdentifier;
        args = inArgs;
    }

    std::vector<std::shared_ptr<Node>> CallNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> result{identifier};
        result.insert(result.end(), args.begin(), args.end());
        return result;
    }

    IncrementNode::IncrementNode(bool inIsPrefix, const std::shared_ptr<Node>& inTarget) : Node(NodeType::Increment)
    {
        isPrefix = inIsPrefix;
        target = inTarget;
    }

    std::vector<std::shared_ptr<Node>> IncrementNode::GetChildren() const
    {
        return {target};
    }

    size_t IncrementNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(), isPrefix);
    }

    DecrementNode::DecrementNode(bool inIsPrefix, const std::shared_ptr<Node>& inTarget) : Node(NodeType::Decrement)
    {
        isPrefix = inIsPrefix;
        target = inTarget;
    }

    std::vector<std::shared_ptr<Node>> DecrementNode::GetChildren() const
    {
        return {target};
    }

    size_t DecrementNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(), isPrefix);
    }

    NegateNode::NegateNode(const std::shared_ptr<Node>& inTarget) : Node(NodeType::Negate)
    {
        target = inTarget;
    }

    std::vector<std::shared_ptr<Node>> NegateNode::GetChildren() const
    {
        return {target};
    }

    PrecedenceNode::PrecedenceNode(const std::shared_ptr<Node>& inTarget) : Node(NodeType::Precedence)
    {
        target = inTarget;
    }

    std::vector<std::shared_ptr<Node>> PrecedenceNode::GetChildren() const
    {
        return {target};
    }

    DiscardNode::DiscardNode() : Node(NodeType::Discard)
    {
    }

    std::vector<std::shared_ptr<Node>> DiscardNode::GetChildren() const
    {
        return {};
    }

    IfNode::IfNode(const std::shared_ptr<Node>& inCondition, const std::shared_ptr<ScopeNode>& inScope,
                   const std::shared_ptr<Node>& inElseScope) : Node(NodeType::If)
    {
        condition = inCondition;
        scope = inScope;
        elseNode = inElseScope;
    }

    std::vector<std::shared_ptr<Node>> IfNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> results{condition, scope};
        if (elseNode)
        {
            results.push_back(elseNode);
        }
        return results;
    }

    ForNode::ForNode(const std::shared_ptr<Node>& inInit, const std::shared_ptr<Node>& inCondition,
                     const std::shared_ptr<Node>& inUpdate,
                     const std::shared_ptr<ScopeNode>& inScope) : Node(NodeType::For)
    {
        init = inInit;
        condition = inCondition;
        update = inUpdate;
        scope = inScope;
    }

    std::vector<std::shared_ptr<Node>> ForNode::GetChildren() const
    {
        return {init, condition, update, scope};
    }

    LayoutNode::LayoutNode(const ELayoutType& inLayoutType, const std::shared_ptr<DeclarationNode>& inDeclaration,
                           const std::unordered_map<std::string, std::string>& inTags) : Node(NodeType::Layout)
    {
        layoutType = inLayoutType;
        declaration = inDeclaration;
        tags = inTags;
    }

    std::vector<std::shared_ptr<Node>> LayoutNode::GetChildren() const
    {
        return {declaration};
    }

    size_t LayoutNode::ComputeSelfHash() const
    {
        std::string tagsStr{};
        for (auto& [fst, snd] : tags)
        {
            tagsStr += fst + "-" + snd;
        }
        return hashCombine(Node::ComputeSelfHash(), static_cast<int>(layoutType), tagsStr);
    }

    PushConstantNode::PushConstantNode(const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations,
                                       const std::unordered_map<std::string, std::string>& inTags) : Node(
        NodeType::PushConstant)
    {
        declarations = inDeclarations;
        tags = inTags;
    }

    std::vector<std::shared_ptr<Node>> PushConstantNode::GetChildren() const
    {
        return mapVector<std::shared_ptr<Node>, std::shared_ptr<DeclarationNode>>(
            declarations, [](const std::shared_ptr<DeclarationNode>& d)
            {
                return d;
            });
    }

    size_t PushConstantNode::ComputeSelfHash() const
    {
        std::string tagsStr{};
        for (auto& [fst, snd] : tags)
        {
            tagsStr += fst + "-" + snd;
        }
        return hashCombine(Node::ComputeSelfHash(), tagsStr);
    }

    size_t PushConstantNode::GetSize() const
    {
        size_t size = 0;
        for (auto& declarationNode : declarations)
        {
            size += declarationNode->GetSize();
        }

        return size;
    }

    DefineNode::DefineNode(const std::string& identifier, const std::shared_ptr<Node>& inExpression) : Node(
        NodeType::Define)
    {
        id = identifier;
        expression = inExpression;
    }

    std::vector<std::shared_ptr<Node>> DefineNode::GetChildren() const
    {
        return {expression};
    }

    size_t DefineNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(), id);
    }

    IncludeNode::IncludeNode(const std::string& inSourceFile, const std::string& inTargetFile) : Node(NodeType::Include)
    {
        sourceFile = inSourceFile;
        targetFile = inTargetFile;
    }

    std::vector<std::shared_ptr<Node>> IncludeNode::GetChildren() const
    {
        return {};
    }

    size_t IncludeNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(), sourceFile, targetFile);
    }

    ConditionalNode::ConditionalNode(const std::shared_ptr<Node>& inCondition, const std::shared_ptr<Node>& inLeft,
                                     const std::shared_ptr<Node>& inRight) : Node(NodeType::Conditional)
    {
        condition = inCondition;
        left = inLeft;
        right = inRight;
    }

    std::vector<std::shared_ptr<Node>> ConditionalNode::GetChildren() const
    {
        return {condition, left, right};
    }

    ReturnNode::ReturnNode(const std::shared_ptr<Node>& inExpression) : Node(NodeType::Return)
    {
        expression = inExpression;
    }

    std::vector<std::shared_ptr<Node>> ReturnNode::GetChildren() const
    {
        return {expression};
    }

    ArrayLiteralNode::ArrayLiteralNode(const std::vector<std::shared_ptr<Node>>& inNodes) : Node(NodeType::ArrayLiteral)
    {
        nodes = inNodes;
    }

    std::vector<std::shared_ptr<Node>> ArrayLiteralNode::GetChildren() const
    {
        return nodes;
    }

    NoOpNode::NoOpNode() : Node(NodeType::NoOp)
    {
    }

    std::vector<std::shared_ptr<Node>> NoOpNode::GetChildren() const
    {
        return {};
    }

    NamedScopeNode::NamedScopeNode(EScopeType inScopeType, const std::shared_ptr<ScopeNode>& inScope) : Node(
        NodeType::NamedScope)
    {
        scopeType = inScopeType;
        scope = inScope;
    }

    std::vector<std::shared_ptr<Node>> NamedScopeNode::GetChildren() const
    {
        return {scope};
    }

    size_t NamedScopeNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(), static_cast<int>(scopeType));
    }
}
