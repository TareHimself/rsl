using System.Globalization;
using ashl.Parser;

namespace ashl.Generator;

public class Generator
{
    private Dictionary<int, string> ComputedTabs = new();
    private Mutex TabsMutex = new();
    

    public virtual string Tabs(int depth)
    {
        lock (ComputedTabs)
        {
            if (ComputedTabs.TryGetValue(depth, value: out var tabs)) return tabs;
            
            var result = "";
        
            for (var i = 0; i < depth; i++)
            {
                result += '\t';
            }
            
            ComputedTabs.Add(depth,result);
            return result;
        }
    }

    public virtual string GetDeclarationType(DeclarationNode node)
    {
        return node.DeclarationType switch
        {
            EDeclarationType.Struct => node is StructDeclarationNode structDeclarationNode ? structDeclarationNode.StructName : throw new Exception("Declaration type is struct but node is not StructNode"),
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
            _ => throw new ArgumentOutOfRangeException()
        };
    }
    public virtual string GenerateDeclaration(DeclarationNode node)
    {
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
                {
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
            }
                goto default;
            case ENodeType.Return:
            {
                if (expression is ReturnNode node)
                {
                    return $"return {GenerateExpression(node.Expression)}";
                }
            }
                goto default;
            case ENodeType.Assign:
            {
                if (expression is AssignNode node)
                {
                    return GenerateAssign(node);
                }
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
                        
                        if (nodeArgument != node.Arguments.Last())
                        {
                            arguments += " , ";
                        }
                    }

                    return $"{GenerateExpression(node.Identifier)}( {arguments} )";
                }
            }
                goto default;
            case ENodeType.Access:
            {
                if (expression is AccessNode node)
                {
                    return $"{GenerateExpression(node.Left)}.{GenerateExpression(node.Right)}";
                }
            }
                goto default;
            case ENodeType.Index:
            {
                if (expression is IndexNode node)
                {
                    return $"{GenerateExpression(node.Left)}[{GenerateExpression(node.IndexExpression)}]";
                }
            }
                goto default;
            case ENodeType.Identifier:
            {
                if (expression is IdentifierNode node)
                {
                    return $"{node.Identity}";
                }
            }
                goto default;
            case ENodeType.Declaration:
            {
                if (expression is DeclarationNode node)
                {
                    return GenerateDeclaration(node);
                }
            }
                goto default;
            case ENodeType.Include:
            {
                if (expression is IncludeNode node)
                {
                    return $"#include \"{node.File}\"";
                }
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
                if (expression is IntLiteral node)
                {
                    return $"{node.Value}";
                }
            }
                goto default;
            case ENodeType.Const:
            {
                if (expression is ConstNode node)
                {
                    return $"const {GenerateDeclaration(node.Declaration)}";
                }
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
                        if (nodeExpression != node.Expressions.Last())
                        {
                            arrayStr += " , ";
                        }

                        
                    }
                    return arrayStr + " }";
                }
            }
                goto default;
            case ENodeType.Negate:
            {
                if (expression is NegateNode node)
                {
                    return $"-{GenerateExpression(node.Expression)}";
                }
            }
                goto default;
            case ENodeType.Precedence:
            {
                if (expression is PrecedenceNode node)
                {
                    return $"( {GenerateExpression(node.Expression)} )";
                }
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
    
    public virtual void GenerateScope(List<string> result,ScopeNode node)
    {
        result.Add("{");
        foreach (var nodeStatement in node.Statements)
        {
            if (nodeStatement is ScopeNode nestedScope)
            {
                GenerateScope(result,nestedScope);
                continue;
            }
            
            result.Add(GenerateStatement(nodeStatement));
        }
        result.Add("}");
    }
    
    public virtual void GenerateFunction(List<string> result,FunctionNode node)
    {
        {
            var currentLine = $"{GenerateDeclaration(node.ReturnDeclaration)} {node.Name}(";
            foreach (var functionArgumentNode in node.Arguments)
            {
                currentLine += GenerateFunctionArgument(functionArgumentNode);
                if (functionArgumentNode != node.Arguments.Last())
                {
                    currentLine += " , ";
                }
            }

            currentLine += ")";
            result.Add(currentLine);
        }
        
        GenerateScope(result,node.Scope);
    }
    
    public virtual void GenerateStruct(List<string> result,StructNode node)
    {
        result.Add($"struct {node.Name}" + "{");
        result.AddRange(node.Declarations.Select(nodeDeclaration => GenerateDeclaration(nodeDeclaration) + ";"));
        result.Add("};");
    }
    
    public virtual string GenerateAssign(AssignNode node)
    {
        return $"{GenerateExpression(node.Left)} = {GenerateExpression(node.Right)}";
    }
    
    public virtual string GenerateLayout(LayoutNode node)
    {
        var layoutTags = "";
        foreach (var tag in node.Tags)
        {
            if (tag.Key is "set" or "location" or "binding")
            {
                layoutTags += $"{tag.Key}={tag.Value} , ";
            }
        }

        if (layoutTags.Length != 0) layoutTags = layoutTags[..^3];
        
        return $"layout({layoutTags}) " + (node.LayoutType switch
            {
                ELayoutType.In => "in",
                ELayoutType.Out => "out",
                ELayoutType.Uniform => "uniform",
                _ => throw new ArgumentOutOfRangeException()
            }) + $" {GenerateDeclaration(node.Declaration)};";
    }

    public List<Node> ExtractScopes(ModuleNode moduleNode, EScopeType targetScope)
    {
        NamedScopeNode? scope = null;
        var curStatementIdx = moduleNode.Statements.Length - 1;
        
        for (;curStatementIdx > -1; curStatementIdx--)
        {
            if (moduleNode.Statements[curStatementIdx] is not NamedScopeNode namedScope ||
                namedScope.ScopeType != targetScope) continue;
            
            scope = namedScope;
            break;
        }
        
        if (scope == null)
        {
            return [];
        }

        var relevantStatements = moduleNode.Statements.Where((node, idx) =>
        {
            if (idx > curStatementIdx) return false;

            if (node is NamedScopeNode namedScopeNode)
            {
                return namedScopeNode.ScopeType == targetScope;
            }

            return true;
        }).SelectMany<Node,Node>((node) =>
        {
            if (node is NamedScopeNode namedScope)
            {
                return namedScope.Statements;
            }

            return [node];
        });

        return relevantStatements.ToList();
    }
    
    public List<string> Run(ModuleNode moduleNode,EScopeType targetScope)
    {
        var lines = new List<string>();

        var scopeNodes = ExtractScopes(moduleNode,targetScope);
        
        foreach (var node in scopeNodes)
        {
            switch (node.NodeType)
            {
                case ENodeType.Function:
                {
                    if (node is FunctionNode asFunction)
                    {
                        GenerateFunction(lines,asFunction);
                        break;
                    }
                }
                    goto default;
                case ENodeType.Layout:
                {
                    if (node is LayoutNode asLayout)
                    {
                        lines.Add(GenerateLayout(asLayout));
                        break;
                    }
                }
                    goto default;
                case ENodeType.Struct:
                {
                    if (node is StructNode asStruct)
                    {
                        GenerateStruct(lines,asStruct);
                        break;
                    }
                }
                    goto default;
                case ENodeType.Assign:
                {
                    lines.Add(GenerateStatement(node));
                    break;
                }
                    goto default;
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }
        return lines;
    }
}