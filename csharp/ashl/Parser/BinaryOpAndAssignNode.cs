namespace rsl.Parser;

public class BinaryOpAndAssignNode : HasLeftNode
{
    public Node Right;
    public EBinaryOp Op;
    
    public BinaryOpAndAssignNode(Node left,Node right,EBinaryOp op) : base(left, ENodeType.BinaryOpAndAssign)
    {
        Right = right;
        Op = op;
    }

    public override IEnumerable<Node> GetChildren()
    {
        return [Left, Right];
    }
}