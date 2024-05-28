namespace ashl.Parser;

public static class Extensions
{
    public static void Walk(this Node start, Func<Node, bool> visitor)
    {
        Stack<Node> pending = [];

        foreach (var item in start.GetChildren().Reverse()) pending.Push(item);

        while (pending.Count != 0)
        {
            var node = pending.Pop();
            if (!visitor(node)) return;
            foreach (var item in node.GetChildren().Reverse()) pending.Push(item);
        }
    }

    public static void ResolveStructReferences(this Node start)
    {
        Dictionary<string, StructNode> structs = new();
        start.Walk(node =>
        {
            if (node is StructNode stru)
                if (stru.Name != "" && !structs.ContainsKey(stru.Name))
                    structs.Add(stru.Name, stru);

            if (node is StructDeclarationNode struDecl)
                if (struDecl.StructName != "" && structs.TryGetValue(struDecl.StructName, out var structNode))
                    struDecl.Struct = structNode;
            return true;
        });
    }
}