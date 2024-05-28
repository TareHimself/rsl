using ashl.Parser;

namespace ashl.Generator;

public class Generator
{
    private readonly Dictionary<int, string> ComputedTabs = new();
    private Mutex TabsMutex = new();


    public virtual string Tabs(int depth)
    {
        lock (ComputedTabs)
        {
            if (ComputedTabs.TryGetValue(depth, out var tabs)) return tabs;

            var result = "";

            for (var i = 0; i < depth; i++) result += '\t';

            ComputedTabs.Add(depth, result);
            return result;
        }
    }

    public virtual string GetDeclarationType(DeclarationNode node)
    {
        return node.DeclarationType switch
        {
            EDeclarationType.Struct => node is StructDeclarationNode structDeclarationNode
                ? structDeclarationNode.StructName
                : throw new Exception("Declaration type is struct but node is not StructNode"),
            EDeclarationType.Block => node is BlockDeclarationNode blockDeclarationNode
                ? blockDeclarationNode.BlockName
                : throw new Exception("Declaration type is block but node is not BlockNode"),
            EDeclarationType.Float => "float",
            EDeclarationType.Int => "int",
            EDeclarationType.Vec2f => "vec2",
            EDeclarationType.Vec2i => "ivec2",
            EDeclarationType.Vec3f => "vec3",
            EDeclarationType.Vec3i => "ivec3",
            EDeclarationType.Vec4f => "vec4",
            EDeclarationType.Vec4i => "ivec4",
            EDeclarationType.Mat3 => "mat3",
            EDeclarationType.Mat4 => "mat4",
            EDeclarationType.Void => "void",
            EDeclarationType.Sampler2D => "sampler2D",
            _ => throw new ArgumentOutOfRangeException()
        };
    }

    public virtual string GenerateDeclaration(DeclarationNode node)
    {
        if (node is BlockDeclarationNode asBlockDeclaration)
        {
            return $"{GetDeclarationType(node)} " + "{\n" + asBlockDeclaration.Declarations.Aggregate("",(t,nodeDeclaration) => t + GenerateDeclaration(nodeDeclaration) + ";\n") + "}" + $" {node.Name}"+ node.Count switch
            {
                0 => "[]",
                1 => "",
                _ => $"[{node.Count}]"
            };
        }
        return $"{GetDeclarationType(node)} {node.Name}" + node.Count switch
        {
            0 => "[]",
            1 => "",
            _ => $"[{node.Count}]"
        };
    }

    public virtual string GenerateFunctionArgument(FunctionArgumentNode node)
    {
        return (node.IsInput ? "in " : "out ") + GetDeclarationType(node.Declaration) +
               (node.Declaration.Count > 1 ? $"[{node.Declaration.Count}]" : "") + $" {node.Declaration.Name}";
    }

    public virtual string GenerateExpression(Node expression)
    {
        switch (expression.NodeType)
        {
            case ENodeType.BinaryOp:
            {
                if (expression is BinaryOpNode node)
                    return $"{GenerateExpression(node.Left)} " + node.Op switch
                        {
                            EBinaryOp.Multiply => "*",
                            EBinaryOp.Divide => "/",
                            EBinaryOp.Add => "+",
                            EBinaryOp.Subtract => "-",
                            EBinaryOp.Mod => "%",
                            EBinaryOp.And => "&&",
                            EBinaryOp.Or => "||",
                            EBinaryOp.Not => "!",
                            EBinaryOp.Equal => "==",
                            EBinaryOp.NotEqual => "!=",
                            EBinaryOp.Less => "<",
                            EBinaryOp.LessEqual => "<=",
                            EBinaryOp.Greater => ">",
                            EBinaryOp.GreaterEqual => ">=",
                            _ => throw new ArgumentOutOfRangeException()
                        } + $" {GenerateExpression(node.Right)}";
            }
                goto default;
            case ENodeType.Return:
            {
                if (expression is ReturnNode node) return $"return {GenerateExpression(node.Expression)}";
            }
                goto default;
            case ENodeType.Assign:
            {
                if (expression is AssignNode node) return GenerateAssign(node);
            }
                goto default;
            case ENodeType.Call:
            {
                if (expression is CallNode node)
                {
                    var arguments = "";
                    foreach (var nodeArgument in node.Arguments)
                    {
                        arguments += GenerateExpression(nodeArgument);

                        if (nodeArgument != node.Arguments.Last()) arguments += " , ";
                    }

                    return $"{GenerateExpression(node.Identifier)}( {arguments} )";
                }
            }
                goto default;
            case ENodeType.Access:
            {
                if (expression is AccessNode node)
                    return $"{GenerateExpression(node.Left)}.{GenerateExpression(node.Right)}";
            }
                goto default;
            case ENodeType.Index:
            {
                if (expression is IndexNode node)
                    return $"{GenerateExpression(node.Left)}[{GenerateExpression(node.IndexExpression)}]";
            }
                goto default;
            case ENodeType.Identifier:
            {
                if (expression is IdentifierNode node) return $"{node.Identity}";
            }
                goto default;
            case ENodeType.Declaration:
            {
                if (expression is DeclarationNode node) return GenerateDeclaration(node);
            }
                goto default;
            case ENodeType.Include:
            {
                if (expression is IncludeNode node) return $"#include \"{node.File}\"";
            }
                goto default;
            case ENodeType.FloatLiteral:
            {
                if (expression is FloatLiteral node)
                {
                    var result = $"{node.Value}";
                    if (!result.Contains('.')) result += ".0";
                    return result;
                }
            }
                goto default;
            case ENodeType.IntLiteral:
            {
                if (expression is IntLiteral node) return $"{node.Value}";
            }
                goto default;
            case ENodeType.Const:
            {
                if (expression is ConstNode node) return $"const {GenerateDeclaration(node.Declaration)}";
            }
                goto default;
            case ENodeType.ArrayLiteral:
            {
                if (expression is ArrayLiteralNode node)
                {
                    var arrayStr = "{ ";
                    foreach (var nodeExpression in node.Expressions)
                    {
                        arrayStr += GenerateExpression(nodeExpression);
                        if (nodeExpression != node.Expressions.Last()) arrayStr += " , ";
                    }

                    return arrayStr + " }";
                }
            }
                goto default;
            case ENodeType.Negate:
            {
                if (expression is NegateNode node) return $"-{GenerateExpression(node.Expression)}";
            }
                goto default;
            case ENodeType.Precedence:
            {
                if (expression is PrecedenceNode node) return $"( {GenerateExpression(node.Expression)} )";
            }
                goto default;
            case ENodeType.Increment:
            {
                if (expression is IncrementNode node)
                    return node.IsPre ? $"++{GenerateExpression(node.Target)}" : $"{GenerateExpression(node.Target)}++";
            }
                goto default;
            case ENodeType.Decrement:
            {
                if (expression is DecrementNode node)
                    return node.IsPre ? $"--{GenerateExpression(node.Target)}" : $"{GenerateExpression(node.Target)}--";
            }
                goto default;
            case ENodeType.Discard:
            {
                if (expression is DiscardNode node) return "discard";
            }
                goto default;
            case ENodeType.Conditional:
            {
                if (expression is ConditionalNode node) return $"{GenerateExpression(node.Condition)} ? {GenerateExpression(node.Left)} : {GenerateExpression(node.Right)}";
            }
                goto default;
            case ENodeType.BinaryOpAndAssign:
            {
                if (expression is BinaryOpAndAssignNode node) return GenerateBinaryOpAndAssign(node);
            }
                goto default;
            default:
                throw new ArgumentOutOfRangeException();
        }
    }

    public virtual string GenerateStatement(Node statement)
    {
        return $"{GenerateExpression(statement)};";
    }

    public virtual string GenerateFor(ForNode node)
    {
        return $"for({GenerateExpression(node.Initial)};{GenerateExpression(node.Condition)};{GenerateExpression(node.Update)})\n" + GenerateScope(node.Scope);
    }
    
    public virtual string GenerateIf(IfNode node)
    {
        var result = $"if({GenerateExpression(node.Condition)})\n" + GenerateScope(node.Scope);
        if (node.Else is NoOpNode) return result;

        return result + $"else " + (node.Else is ScopeNode
            ? GenerateScope((ScopeNode)node.Else)
            : GenerateIf((IfNode)node.Else));
    }

    public virtual string GenerateScope(ScopeNode node)
    {
        var result = "{\n";
        foreach (var nodeStatement in node.Statements)
        {
            if (nodeStatement is ScopeNode nestedScope)
            {
                result += GenerateScope(nestedScope);
                continue;
            }

            if (nodeStatement is ForNode forNode)
            {
                result += GenerateFor(forNode);
                continue;
            }
            
            if (nodeStatement is IfNode ifNode)
            {
                result += GenerateIf(ifNode);
                continue;
            }

            result += GenerateStatement(nodeStatement) + "\n";
        }

        return result + "}\n";
    }

    public virtual string GenerateFunction(FunctionNode node)
    {
        var result = $"{GenerateDeclaration(node.ReturnDeclaration)}{node.Name}(";
        foreach (var functionArgumentNode in node.Arguments)
        {
            result += GenerateFunctionArgument(functionArgumentNode);
            if (functionArgumentNode != node.Arguments.Last()) result += " , ";
        }

        result+= ")";
        
        return result + GenerateScope(node.Scope);
    }

    public virtual string GenerateStruct(StructNode node)
    {
        return $"struct {node.Name}" + "{\n" + node.Declarations
                   .Select(nodeDeclaration => GenerateDeclaration(nodeDeclaration) + ";\n")
                   .Aggregate("", (c, d) => c + d) +
               "};\n";
    }

    public virtual string GenerateAssign(AssignNode node)
    {
        return $"{GenerateExpression(node.Left)} = {GenerateExpression(node.Right)}";
    }
    
    public virtual string GenerateBinaryOpAndAssign(BinaryOpAndAssignNode node)
    {
        return GenerateExpression(node.Left) + node.Op switch
        {
            EBinaryOp.Add => " += ",
            EBinaryOp.Subtract => " -= ",
            EBinaryOp.Divide => " /= ",
            EBinaryOp.Multiply => " *= ",

            _ => throw new ArgumentOutOfRangeException()
        } + GenerateExpression(node.Right);
    }

    public string GenerateLayout(LayoutNode node)
    {
        var layoutTags = "";
        foreach (var tag in node.Tags)
            if (tag.Key is "set" or "location" or "binding" or "scalar" or "push_constant")
            {
                if (tag.Value == "")
                    layoutTags += $"{tag.Key} , ";
                else
                    layoutTags += $"{tag.Key}={tag.Value} , ";
            }

        if (layoutTags.Length != 0) layoutTags = layoutTags[..^3];

        return $"layout({layoutTags}) " + node.LayoutType switch
            {
                ELayoutType.In => "in",
                ELayoutType.Out => "out",
                ELayoutType.Uniform => "uniform",
                _ => throw new ArgumentOutOfRangeException()
            } + $" {GenerateDeclaration(node.Declaration)};\n";
    }

    public virtual string GeneratePushConstant(PushConstantNode node)
    {
        var pushConstantTags = "";
        foreach (var tag in node.Tags)
            if (tag.Key is "scalar")
            {
                if (tag.Value == "")
                    pushConstantTags += $" , {tag.Key}";
                else
                    pushConstantTags += $" , {tag.Key}={tag.Value}";
            }

        return $"layout(push_constant{pushConstantTags}) uniform constant " + "{\n" + node.Data.Declarations
                   .Select(nodeDeclaration => GenerateDeclaration(nodeDeclaration) + ";\n")
                   .Aggregate("", (c, d) => c + d) +
               "} push;\n";
    }

    public List<Node> ExtractScopes(ModuleNode moduleNode, EScopeType targetScope)
    {
        NamedScopeNode? scope = null;
        var curStatementIdx = moduleNode.Statements.Length - 1;

        for (; curStatementIdx > -1; curStatementIdx--)
        {
            if (moduleNode.Statements[curStatementIdx] is not NamedScopeNode namedScope ||
                namedScope.ScopeType != targetScope) continue;

            scope = namedScope;
            break;
        }

        if (scope == null) return [];

        var relevantStatements = moduleNode.Statements.Where((node, idx) =>
        {
            if (idx > curStatementIdx) return false;

            if (node is NamedScopeNode namedScopeNode) return namedScopeNode.ScopeType == targetScope;

            return true;
        }).SelectMany<Node, Node>(node =>
        {
            if (node is NamedScopeNode namedScope) return namedScope.Statements;

            return [node];
        });

        return relevantStatements.ToList();
    }

    public string Run(ModuleNode moduleNode, EScopeType targetScope) =>
        Run(ExtractScopes(moduleNode, targetScope));
    
    public string Run(List<Node> nodes)
    {
        var result = "";

        foreach (var node in nodes)
        {
            switch (node.NodeType)
            {
                case ENodeType.Function:
                {
                    if (node is FunctionNode asFunction)
                    {
                        result += GenerateFunction(asFunction);
                        break;
                    }
                }
                    goto default;
                case ENodeType.Layout:
                {
                    if (node is LayoutNode asLayout)
                    {
                        result += GenerateLayout(asLayout);
                        break;
                    }
                }
                    goto default;
                case ENodeType.PushConstant:
                {
                    if (node is PushConstantNode asPushConstant)
                    {
                        result += GeneratePushConstant(asPushConstant);
                        break;
                    }
                }
                    goto default;
                case ENodeType.Struct:
                {
                    if (node is StructNode asStruct)
                    {
                        result += GenerateStruct(asStruct);
                        break;
                    }
                }
                    goto default;
                case ENodeType.Assign:
                {
                    result += GenerateStatement(node);
                    break;
                }
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

        return result;
    }
}