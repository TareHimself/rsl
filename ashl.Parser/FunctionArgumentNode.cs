namespace ashl.Parser;

/// <summary>
/// <see cref="IsInput">in/out</see> <see cref="Declaration"/>
/// </summary>
public class FunctionArgumentNode : Node
{
    public bool IsInput;
    public DeclarationNode Declaration;
    public FunctionArgumentNode(bool isInput,DeclarationNode declaration) : base(ENodeType.FunctionArgument)
    {
        IsInput = isInput;
        Declaration = declaration;
    }
}