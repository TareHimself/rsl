using rsl.Parser;
using rsl.Tokenizer;

namespace rsl.Generator;

public class GlslGenerator : Generator
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
            EDeclarationType.Float2 => "vec2",
            EDeclarationType.Int2 => "ivec2",
            EDeclarationType.Float3 => "vec3",
            EDeclarationType.Int3 => "ivec3",
            EDeclarationType.Float4 => "vec4",
            EDeclarationType.Int4 => "ivec4",
            EDeclarationType.Mat3 => "mat3",
            EDeclarationType.Mat4 => "mat4",
            EDeclarationType.Void => "void",
            EDeclarationType.Buffer => "buffer",
            EDeclarationType.Sampler2D => "sampler2D",
            _ => throw new ArgumentOutOfRangeException()
        };
    }

    public virtual string GenerateDeclaration(DeclarationNode node,int depth)
    {
        if (node is BlockDeclarationNode asBlockDeclaration)
        {
            return $"{GetDeclarationType(node)} " + "{\n" + asBlockDeclaration.Declarations.Aggregate("",(t,nodeDeclaration) => t + Tabs(depth + 1) + GenerateDeclaration(nodeDeclaration,depth) + ";\n") + Tabs(depth) + "}" + $" {node.Name}"+ node.Count switch
            {
                0 => "[]",
                1 => "",
                _ => $"[{node.Count}]"
            };
        }

        if (node is BufferDeclarationNode asBufferDeclaration)
        {
            return $"{GetDeclarationType(node)} {node.Name}" + "{\n" + asBufferDeclaration.Declarations.Aggregate("",(t,nodeDeclaration) => t + Tabs(depth + 1) + GenerateDeclaration(nodeDeclaration,depth) + ";\n") + Tabs(depth) + "}";
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

                    return $"{GenerateExpression(new IdentifierNode(node.Identifier))}( {arguments} )";
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
                if (expression is IdentifierNode node)
                {
                    var tokenType = Token.KeywordToTokenType(node.Identity);
                    
                    if (tokenType == null) return $"{node.Identity}";

                    return tokenType switch
                    {
                        TokenType.TypeFloat2 => "vec2",
                        TokenType.TypeFloat3 => "vec3",
                        TokenType.TypeFloat4 => "vec4",
                        TokenType.TypeInt2 => "ivec2",
                        TokenType.TypeInt3 => "ivec3",
                        TokenType.TypeInt4 => "ivec4",
                        _ => $"{node.Identity}"
                    };
                }
            }
                goto default;
            case ENodeType.Declaration:
            {
                if (expression is DeclarationNode node) return GenerateDeclaration(node,0);
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
                if (expression is ConstNode node) return $"const {GenerateDeclaration(node.Declaration,0)}";
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

    public virtual string GenerateStatement(Node statement,int depth)
    {
        return $"{Tabs(depth)}{GenerateExpression(statement)};";
    }

    public virtual string GenerateFor(ForNode node,int depth)
    {
        return $"{Tabs(depth)}for({GenerateExpression(node.Initial)};{GenerateExpression(node.Condition)};{GenerateExpression(node.Update)})\n" + GenerateScope(node.Scope,depth);
    }
    
    public virtual string GenerateIf(IfNode node,int depth)
    {
        var result = $"{Tabs(depth)}if({GenerateExpression(node.Condition)})" + GenerateScope(node.Scope,depth);

        if (node.Else is NoOpNode) return result;

        return result + $"{Tabs(depth)}else " + (node.Else is ScopeNode
            ? GenerateScope((ScopeNode)node.Else,depth)
            : "\n" + GenerateIf((IfNode)node.Else,depth));
    }

    public virtual string GenerateScope(ScopeNode node,int depth)
    {
        var result = "\n" + Tabs(depth) + "{\n";
        foreach (var nodeStatement in node.Statements)
        {
            if (nodeStatement is ScopeNode nestedScope)
            {
                result += GenerateScope(nestedScope,depth + 1);
                continue;
            }

            if (nodeStatement is ForNode forNode)
            {
                result += GenerateFor(forNode,depth + 1);
                continue;
            }
            
            if (nodeStatement is IfNode ifNode)
            {
                result += GenerateIf(ifNode,depth + 1);
                continue;
            }

            result += GenerateStatement(nodeStatement,depth + 1) + "\n";
        }

        return result + Tabs(depth) + "}\n";
    }

    public virtual string GenerateFunction(FunctionNode node,int depth)
    {
        var result = $"{Tabs(depth)}{GenerateDeclaration(node.ReturnDeclaration,depth)}{node.Name}(";
        foreach (var functionArgumentNode in node.Arguments)
        {
            result += GenerateFunctionArgument(functionArgumentNode);
            if (functionArgumentNode != node.Arguments.Last()) result += " , ";
        }

        result+= ")";
        
        return result + GenerateScope(node.Scope,depth);
    }

    public virtual string GenerateStruct(StructNode node,int depth)
    {
        return $"struct {node.Name}" + "{\n" + node.Declarations
                   .Select(nodeDeclaration => Tabs(depth + 1) + GenerateDeclaration(nodeDeclaration,depth) + ";\n")
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

    public string GenerateLayout(LayoutNode node,int depth)
    {
        var layoutTags = "";
        foreach (var tag in node.Tags)
            if (tag.Key is "set" or "location" or "binding" or "scalar" or "push_constant" or "buffer_reference")
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
                ELayoutType.ReadOnly => "readonly",
                _ => throw new ArgumentOutOfRangeException()
            } + $" {GenerateDeclaration(node.Declaration,depth)};\n";
    }
    
    public string GenerateDefine(DefineNode node)
    {
        return $"#define {node.Identifier} {GenerateExpression(node.Expression)}\n";
    }

    public virtual string GeneratePushConstant(PushConstantNode node,int depth)
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
                   .Select(nodeDeclaration => Tabs(depth + 1) + GenerateDeclaration(nodeDeclaration,depth) + ";\n")
                   .Aggregate("", (c, d) => c + d) +
               Tabs(depth) + "} push;\n";
    }

    public override string Run(List<Node> nodes)
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
                        result += GenerateFunction(asFunction,0);
                        break;
                    }
                }
                    goto default;
                case ENodeType.Layout:
                {
                    if (node is LayoutNode asLayout)
                    {
                        result += GenerateLayout(asLayout,0);
                        break;
                    }
                }
                    goto default;
                case ENodeType.Define:
                {
                    if (node is DefineNode asDefine)
                    {
                        result += GenerateDefine(asDefine);
                        break;
                    }
                }
                    goto default;
                case ENodeType.PushConstant:
                {
                    if (node is PushConstantNode asPushConstant)
                    {
                        result += GeneratePushConstant(asPushConstant,0);
                        break;
                    }
                }
                    goto default;
                case ENodeType.Struct:
                {
                    if (node is StructNode asStruct)
                    {
                        result += GenerateStruct(asStruct,0);
                        break;
                    }
                }
                    goto default;
                case ENodeType.Assign:
                {
                    result += GenerateStatement(node,0);
                    break;
                }
                default:
                    throw new ArgumentOutOfRangeException();
            }

            result += "\n";
        }

        return result;
    }
}