// See https://aka.ms/new-console-template for more information

using ashl.Generator;
using ashl.Parser;
using ashl.Tokenizer;

Console.WriteLine("Hello, World!");

var tokenizer = new Tokenizer();
var tokens = tokenizer.Run(@"D:\Github\ashl\test.ash");
var parser = new Parser();
var ast = parser.Run(tokens);

ast.ResolveIncludes((node, module) =>
{
    if (Path.IsPathRooted(node.File))
    {
        return Path.GetFullPath(node.File);
    }

    return Path.GetFullPath(node.File, Directory.GetParent(node.SourceFile)?.FullName ?? Directory.GetCurrentDirectory());
},tokenizer,parser);

var generator = new Generator();

var data = generator.Run(ast,EScopeType.Vertex);
File.WriteAllLines(@"D:\Github\ashl\test.vert",data);
Console.WriteLine("Data {0}",Token.KeywordToTokenType("float4"));

// var tokenizer = new Tokenizer();
// var tokens = tokenizer.Run(@"D:\Github\ashl\utils.ash");
// var parser = new Parser();
// var ast = parser.Run(tokens);
// Console.WriteLine("Data {0}",Token.KeywordToTokenType("float4"));