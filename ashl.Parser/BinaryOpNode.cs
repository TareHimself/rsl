using ashl.Tokenizer;

namespace ashl.Parser;

/// <summary>
/// <see cref="HasLeftNode.Left"/> <see cref="EBinaryOp">Operation</see> <see cref="Right"/>
/// </summary>
public class BinaryOpNode : HasLeftNode
{
    public EBinaryOp Op;
    public Node Right;

    public BinaryOpNode(Node left,Node right,EBinaryOp op) : base(left,ENodeType.BinaryOp)
    {
        Right = right;
        Op = op;
    }

    public BinaryOpNode(Node left, Node right, TokenType op) : this(left, right, op switch
    {
        TokenType.OpAnd => EBinaryOp.And,
        TokenType.OpOr => EBinaryOp.Or,
        TokenType.OpNot => EBinaryOp.Not,
        TokenType.OpEqual => EBinaryOp.Equal,
        TokenType.OpNotEqual => EBinaryOp.NotEqual,
        TokenType.OpGreater => EBinaryOp.Greater,
        TokenType.OpGreaterEqual => EBinaryOp.GreaterEqual,
        TokenType.OpLess => EBinaryOp.Less,
        TokenType.OpLessEqual => EBinaryOp.LessEqual,
        TokenType.OpAdd => EBinaryOp.Add,
        TokenType.OpSubtract => EBinaryOp.Subtract,
        TokenType.OpDivide => EBinaryOp.Divide,
        TokenType.OpMultiply => EBinaryOp.Multiply,
        _ => throw new Exception("Unknown Token Type")
    })
    {
        
    }

    public override IEnumerable<Node> GetChildren() => [Left,Right];
}