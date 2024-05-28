﻿namespace ashl.Parser;

public class ConditionalNode : HasLeftNode
{
    public Node Condition;
    public Node Right;

    public ConditionalNode(Node condition,Node left, Node right) : base(left, ENodeType.Conditional)
    {
        Condition = condition;
        Right = right;
    }

    public override IEnumerable<Node> GetChildren() => [Condition, Left, Right];
}