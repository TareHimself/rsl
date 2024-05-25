// See https://aka.ms/new-console-template for more information

using ashl.Generator;
using ashl.Parser;
using ashl.Tokenizer;

Console.WriteLine("Hello, World!");

var tokenizer = new Tokenizer();
var tokens = tokenizer.Run(@"C:\Users\Taree\Documents\Github\ashl\rect.ash");
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

ast.ResolveStructReferences();
var generator = new Generator();

var data = generator.Run(ast,EScopeType.Vertex);
File.WriteAllLines(@"C:\Users\Taree\Documents\Github\ashl\rect.vert",generator.Run(ast,EScopeType.Vertex).Select(c => c + "\n"));
File.WriteAllLines(@"C:\Users\Taree\Documents\Github\ashl\rect.frag",generator.Run(ast,EScopeType.Fragment).Select(c => c + "\n"));


Console.WriteLine("Data {0}",Token.KeywordToTokenType("float4"));

// var tokenizer = new Tokenizer();
// var tokens = tokenizer.Run(@"D:\Github\ashl\utils.ash");
// var parser = new Parser();
// var ast = parser.Run(tokens);
// Console.WriteLine("Data {0}",Token.KeywordToTokenType("float4"));