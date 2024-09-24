#pragma once
#include <vector>
#include <memory>
#include <string>

#include "Token.hpp"

namespace rsl
{
    enum class NodeType
    {
        Unknown,
        NoOp,
        BinaryOp,
        Module,
        Function,
        FunctionArgument,
        Return,
        Assign,
        BinaryOpAndAssign,
        Layout,
        Call,
        Access,
        Index,
        Scope,
        NamedScope,
        Identifier,
        Struct,
        Declaration,
        Include,
        FloatLiteral,
        IntLiteral,
        Const,
        ArrayLiteral,
        Negate,
        Precedence,
        PushConstant,
        For,
        Increment,
        Decrement,
        Discard,
        If,
        Conditional,
        Define,
        BooleanLiteral,
    };

    struct Node
    {
    protected:
        [[nodiscard]] virtual size_t ComputeSelfHash() const;

    public:
        virtual ~Node() = default;
        NodeType nodeType;

        explicit Node(const NodeType& inNodeType);

        [[nodiscard]] virtual std::vector<std::shared_ptr<Node>> GetChildren() const = 0;
        [[nodiscard]] virtual size_t ComputeHash() const;
    };


    struct ModuleNode : Node
    {
        explicit ModuleNode(const std::vector<std::shared_ptr<Node>>& inStatements);
        std::vector<std::shared_ptr<Node>> statements{};
        [[nodiscard]] std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };


    struct ScopeNode : Node
    {
        explicit ScopeNode(const std::vector<std::shared_ptr<Node>>& inStatements);
        std::vector<std::shared_ptr<Node>> statements{};
        [[nodiscard]] std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    enum struct EDeclarationType
    {
        Struct,
        Block,
        Buffer,
        Float,
        Int,
        Float2,
        Int2,
        Float3,
        Int3,
        Float4,
        Int4,
        Mat3,
        Mat4,
        Boolean,
        Void,
        Sampler,
        Texture2D,
        Sampler2D
    };


    struct DeclarationNode : Node
    {
        int declarationCount;
        EDeclarationType declarationType;
        std::string declarationName{};

        static EDeclarationType TokenTypeToDeclarationType(TokenType tokenType);
        DeclarationNode(const EDeclarationType& inDeclarationType, const std::string& inDeclarationName,
                        const int& inDeclarationCount);

        DeclarationNode(const Token& typeToken, const std::string& inDeclarationName, const int& inDeclarationCount);

        [[nodiscard]] virtual uint64_t GetSize() const;

        virtual std::string GetTypeName();

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
        size_t ComputeSelfHash() const override;
    };

    struct StructNode : Node
    {
        std::vector<std::shared_ptr<DeclarationNode>> declarations{};
        std::string name{};
        [[nodiscard]] uint64_t GetSize() const;
        StructNode(const std::string& inName, const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations);
        std::vector<std::shared_ptr<Node>> GetChildren() const override;
        size_t ComputeSelfHash() const override;
    };

    struct StructDeclarationNode : DeclarationNode
    {
        uint64_t GetSize() const override;
        std::string GetTypeName() override;
        [[nodiscard]] std::vector<std::shared_ptr<Node>> GetChildren() const override;

        std::shared_ptr<StructNode> structNode{};
        std::string structName{};
        explicit StructDeclarationNode(const std::string& inStructName, const std::string& inDeclarationName,
                                       const int& inCount);
        explicit StructDeclarationNode(const std::shared_ptr<StructNode>& inStruct,
                                       const std::string& inDeclarationName, const int& inCount);
        size_t ComputeSelfHash() const override;
    };

    struct BufferDeclarationNode : DeclarationNode
    {
        std::vector<std::shared_ptr<DeclarationNode>> declarations{};
        explicit BufferDeclarationNode(const std::string& inName, const int& inCount,
                                       const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations);
        std::vector<std::shared_ptr<Node>> GetChildren() const override;
        std::string GetTypeName() override;
    };

    struct BlockDeclarationNode : DeclarationNode
    {
        std::vector<std::shared_ptr<DeclarationNode>> declarations{};
        [[nodiscard]] uint64_t GetSize() const override;
        std::string GetTypeName() override;
        explicit BlockDeclarationNode(const std::string& inDeclarationName, const int& inCount,
                                      const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations);
    };


    struct AssignNode : Node
    {
        std::shared_ptr<Node> target;
        std::shared_ptr<Node> value;
        AssignNode(const std::shared_ptr<Node>& inTarget, const std::shared_ptr<Node>& inValue);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    enum class EBinaryOp
    {
        Multiply,
        Divide,
        Add,
        Subtract,
        Mod,
        And,
        Or,
        Not,
        Equal,
        NotEqual,
        Less,
        LessEqual,
        Greater,
        GreaterEqual
    };

    struct BinaryOpNode : Node
    {
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        EBinaryOp op;


        BinaryOpNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inRight, const EBinaryOp& inOp);

        BinaryOpNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inRight, const TokenType& inOp);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;

        size_t ComputeSelfHash() const override;
    };

    struct FunctionArgumentNode : Node
    {
        bool isInput;
        std::shared_ptr<DeclarationNode> declaration{};

        explicit FunctionArgumentNode(bool inIsInput, const std::shared_ptr<DeclarationNode>& inDeclaration);
        [[nodiscard]] std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct FunctionNode : Node
    {
        std::shared_ptr<DeclarationNode> returnDeclaration{};
        std::string name{};
        std::vector<std::shared_ptr<FunctionArgumentNode>> arguments{};
        std::shared_ptr<ScopeNode> scope{};

        explicit FunctionNode(const std::shared_ptr<DeclarationNode>& inReturnDeclaration, const std::string& inName,
                              const std::vector<std::shared_ptr<FunctionArgumentNode>>& inArguments,
                              const std::shared_ptr<ScopeNode>& inScope);
        [[nodiscard]] std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct IdentifierNode : Node
    {
        std::string id{};

        IdentifierNode(const std::string& inId);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;

        size_t ComputeSelfHash() const override;
    };

    struct AccessNode : Node
    {
        std::shared_ptr<Node> left{};
        std::shared_ptr<Node> right{};

        AccessNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inRight);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct IndexNode : Node
    {
        std::shared_ptr<Node> left{};
        std::shared_ptr<Node> indexExpression{};

        IndexNode(const std::shared_ptr<Node>& inLeft, const std::shared_ptr<Node>& inIndexExpression);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct ConstNode : Node
    {
        std::shared_ptr<DeclarationNode> declaration{};

        ConstNode(const std::shared_ptr<DeclarationNode>& inDeclaration);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct IntegerLiteralNode : Node
    {
        int data{};

        IntegerLiteralNode(const int& inData);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct BooleanLiteralNode : Node
    {
        bool data{};

        BooleanLiteralNode(const bool& inData);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct FloatLiteralNode : Node
    {
        float data{};

        FloatLiteralNode(const float& inData);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct CallNode : Node
    {
        std::shared_ptr<IdentifierNode> identifier{};
        std::vector<std::shared_ptr<Node>> args{};

        CallNode(const std::shared_ptr<IdentifierNode>& inIdentifier, const std::vector<std::shared_ptr<Node>>& inArgs);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct IncrementNode : Node
    {
        bool isPrefix;
        std::shared_ptr<Node> target{};

        IncrementNode(bool inIsPrefix, const std::shared_ptr<Node>& inTarget);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;

        size_t ComputeSelfHash() const override;
    };

    struct DecrementNode : Node
    {
        bool isPrefix;
        std::shared_ptr<Node> target{};

        DecrementNode(bool inIsPrefix, const std::shared_ptr<Node>& inTarget);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;

        size_t ComputeSelfHash() const override;
    };

    struct NegateNode : Node
    {
        std::shared_ptr<Node> target{};

        NegateNode(const std::shared_ptr<Node>& inTarget);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct PrecedenceNode : Node
    {
        std::shared_ptr<Node> target{};

        PrecedenceNode(const std::shared_ptr<Node>& inTarget);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct DiscardNode : Node
    {
        DiscardNode();

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct IfNode : Node
    {
        std::shared_ptr<Node> condition{};
        std::shared_ptr<ScopeNode> scope{};
        std::shared_ptr<Node> elseNode{};

        IfNode(const std::shared_ptr<Node>& inCondition, const std::shared_ptr<ScopeNode>& inScope,
               const std::shared_ptr<Node>& inElseScope = {});

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };


    struct ForNode : Node
    {
        std::shared_ptr<Node> init{};
        std::shared_ptr<Node> condition{};
        std::shared_ptr<Node> update{};
        std::shared_ptr<ScopeNode> scope{};

        ForNode(const std::shared_ptr<Node>& inInit, const std::shared_ptr<Node>& inCondition,
                const std::shared_ptr<Node>& inUpdate, const std::shared_ptr<ScopeNode>& inScope);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };


    enum class ELayoutType
    {
        Uniform,
        Readonly,
        Input,
        Output
    };

    struct LayoutNode : Node
    {
        ELayoutType layoutType;
        std::unordered_map<std::string, std::string> tags{};
        std::shared_ptr<DeclarationNode> declaration{};

        LayoutNode(const ELayoutType& inLayoutType, const std::shared_ptr<DeclarationNode>& inDeclaration,
                   const std::unordered_map<std::string, std::string>& inTags);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;

        size_t ComputeSelfHash() const override;
    };

    struct PushConstantNode : Node
    {
        std::vector<std::shared_ptr<DeclarationNode>> declarations{};
        std::unordered_map<std::string, std::string> tags{};

        PushConstantNode(const std::vector<std::shared_ptr<DeclarationNode>>& inDeclarations,
                         const std::unordered_map<std::string, std::string>& inTags);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;

        size_t ComputeSelfHash() const override;

        size_t GetSize() const;
    };

    struct DefineNode : Node
    {
        std::string id;
        std::shared_ptr<Node> expression{};

        DefineNode(const std::string& identifier, const std::shared_ptr<Node>& inExpression);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;

        size_t ComputeSelfHash() const override;
    };

    struct IncludeNode : Node
    {
        std::string sourceFile{};
        std::string targetFile{};

        IncludeNode(const std::string& inSourceFile, const std::string& inTargetFile);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;

        size_t ComputeSelfHash() const override;
    };

    struct ConditionalNode : Node
    {
        std::shared_ptr<Node> condition{};
        std::shared_ptr<Node> left{};
        std::shared_ptr<Node> right{};

        ConditionalNode(const std::shared_ptr<Node>& inCondition, const std::shared_ptr<Node>& inLeft,
                        const std::shared_ptr<Node>& inRight);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct ReturnNode : Node
    {
        std::shared_ptr<Node> expression{};

        ReturnNode(const std::shared_ptr<Node>& inExpression);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct ArrayLiteralNode : Node
    {
        std::vector<std::shared_ptr<Node>> nodes{};

        ArrayLiteralNode(const std::vector<std::shared_ptr<Node>>& inNodes);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };

    struct NoOpNode : Node
    {
        NoOpNode();

        std::vector<std::shared_ptr<Node>> GetChildren() const override;
    };


    enum class EScopeType
    {
        Fragment,
        Vertex
    };

    struct NamedScopeNode : Node
    {
        EScopeType scopeType;
        std::shared_ptr<ScopeNode> scope{};

        NamedScopeNode(EScopeType inScopeType, const std::shared_ptr<ScopeNode>& inScope);

        std::vector<std::shared_ptr<Node>> GetChildren() const override;

        size_t ComputeSelfHash() const override;
    };
}
