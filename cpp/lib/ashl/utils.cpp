#include "ashl/utils.hpp"
#include "ashl/tokenizer.hpp"
#include <filesystem>
#include <queue>

#include "ashl/parser.hpp"

namespace ashl
{
    std::vector<std::string> split(const std::string& data,const std::string& delimiter)
    {
        std::vector<std::string> result{};
        std::string remaining = data;

        if(delimiter.empty())
        {
            result.reserve(remaining.size());
            for (const auto& value : remaining)
            {
                result.push_back(std::string{value});
            }
        }

        size_t pos = 0;
        while((pos = remaining.find(delimiter)) != std::string::npos)
        {
            result.push_back(remaining.substr(0,pos));
            remaining.erase(0,pos + delimiter.length());
        }

        return result;
    }

    bool isNumeric(const char& data)
    {
        return data >= 48 && data <= 57;
    }

    bool isInteger(const std::string& data)
    {
        std::string test = data;
        if(test.starts_with('-'))
        {
            test = test.substr(1,test.size() - 1);
        }
        
        for(const auto &c : test)
        {
            if(!isNumeric(c))
            {
                return false;
            }
        }

        return true;
    }

    bool isBoolean(const std::string& data)
    {
        return data == "true" || data == "false";
    }

    bool isFloat(const std::string& data)
    {
        auto parts = split(data,".");
        if(parts.empty() || parts.size() > 2) return false;

        for (const auto& part : parts)
        {
            if(part.empty()) continue;
            if(!isInteger(part)) return false;
        }

        return true;
    }

    int parseInt(const std::string& data)
    {
        return std::stoi(data);
    }

    bool parseBoolean(const std::string& data)
    {
        return data == "true";
    }

    float parseFloat(const std::string& data)
    {
        return std::stof(data);
    }

    void resolveIncludes(const std::shared_ptr<NamedScopeNode>& node,std::set<std::string>& included)
    {
        std::vector<std::shared_ptr<Node>> pendingStatements = node->scope->statements;
        std::vector<std::shared_ptr<Node>> statements{};
        statements.reserve(pendingStatements.size());
        for (size_t i = 0; i < pendingStatements.size(); i++)
        {
            auto statement = pendingStatements[i];
            if(pendingStatements[i]->nodeType == ENodeType::Include)
            {
                if(auto asInclude = std::dynamic_pointer_cast<IncludeNode>(pendingStatements[i]))
                {
                    auto filePath = std::filesystem::exists(std::filesystem::absolute(asInclude->targetFile)) ? std::filesystem::absolute(asInclude->targetFile) : std::filesystem::relative(asInclude->targetFile,asInclude->sourceFile);

                    auto filePathAsStr = filePath.string();

                    if(included.contains(filePathAsStr)) continue;

                    included.emplace(filePathAsStr);

                    auto tokens = tokenize(filePathAsStr);

                    auto ast = parse(tokens);

                    pendingStatements.insert(pendingStatements.begin() + i,ast->statements.begin(),ast->statements.end());
                    i--;
                    continue;
                }
            }

            statements.push_back(pendingStatements[i]);
        }

        node->scope->statements = statements;
    }

    void resolveIncludes(const std::shared_ptr<NamedScopeNode>& node)
    {
        std::set<std::string> includes{};
        resolveIncludes(node,includes);
    }

    void resolveIncludes(const std::shared_ptr<ModuleNode>& node,std::set<std::string>& included)
    {
        std::vector<std::shared_ptr<Node>> pendingStatements = node->statements;
        std::vector<std::shared_ptr<Node>> statements{};
        statements.reserve(pendingStatements.size());
        for (size_t i = 0; i < pendingStatements.size(); i++)
        {
            auto statement = pendingStatements[i];
            if(pendingStatements[i]->nodeType == ENodeType::Include)
            {
                if(auto asInclude = std::dynamic_pointer_cast<IncludeNode>(pendingStatements[i]))
                {
                    std::filesystem::path sourcePath = std::filesystem::absolute(asInclude->sourceFile);
                    auto filePath = std::filesystem::exists(std::filesystem::absolute(asInclude->targetFile)) ? std::filesystem::absolute(asInclude->targetFile) : sourcePath.parent_path() / asInclude->targetFile;

                    auto filePathAsStr = filePath.string();

                    if(included.contains(filePathAsStr)) continue;

                    included.emplace(filePathAsStr);

                    auto tokens = tokenize(filePathAsStr);

                    auto ast = parse(tokens);

                    pendingStatements.insert(pendingStatements.begin() + i,ast->statements.begin(),ast->statements.end());
                    i--;
                    continue;
                }
            }

            if(pendingStatements[i]->nodeType == ENodeType::NamedScope)
            {
                if(auto asNamedScope = std::dynamic_pointer_cast<NamedScopeNode>(pendingStatements[i]))
                {
                    resolveIncludes(asNamedScope,included);
                }
            }

            statements.push_back(pendingStatements[i]);
        }

        node->statements = statements;
    }

    void resolveIncludes(const std::shared_ptr<ModuleNode>& node)
    {
        std::set<std::string> includes{};
        resolveIncludes(node,includes);
    }

    void walk(const std::shared_ptr<Node>& start, const std::function<bool(const std::shared_ptr<Node>&)>& callback)
    {
        std::queue<std::shared_ptr<Node>> nodes{};
        nodes.push(start);
        while(!nodes.empty())
        {
            if(callback(nodes.front()))
            {
                for (auto &child : nodes.front()->GetChildren())
                {
                    nodes.push(child);
                } 
            }
            nodes.pop();
        }
    }
    

    void resolveReferences(const std::shared_ptr<ModuleNode>& node)
    {

        std::map<std::string,std::shared_ptr<StructNode>> structs{};
        walk(node,[&structs](const std::shared_ptr<Node>& node)
        {
            if(node->nodeType == ENodeType::Struct)
            {
                if(auto asStruct = std::dynamic_pointer_cast<StructNode>(node))
                {
                    structs.emplace(asStruct->name,asStruct);
                }
            }

            if(node->nodeType == ENodeType::Declaration)
            {
                if(auto asStructDeclaration = std::dynamic_pointer_cast<StructDeclarationNode>(node))
                {
                    if(structs.contains(asStructDeclaration->structName))
                    {
                        asStructDeclaration->structNode = structs[asStructDeclaration->structName];
                    }
                }
            }
            
            return true;
        });
    }

    std::shared_ptr<ModuleNode> extractScope(const std::shared_ptr<ModuleNode>& node, const EScopeType& scopeType)
    {
        std::vector<std::shared_ptr<Node>> statements{};
        
        statements.reserve(node->statements.size());

        for (auto &statement : node->statements)
        {
            if(statement->nodeType == ENodeType::NamedScope)
            {
                if(auto asNamedScope = std::dynamic_pointer_cast<NamedScopeNode>(statement))
                {
                    if(asNamedScope->scopeType == scopeType)
                    {
                        statements.insert(statements.end(),asNamedScope->scope->statements.begin(),asNamedScope->scope->statements.end());
                    }
                    continue;
                }
            }

            statements.push_back(statement);
        }
        
        return std::make_shared<ModuleNode>(statements);
    }
}
