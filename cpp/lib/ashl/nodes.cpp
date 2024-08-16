#include "ashl/nodes.hpp"
#include "ashl/utils.hpp"

namespace ashl
{
    size_t Node::ComputeSelfHash() const
    {
        size_t seed = static_cast<size_t>(nodeType);
        for (const auto& child : GetChildren())
        {
            seed = hashCombine(seed,child->ComputeHash());
        }
        return seed;
    }

    Node::Node(const ENodeType& inNodeType)
    {
        nodeType = inNodeType;
    }

    size_t Node::ComputeHash() const
    {
        return ComputeSelfHash();
    }

    ModuleNode::ModuleNode(const std::vector<std::shared_ptr<Node>>& inStatements) : Node(ENodeType::Module)
    {
        statements = inStatements;
    }

    std::vector<std::shared_ptr<Node>> ModuleNode::GetChildren() const
    {
        return statements;
    }

    ScopeNode::ScopeNode(const std::vector<std::shared_ptr<Node>>& inStatements) : Node(ENodeType::Scope)
    {
        statements = inStatements;
    }

    std::vector<std::shared_ptr<Node>> ScopeNode::GetChildren() const
    {
        return statements;
    }

    EDeclarationType DeclarationNode::TokenTypeToDeclarationType(ETokenType tokenType)
    {
        switch (tokenType)
        {
        case ETokenType::TypeBoolean:
            return EDeclarationType::Boolean;
        case ETokenType::TypeVoid:
            return EDeclarationType::Void;
        case ETokenType::TypeFloat:
            return EDeclarationType::Float;
        case ETokenType::TypeFloat2:
            return EDeclarationType::Float2;
        case ETokenType::TypeFloat3:
            return EDeclarationType::Float3;
        case ETokenType::TypeFloat4:
            return EDeclarationType::Float4;
        case ETokenType::TypeInt:
            return EDeclarationType::Int;
        case ETokenType::TypeInt2:
            return EDeclarationType::Int2;
        case ETokenType::TypeInt3:
            return EDeclarationType::Int3;
        case ETokenType::TypeInt4:
            return EDeclarationType::Int4;
        case ETokenType::TypeMat3:
            return EDeclarationType::Mat3;
        case ETokenType::TypeMat4:
            return EDeclarationType::Mat4;
        case ETokenType::TypeBuffer:
            return EDeclarationType::Buffer;
        case ETokenType::TypeSampler2D:
            return EDeclarationType::Sampler2D;
        default:
            return EDeclarationType::Struct;
        }
    }

    DeclarationNode::DeclarationNode(const EDeclarationType& inDeclarationType, const std::string& inDeclarationName,
                                     const int& inDeclarationCount) : Node(ENodeType::Declaration)
    {
        declarationType = inDeclarationType;
        declarationName = inDeclarationName;
        declarationCount = inDeclarationCount;
    }

    DeclarationNode::DeclarationNode(const Token& typeToken, const std::string& inDeclarationName,
        const int& inDeclarationCount): Node(ENodeType::Declaration)
    {

        declarationType = TokenTypeToDeclarationType(typeToken.type);
        declarationName = inDeclarationName;
        declarationCount = inDeclarationCount;
    }

    uint64_t DeclarationNode::GetSize() const
    {
        switch (declarationType) {
        case EDeclarationType::Struct:
            throw std::exception("Node with type 'Struct' must use class 'StructDeclarationNode'");
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
            throw std::exception("Unknown Declaration Size");
        }
    }

    std::string DeclarationNode::GetTypeName()
    {
        switch (declarationType) {
        case EDeclarationType::Float:
            return  Token::TOKENS_TO_KEYWORDS[ETokenType::TypeFloat];
        case EDeclarationType::Int:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeInt];
        case EDeclarationType::Float2:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeFloat2];
        case EDeclarationType::Int2:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeInt2];
        case EDeclarationType::Float3:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeFloat3];
        case EDeclarationType::Int3:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeInt3];
        case EDeclarationType::Float4:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeFloat4];
        case EDeclarationType::Int4:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeInt4];
        case EDeclarationType::Mat3:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeMat3];
        case EDeclarationType::Mat4:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeMat4];
        case EDeclarationType::Void:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeVoid];
        case EDeclarationType::Sampler2D:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeSampler2D];
        case EDeclarationType::Buffer:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeBuffer];
        case EDeclarationType::Boolean:
            return Token::TOKENS_TO_KEYWORDS[ETokenType::TypeBoolean];
        default:
            throw std::exception("Unknown declaration type");
        }
    }

    std::vector<std::shared_ptr<Node>> DeclarationNode::GetChildren() const
    {
        return {};
    }

    size_t DeclarationNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(),declarationType,declarationName,declarationCount);
    }

    uint64_t StructNode::GetSize() const
    {
        uint64_t size = 0;
        for (auto &declarationNode : declarations)
        {
            size += declarationNode->GetSize();
        }
        return size;
    }

    StructNode::StructNode(const std::string& inName,
                           const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations) : Node(ENodeType::Struct)
    {
        name = inName;
        declarations = inDeclarations;
    }

    std::vector<std::shared_ptr<Node>> StructNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> r{};
        r.reserve(declarations.size());
        for (auto &declarationNode : declarations)
        {
            r.push_back(declarationNode);
        }
        return r;
    }

    size_t StructNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(),name);
    }

    uint64_t StructDeclarationNode::GetSize() const
    {
        if(!structNode) throw std::exception("Struct Reference Is Invalid");

        uint64_t size = 0;
        return structNode->GetSize();
    }

    std::string StructDeclarationNode::GetTypeName()
    {
        return structName;
    }

    std::vector<std::shared_ptr<Node>> StructDeclarationNode::GetChildren() const
    {
        auto d = DeclarationNode::GetChildren();
        if(structNode)
        {
            d.push_back(structNode);
        }
        return d;
    }

    StructDeclarationNode::StructDeclarationNode(const std::string& inStructName,const std::string& inDeclarationName, const int& inCount) : DeclarationNode(EDeclarationType::Struct,inDeclarationName,inCount)
    {
        structName = inStructName;
    }

    StructDeclarationNode::StructDeclarationNode(const std::shared_ptr<StructNode>& inStruct,const std::string& inDeclarationName, const int& inCount) : DeclarationNode(EDeclarationType::Struct,inDeclarationName,inCount)
    {
        structNode = inStruct;
        structName = inStruct->name;
    }

    size_t StructDeclarationNode::ComputeSelfHash() const
    {
        return hashCombine(DeclarationNode::ComputeSelfHash(),structName);
    }

    BufferDeclarationNode::BufferDeclarationNode(const std::string& inName, const int& inCount,
                                                 const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations): DeclarationNode(EDeclarationType::Block,inName,inCount)
    {
        declarations = inDeclarations;
    }

    std::vector<std::shared_ptr<Node>> BufferDeclarationNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> r{};
        r.reserve(declarations.size());
        for (auto &declarationNode : declarations)
        {
            r.push_back(declarationNode);
        }
        return r;
    }

    std::string BufferDeclarationNode::GetTypeName()
    {
        return "buffer";
    }

    uint64_t BlockDeclarationNode::GetSize() const
    {
        uint64_t size = 0;
        for (auto &declarationNode : declarations)
        {
            size += declarationNode->GetSize();
        }
        return size;
    }

    std::string BlockDeclarationNode::GetTypeName()
    {
        return "_block_" + declarationName;
    }

    BlockDeclarationNode::BlockDeclarationNode(const std::string& inDeclarationName,
                                               const int& inCount, const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations):
        DeclarationNode(EDeclarationType::Block,inDeclarationName,inCount)
    {
        declarations = inDeclarations;
    }

    AssignNode::AssignNode(const std::shared_ptr<Node>& inTarget, const std::shared_ptr<Node>& inValue) : Node(ENodeType::Assign)
    {
        target = inTarget;
        value = inValue;
    }

    std::vector<std::shared_ptr<Node>> AssignNode::GetChildren() const
    {
        return {target,value};
    }
    

    BinaryOpNode::BinaryOpNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inRight,
        const EBinaryOp& inOp) : Node(ENodeType::BinaryOp)
    {
        left = inLeft;
        right = inRight;
        op = inOp;
    }

    BinaryOpNode::BinaryOpNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inRight,
        const ETokenType& inOp) : Node(ENodeType::BinaryOp)
    {
        left = inLeft;
        right = inRight;

        switch (inOp)
        {
        case ETokenType::OpLess:
            op = EBinaryOp::Less;
            break;
        case ETokenType::OpGreater:
            op = EBinaryOp::Greater;
            break;
        case ETokenType::OpEqual:
            op = EBinaryOp::Equal;
            break;
        case ETokenType::OpNotEqual:
            op = EBinaryOp::NotEqual;
            break;
        case ETokenType::OpLessEqual:
            op = EBinaryOp::LessEqual;
            break;
        case ETokenType::OpGreaterEqual:
            op = EBinaryOp::GreaterEqual;
            break;
        case ETokenType::OpAnd:
            op = EBinaryOp::And;
            break;
        case ETokenType::OpOr:
            op = EBinaryOp::Or;
            break;
        case ETokenType::OpNot:
            op = EBinaryOp::Not;
            break;
        case ETokenType::OpAdd:
            op = EBinaryOp::Add;
            break;
        case ETokenType::OpSubtract:
            op = EBinaryOp::Subtract;
            break;
        case ETokenType::OpDivide:
            op = EBinaryOp::Divide;
            break;
        case ETokenType::OpMultiply:
            op = EBinaryOp::Multiply;
            break;
        case ETokenType::OpMod:
            op = EBinaryOp::Mod;
            break;
        }
    }

    std::vector<std::shared_ptr<Node>> BinaryOpNode::GetChildren() const
    {
        return {left,right};
    }

    size_t BinaryOpNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(),static_cast<int>(op));
    }

    FunctionArgumentNode::FunctionArgumentNode(bool inIsInput, const std::shared_ptr<DeclarationNode>& inDeclaration) : Node(ENodeType::FunctionArgument)
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
        const std::shared_ptr<ScopeNode>& inScope) : Node(ENodeType::Function)
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
        children.insert(children.end(),arguments.begin(),arguments.end());
        children.push_back(scope);
        return children;
    }

    IdentifierNode::IdentifierNode(const std::string& inId) : Node(ENodeType::Identifier)
    {
        id = inId;
    }

    std::vector<std::shared_ptr<Node>> IdentifierNode::GetChildren() const
    {
        return {};
    }

    size_t IdentifierNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(),id);
    }

    AccessNode::AccessNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inRight) : Node(ENodeType::Access)
    {
        left = inLeft;
        right = inRight;
    }

    std::vector<std::shared_ptr<Node>> AccessNode::GetChildren() const
    {
        return {left,right};
    }

    IndexNode::IndexNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inIndexExpression) : Node(ENodeType::Index)
    {
        left = inLeft;
        indexExpression= inIndexExpression;
    }

    std::vector<std::shared_ptr<Node>> IndexNode::GetChildren() const
    {
        return {left,indexExpression};
    }

    ConstNode::ConstNode(const std::shared_ptr<DeclarationNode>& inDeclaration) : Node(ENodeType::Const)
    {
        declaration = inDeclaration;
    }

    std::vector<std::shared_ptr<Node>> ConstNode::GetChildren() const
    {
        return {declaration};
    }

    IntegerLiteralNode::IntegerLiteralNode(const int& inData) : Node(ENodeType::IntLiteral)
    {
        data = inData;
    }

    std::vector<std::shared_ptr<Node>> IntegerLiteralNode::GetChildren() const
    {
        return {};
    }

    BooleanLiteralNode::BooleanLiteralNode(const bool& inData) : Node(ENodeType::BooleanLiteral)
    {
        data = inData;
    }

    std::vector<std::shared_ptr<Node>> BooleanLiteralNode::GetChildren() const
    {
        return {};
    }

    FloatLiteralNode::FloatLiteralNode(const float& inData) : Node(ENodeType::FloatLiteral)
    {
        data = inData;
    }

    std::vector<std::shared_ptr<Node>> FloatLiteralNode::GetChildren() const
    {
        return {};
    }

    CallNode::CallNode(const std::shared_ptr<IdentifierNode>& inIdentifier,
                       const std::vector<std::shared_ptr<Node>>& inArgs) : Node(ENodeType::Call)
    {
        identifier = inIdentifier;
        args = inArgs;
    }

    std::vector<std::shared_ptr<Node>> CallNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> result{identifier};
        result.insert(result.end(),args.begin(),args.end());
        return result;
    }

    IncrementNode::IncrementNode(bool inIsPrefix, const std::shared_ptr<Node>& inTarget) : Node(ENodeType::Increment)
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
        return hashCombine(Node::ComputeSelfHash(),isPrefix);
    }

    DecrementNode::DecrementNode(bool inIsPrefix, const std::shared_ptr<Node>& inTarget) : Node(ENodeType::Decrement)
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
        return hashCombine(Node::ComputeSelfHash(),isPrefix);
    }

    NegateNode::NegateNode(const std::shared_ptr<Node>& inTarget) : Node(ENodeType::Negate)
    {
        target = inTarget;
    }

    std::vector<std::shared_ptr<Node>> NegateNode::GetChildren() const
    {
        return {target};
    }

    PrecedenceNode::PrecedenceNode(const std::shared_ptr<Node>& inTarget) : Node(ENodeType::Precedence)
    {
        target = inTarget;
    }

    std::vector<std::shared_ptr<Node>> PrecedenceNode::GetChildren() const
    {
        return {target};
    }

    DiscardNode::DiscardNode() : Node(ENodeType::Discard)
    {
    }

    std::vector<std::shared_ptr<Node>> DiscardNode::GetChildren() const
    {
        return {};
    }

    IfNode::IfNode(const std::shared_ptr<Node>& inCondition, const std::shared_ptr<ScopeNode>& inScope,
        const std::shared_ptr<Node>& inElseScope) : Node(ENodeType::If)
    {
        condition = inCondition;
        scope = inScope;
        elseNode = inElseScope;
    }

    std::vector<std::shared_ptr<Node>> IfNode::GetChildren() const
    {
        std::vector<std::shared_ptr<Node>> results{condition,scope};
        if(elseNode)
        {
            results.push_back(elseNode);
        }
        return results;
    }

    ForNode::ForNode(const std::shared_ptr<Node>& inInit, const std::shared_ptr<Node>& inCondition,
        const std::shared_ptr<Node>& inUpdate, const std::shared_ptr<ScopeNode>& inScope) : Node(ENodeType::For)
    {
        init = inInit;
        condition = inCondition;
        update = inUpdate;
        scope = inScope;
    }

    std::vector<std::shared_ptr<Node>> ForNode::GetChildren() const
    {
        return {init,condition,update,scope};
    }

    LayoutNode::LayoutNode(const ELayoutType& inLayoutType, const std::shared_ptr<DeclarationNode>& inDeclaration,
        const std::unordered_map<std::string, std::string>& inTags) : Node(ENodeType::Layout)
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
        for (auto &[fst, snd] : tags)
        {
            tagsStr += fst + "-" + snd;
        } 
        return hashCombine(Node::ComputeSelfHash(),static_cast<int>(layoutType),tagsStr);
    }

    PushConstantNode::PushConstantNode(const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations,
        const std::unordered_map<std::string, std::string>& inTags) : Node(ENodeType::PushConstant)
    {
        declarations = inDeclarations;
        tags = inTags;
    }

    std::vector<std::shared_ptr<Node>> PushConstantNode::GetChildren() const
    {
        return mapVector<std::shared_ptr<Node>,std::shared_ptr<DeclarationNode>>(declarations,[](const std::shared_ptr<DeclarationNode>& d)
        {
            return d;
        });
    }

    size_t PushConstantNode::ComputeSelfHash() const
    {
        std::string tagsStr{};
        for (auto &[fst, snd] : tags)
        {
            tagsStr += fst + "-" + snd;
        } 
        return hashCombine(Node::ComputeSelfHash(),tagsStr);
    }

    size_t PushConstantNode::GetSize() const
    {
        size_t size = 0;
        for (auto &declarationNode : declarations)
        {
            size += declarationNode->GetSize();
        }

        return size;
    }
    
    DefineNode::DefineNode(const std::string& identifier, const std::shared_ptr<Node>& inExpression) : Node(ENodeType::Define)
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
        return hashCombine(Node::ComputeSelfHash(),id);
    }

    IncludeNode::IncludeNode(const std::string& inSourceFile, const std::string& inTargetFile) : Node(ENodeType::Include)
    {
        sourceFile=  inSourceFile;
        targetFile = inTargetFile;
    }

    std::vector<std::shared_ptr<Node>> IncludeNode::GetChildren() const
    {
        return {};
    }

    size_t IncludeNode::ComputeSelfHash() const
    {
        return hashCombine(Node::ComputeSelfHash(),sourceFile,targetFile);
    }

    ConditionalNode::ConditionalNode(const std::shared_ptr<Node>& inCondition, const std::shared_ptr<Node>& inLeft,
        const std::shared_ptr<Node>& inRight) : Node(ENodeType::Conditional)
    {
        condition = inCondition;
        left = inLeft;
        right = inRight;
    }

    std::vector<std::shared_ptr<Node>> ConditionalNode::GetChildren() const
    {
        return {condition,left,right};
    }

    ReturnNode::ReturnNode(const std::shared_ptr<Node>& inExpression) : Node(ENodeType::Return)
    {
        expression = inExpression;
    }

    std::vector<std::shared_ptr<Node>> ReturnNode::GetChildren() const
    {
        return {expression};
    }

    ArrayLiteralNode::ArrayLiteralNode(const std::vector<std::shared_ptr<Node>>& inNodes) : Node(ENodeType::ArrayLiteral)
    {
        nodes = inNodes;
    }

    std::vector<std::shared_ptr<Node>> ArrayLiteralNode::GetChildren() const
    {
        return nodes;
    }

    NoOpNode::NoOpNode() : Node(ENodeType::NoOp)
    {
    }

    std::vector<std::shared_ptr<Node>> NoOpNode::GetChildren() const
    {
        return {};
    }

    NamedScopeNode::NamedScopeNode(EScopeType inScopeType, const std::shared_ptr<ScopeNode>& inScope) : Node(ENodeType::NamedScope)
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
        return hashCombine(Node::ComputeSelfHash(),static_cast<int>(scopeType));
    }
}
