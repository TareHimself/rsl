// See https://aka.ms/new-console-template for more information

using ashl.Generator;
using ashl.Parser;
using ashl.Tokenizer;

/*
Console.WriteLine("Hello, World!");
var basePath = @"D:\Github\ashl";
var tokenizer = new Tokenizer();
var tokens = tokenizer.Run($@"D:\Github\vengine\aerox.Runtime\shaders\2d\blur.ash");
var parser = new Parser();
var ast = parser.Run(tokens);

ast.ResolveIncludes((node, module) =>
{
    if (Path.IsPathRooted(node.File)) return Path.GetFullPath(node.File);

    return Path.GetFullPath(node.File,
        Directory.GetParent(node.SourceFile)?.FullName ?? Directory.GetCurrentDirectory());
}, tokenizer, parser);

ast.ResolveStructReferences();
var generator = new Generator();

var data = generator.Run(ast, EScopeType.Vertex);
File.WriteAllText(@$"{basePath}\rect.vert", "#version 450\n#extension GL_GOOGLE_include_directive : require\n#extension GL_EXT_buffer_reference : require\n#extension GL_EXT_scalar_block_layout : require\n" + generator.Run(ast, EScopeType.Vertex));
File.WriteAllText($@"{basePath}\rect.frag", "#version 450\n#extension GL_GOOGLE_include_directive : require\n#extension GL_EXT_buffer_reference : require\n#extension GL_EXT_scalar_block_layout : require\n" + generator.Run(ast, EScopeType.Fragment));
*/


var tokenizer = new Tokenizer();
var tokens = tokenizer.Run(@"D:\Github\aerox\aerox.Runtime.Scene\shaders\scene\mesh_vert.ash");
var parser = new Parser();
var ast = parser.Run(tokens);
ast = ast.ResolveIncludes((node, module) =>
{
    if (Path.IsPathRooted(node.File)) return Path.GetFullPath(node.File);

    return Path.GetFullPath(node.File,
        Directory.GetParent(node.SourceFile)?.FullName ?? Directory.GetCurrentDirectory());
}, tokenizer, parser).ExtractScope(EScopeType.Vertex);
ast.ResolveStructReferences();
var optimzed = ast.ExtractFunctionWithDependencies("main");
var x = "";
// var tokenizer = new Tokenizer();
// var tokens = tokenizer.Run(@"D:\Github\ashl\utils.ash");
// var parser = new Parser();
// var ast = parser.Run(tokens);
// Console.WriteLine("Data {0}",Token.KeywordToTokenType("float4"));