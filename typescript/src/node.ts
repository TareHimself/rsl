import { ETokenType, TokenDebugInfo } from "./token";

export enum ENodeType {
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
    DeclarationScope,
    NamedScope,
    Identifier,
    Type,
    Struct,
    Declaration,
    Include,
    FloatLiteral,
    IntLiteral,
    BooleanLiteral,
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
    Define
}

export abstract class Node {
    debug: TokenDebugInfo;
    nodeType: ENodeType;
    constructor(nodeType: ENodeType, debug: TokenDebugInfo) {
        this.nodeType = nodeType;
        this.debug = debug;
    }

    abstract getChildren(): Node[];
}



export class NoOpNode extends Node {
    constructor() {
        super(ENodeType.NoOp, TokenDebugInfo.empty())
    }
    getChildren(): Node[] {
        return [];
    }
}

export enum EBinaryOp {
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
}

export class BinaryOpNode extends Node {
    left: Node;
    right: Node;
    constructor(left: Node, right: Node,op: EBinaryOp, debug: TokenDebugInfo) {
        super(ENodeType.BinaryOp, debug);
        this.left = left;
        this.right = right;
    }

    getChildren(): Node[] {
        return [this.left,this.right]
    }

    static TokenTypeToBinaryOp(type: ETokenType) {
        switch (type) {
            case ETokenType.OpAnd:
                return EBinaryOp.And;
            case ETokenType.OpOr:
                return EBinaryOp.Or;
            case ETokenType.OpNot:
                return EBinaryOp.Not;
            case ETokenType.OpEqual:
                return EBinaryOp.Equal;
            case ETokenType.OpNotEqual:
                return EBinaryOp.NotEqual;
            case ETokenType.OpGreater:
                return EBinaryOp.Greater;
            case ETokenType.OpGreaterEqual:
                return EBinaryOp.GreaterEqual;
            case ETokenType.OpLess:
                return EBinaryOp.Less;
            case ETokenType.OpLessEqual:
                return EBinaryOp.LessEqual;
            case ETokenType.OpAdd:
                return EBinaryOp.Add;
            case ETokenType.OpSubtract:
                return EBinaryOp.Subtract;
            case ETokenType.OpDivide:
                return EBinaryOp.Divide;
            case ETokenType.OpMultiply:
                return EBinaryOp.Multiply;
        }
    }

    static FromTokenType(left: Node, right: Node,type: ETokenType,debug: TokenDebugInfo) {
        return new BinaryOpNode(left,right,BinaryOpNode.TokenTypeToBinaryOp(type),debug);
    }
}
export class ModuleNode extends Node {
    filePath: string;
    statements: Node[];

    constructor(filePath: string,statements: Node[],debug: TokenDebugInfo){
        super(ENodeType.Module,debug);
        this.filePath = filePath;
        this.statements = statements;
    }
    getChildren(): Node[] {
        return this.statements;
    }

}
export class FunctionNode extends Node {

    args: FunctionArgumentNode[];
    name: string;
    returnDeclaration: DeclarationNode;
    scope: ScopeNode;

    constructor(name: string,returnDeclaration: DeclarationNode,args: FunctionArgumentNode[],scope: ScopeNode,debug: TokenDebugInfo){
        super(ENodeType.Function,debug);
        this.name = name;
        this.returnDeclaration = returnDeclaration;
        this.args = args;
        this.scope = scope;
    }
    getChildren(): Node[] {
        return [this.returnDeclaration,...this.args,this.scope]
    }

}
export class FunctionArgumentNode extends Node {
    declaration: DeclarationNode;
    isInput: boolean;

    constructor(declaration: DeclarationNode,isInput: boolean,debug: TokenDebugInfo){
        super(ENodeType.FunctionArgument,debug);
        this.declaration = declaration;
        this.isInput = isInput;
    }

    getChildren(): Node[] {
        return [this.declaration]
    }

}

export class ReturnNode extends Node {
    expression: Node;

    constructor(expression: Node, debug: TokenDebugInfo) {
        super(ENodeType.Return, debug);
        this.expression = expression;
    }

    getChildren(): Node[] {
        return [this.expression];
    }
}

export class AssignNode extends Node {
    left: Node;
    right: Node;

    constructor(left: Node, right: Node, debug: TokenDebugInfo) {
        super(ENodeType.Assign, debug);
        this.left = left;
        this.right = right;
    }

    getChildren(): Node[] {
        return [this.left, this.right];
    }
}

export class BinaryOpAndAssignNode extends Node {
    left: Node;
    right: Node;
    op: EBinaryOp;


    static TokenTypeToBinaryOpAssign(type: ETokenType) {
        switch (type) {
            case ETokenType.OpAddAssign:
                return EBinaryOp.Add;
            case ETokenType.OpDivideAssign:
                return EBinaryOp.Divide;
            case ETokenType.OpMultiplyAssign:
                return EBinaryOp.Multiply;
            case ETokenType.OpSubtractAssign:
                return EBinaryOp.Subtract;
            default:
                throw new Error("Unexpected token type");
        }
    }

    constructor(left: Node, right: Node, op: EBinaryOp, debug: TokenDebugInfo) {
        super(ENodeType.BinaryOpAndAssign, debug);
        this.left = left;
        this.right = right;
        this.op = op;
    }

    getChildren(): Node[] {
        return [this.left, this.right];
    }

    static FromTokenType(left: Node, right: Node,type: ETokenType,debug: TokenDebugInfo) {
        return new BinaryOpAndAssignNode(left,right,BinaryOpAndAssignNode.TokenTypeToBinaryOpAssign(type),debug);
    }
}

export enum ELayoutType {
    In,
    Out,
    Uniform,
    ReadOnly
}

export class LayoutNode extends Node {
    declaration: DeclarationNode;
    layoutType: ELayoutType;
    tags: { [key: string]: string };

    constructor(tags: { [key: string]: string }, layoutType: ELayoutType, declaration: DeclarationNode, debug: TokenDebugInfo) {
        super(ENodeType.Layout, debug);
        this.tags = tags;
        this.layoutType = layoutType;
        this.declaration = declaration;
    }

    getChildren(): Node[] {
        return [this.declaration];
    }
}

export class CallNode extends Node {
    args: Node[];
    identifier: string;

    constructor(identifier: string, args: Node[], debug: TokenDebugInfo) {
        super(ENodeType.Call, debug);
        this.identifier = identifier;
        this.args = args;
    }

    getChildren(): Node[] {
        return this.args;
    }
}

export class AccessNode extends Node {
    left: Node;
    right: Node;

    constructor(left: Node, right: Node, debug: TokenDebugInfo) {
        super(ENodeType.Access, debug);
        this.left = left;
        this.right = right;
    }

    getChildren(): Node[] {
        return [this.left, this.right];
    }
}

export class IndexNode extends Node {
    left: Node;
    indexExpression: Node;

    constructor(left: Node, indexExpression: Node, debug: TokenDebugInfo) {
        super(ENodeType.Index, debug);
        this.left = left;
        this.indexExpression = indexExpression;
    }

    getChildren(): Node[] {
        return [this.left, this.indexExpression];
    }
}

export class ScopeNode extends Node {
    statements: Node[];

    constructor(statements: Node[], debug: TokenDebugInfo) {
        super(ENodeType.Scope, debug);
        this.statements = statements;
    }

    getChildren(): Node[] {
        return this.statements;
    }
}

export class DeclarationScopeNode extends Node {
    declarations: DeclarationNode[];

    constructor(declarations: DeclarationNode[], debug: TokenDebugInfo) {
        super(ENodeType.DeclarationScope, debug);
        this.declarations = declarations;
    }

    getChildren(): Node[] {
        return this.declarations;
    }

    getSize() {
        return this.declarations.reduce((total, decl) => total + decl.sizeOf(), 0)
    }
}

export enum EScopeType {
    Vertex,
    Fragment
}

export class NamedScopeNode extends ScopeNode {
    scopeType: EScopeType;

    constructor(scopeType: EScopeType, statements: Node[], debug: TokenDebugInfo) {
        super(statements, debug);
        this.nodeType = ENodeType.NamedScope;
        this.scopeType = scopeType;
    }
}

export class TypeNode extends Node {
    id: string;

    constructor(id: string, debug: TokenDebugInfo) {
        super(ENodeType.Type, debug);
        this.id = id;
    }

    getChildren(): Node[] {
        return [];
    }
}

export class IdentifierNode extends Node {
    id: string;

    constructor(id: string, debug: TokenDebugInfo) {
        super(ENodeType.Identifier, debug);
        this.id = id;
    }

    getChildren(): Node[] {
        return [];
    }
}

export class StructNode extends Node {
    scope: DeclarationScopeNode;
    name: string;

    constructor(name: string, scope: DeclarationScopeNode, debug: TokenDebugInfo) {
        super(ENodeType.Struct, debug);
        this.name = name;
        this.scope = scope;
    }

    getChildren(): Node[] {
        return [this.scope];
    }

    getSize(){
        return this.scope.getSize();
    }
}

export enum EDeclarationType {
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
    Void,
    Sampler2D
}


export class DeclarationNode extends Node {
    readonly declarationCount: number;
    readonly declarationType: EDeclarationType;
    declarationName: string;

    static tokenTypeToDeclarationType(tokenType: ETokenType){
        switch (tokenType) {
            case ETokenType.TypeFloat:
                return EDeclarationType.Float;
            case ETokenType.TypeFloat2:
                return EDeclarationType.Float2;
            case ETokenType.TypeFloat3:
                return EDeclarationType.Float3;
            case ETokenType.TypeFloat4:
                return EDeclarationType.Float4;
            case ETokenType.TypeInt:
                return EDeclarationType.Int;
            case ETokenType.TypeInt2:
                return EDeclarationType.Int2;
            case ETokenType.TypeInt3:
                return EDeclarationType.Int3;
            case ETokenType.TypeInt4:
                return EDeclarationType.Int4;
            case ETokenType.TypeMat3:
                return EDeclarationType.Mat3;
            case ETokenType.TypeMat4:
                return EDeclarationType.Mat4;
            case ETokenType.TypeVoid:
                return EDeclarationType.Void;
            case ETokenType.TypeSampler2D:
                return EDeclarationType.Sampler2D;
            default:
                throw new Error("Unknown Declaration Type");
        }
    }

    constructor(declarationType: EDeclarationType, name: string, count: number, debug: TokenDebugInfo) {
        super(ENodeType.Declaration, debug);
        this.declarationType = declarationType;
        this.declarationName = name;
        this.declarationCount = count;
    }

    sizeOf(): number {
        switch (this.declarationType) {
            case EDeclarationType.Struct:
                throw new Error("Node with type 'Struct' must use class 'StructDeclarationNode'");
            case EDeclarationType.Float:
            case EDeclarationType.Int:
                return 4 * this.declarationCount;
            case EDeclarationType.Float2:
            case EDeclarationType.Int2:
                return 8 * this.declarationCount;
            case EDeclarationType.Float3:
            case EDeclarationType.Int3:
                return 12 * this.declarationCount;
            case EDeclarationType.Float4:
            case EDeclarationType.Int4:
                return 16 * this.declarationCount;
            case EDeclarationType.Mat3:
                return 12 * 3 * this.declarationCount;
            case EDeclarationType.Mat4:
                return 16 * 4 * this.declarationCount;
            default:
                throw new Error("ArgumentOutOfRangeException");
        }
    }

    getTypeName(): string {
        switch (this.declarationType) {
            case EDeclarationType.Float:
                return "float";
            case EDeclarationType.Int:
                return "int";
            case EDeclarationType.Float2:
                return "float2";
            case EDeclarationType.Int2:
                return "int2";
            case EDeclarationType.Float3:
                return "float3";
            case EDeclarationType.Int3:
                return "int3";
            case EDeclarationType.Float4:
                return "float4";
            case EDeclarationType.Int4:
                return "int4";
            case EDeclarationType.Mat3:
                return "mat3";
            case EDeclarationType.Mat4:
                return "mat4";
            case EDeclarationType.Void:
                return "void";
            case EDeclarationType.Sampler2D:
                return "sampler2D";
            case EDeclarationType.Buffer:
                return "buffer";
            default:
                throw new Error("ArgumentOutOfRangeException");
        }
    }

    getChildren(): Node[] {
        return [];
    }
}

export class IncludeNode extends Node {
    file: IdentifierNode;
    sourceFile: string;

    constructor(sourceFile: string, file: IdentifierNode, debug: TokenDebugInfo) {
        super(ENodeType.Include, debug);
        this.sourceFile = sourceFile;
        this.file = file;
    }

    getChildren(): Node[] {
        return [];
    }
}

export class FloatLiteral extends Node {
    value: number;

    constructor(value: number, debug: TokenDebugInfo) {
        super(ENodeType.FloatLiteral, debug);
        this.value = value;
    }

    getChildren(): Node[] {
        return [];
    }
}

export class IntLiteral extends Node {
    value: number;

    constructor(value: number, debug: TokenDebugInfo) {
        super(ENodeType.IntLiteral, debug);
        this.value = value;
    }

    getChildren(): Node[] {
        return [];
    }
}

export class ConstNode extends Node {
    declaration: DeclarationNode;

    constructor(declaration: DeclarationNode, debug: TokenDebugInfo) {
        super(ENodeType.Const, debug);
        this.declaration = declaration;
    }

    getChildren(): Node[] {
        return [this.declaration];
    }
}

export class ArrayLiteralNode extends Node {
    expressions: Node[];

    constructor(expressions: Node[], debug: TokenDebugInfo) {
        super(ENodeType.ArrayLiteral, debug);
        this.expressions = expressions;
    }

    getChildren(): Node[] {
        return this.expressions;
    }
}


export class NegateNode extends Node {
    expression: Node;

    constructor(expression: Node, debug: TokenDebugInfo) {
        super(ENodeType.Negate, debug);
        this.expression = expression;
    }

    getChildren(): Node[] {
        return [this.expression];
    }
}


export class PrecedenceNode extends Node {
    expression: Node;

    constructor(expression: Node, debug: TokenDebugInfo) {
        super(ENodeType.Precedence, debug);
        this.expression = expression;
    }

    getChildren(): Node[] {
        return [this.expression];
    }
}


export class PushConstantNode extends Node {
    data: StructNode;
    name: string;
    tags: { [key: string]: string };

    constructor(name: string, data: StructNode, tags: { [key: string]: string }, debug: TokenDebugInfo) {
        super(ENodeType.PushConstant, debug);
        this.name = name;
        this.data = data;
        this.tags = tags;
    }

    getChildren(): Node[] {
        return [this.data];
    }
}


export class ForNode extends Node {
    initial: Node;
    condition: Node;
    update: Node;
    scope: ScopeNode;

    constructor(initial: Node, condition: Node, update: Node, scope: ScopeNode, debug: TokenDebugInfo) {
        super(ENodeType.For, debug);
        this.initial = initial;
        this.condition = condition;
        this.update = update;
        this.scope = scope;
    }

    getChildren(): Node[] {
        return [this.initial, this.condition, this.update, this.scope];
    }
}


export class IncrementNode extends Node {
    target: Node;
    isPre: boolean;

    constructor(target: Node, isPre: boolean, debug: TokenDebugInfo) {
        super(ENodeType.Increment, debug);
        this.target = target;
        this.isPre = isPre;
    }

    getChildren(): Node[] {
        return [this.target];
    }
}


export class DecrementNode extends Node {
    target: Node;
    isPre: boolean;

    constructor(target: Node, isPre: boolean, debug: TokenDebugInfo) {
        super(ENodeType.Decrement, debug);
        this.target = target;
        this.isPre = isPre;
    }

    getChildren(): Node[] {
        return [this.target];
    }
}


export class DiscardNode extends Node {
    constructor(debug: TokenDebugInfo) {
        super(ENodeType.Discard, debug);
    }

    getChildren(): Node[] {
        return [];
    }
}


export class IfNode extends Node {
    condition: Node;
    scope: ScopeNode;
    elseNode: Node;

    constructor(condition: Node, scope: ScopeNode, elseNode: Node, debug: TokenDebugInfo) {
        super(ENodeType.If, debug);
        this.condition = condition;
        this.scope = scope;
        this.elseNode = elseNode;
    }

    getChildren(): Node[] {
        return [this.condition, this.scope, this.elseNode];
    }
}


export class ConditionalNode extends Node {
    condition: Node;
    right: Node;
    left: Node;

    constructor(condition: Node, left: Node, right: Node, debug: TokenDebugInfo) {
        super(ENodeType.Conditional, debug);
        this.condition = condition;
        this.left = left;
        this.right = right;
    }

    getChildren(): Node[] {
        return [this.condition, this.left, this.right];
    }
}


export class DefineNode extends Node {
    identifier: string;
    expression: Node;

    constructor(identifier: string, expression: Node, debug: TokenDebugInfo) {
        super(ENodeType.Define, debug);
        this.identifier = identifier;
        this.expression = expression;
    }

    getChildren(): Node[] {
        return [this.expression];
    }
}

export class BufferDeclarationNode extends DeclarationNode {
    scope: DeclarationScopeNode;

    constructor(name: string, count: number, scope: DeclarationScopeNode, debug: TokenDebugInfo) {
        super(EDeclarationType.Buffer, name, count, debug);
        this.scope = scope;
    }

    sizeOf(): number {
        return this.scope.getSize();
    }

    getChildren(): Node[] {
        return [this.scope];
    }

    getTypeName(): string {
        return "";
    }
}

export class BlockDeclarationNode extends DeclarationNode {
    scope: DeclarationScopeNode;

    constructor( name: string, count: number, scope: DeclarationScopeNode, debug: TokenDebugInfo) {
        super(EDeclarationType.Block, name, count, debug);
        this.scope = scope;
    }

    sizeOf(): number {
        return this.scope.declarations.reduce((total, decl) => total + decl.sizeOf(), 0);
    }

    getChildren(): Node[] {
        return [this.scope];
    }

    getTypeName(): string {
        return `block_${this.declarationName}`;
    }
}

export class StructDeclarationNode extends DeclarationNode {
    struct?: StructNode;
    structName: string;

    constructor(structName: string,name: string,count: number, debug: TokenDebugInfo);
    constructor(struct: StructNode, name: string, count: number, debug: TokenDebugInfo);
    constructor(structOrStructName: string | StructNode,name: string, count: number,debug: TokenDebugInfo) {
        if (typeof structOrStructName === 'string') {
            super(EDeclarationType.Struct,name, count, debug);
            this.structName = structOrStructName;
        } else {
            super(EDeclarationType.Struct, name, count, debug);
            this.struct = structOrStructName;
            this.structName = structOrStructName.name;
        }
    }

    sizeOf(): number {
        if (!this.struct) return 8;
        return this.struct.getSize();
    }

    getChildren(): Node[] {
        return this.struct ? [this.struct] : [];
    }

    getTypeName(): string {
        return this.structName;
    }
}