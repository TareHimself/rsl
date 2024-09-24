using rsl.Parser;
using rsl.Tokenizer;

namespace rsl.Generator;

public abstract class Generator
{
    public string Run(ModuleNode moduleNode, EScopeType targetScope) =>
        Run(moduleNode.ExtractScope(targetScope).Statements.ToList());
    
    public abstract string Run(List<Node> nodes);

}