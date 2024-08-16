#pragma once
#include <cstdint>
#include <memory>
#include <functional>
#include <set>
#include <string>

#include "nodes.hpp"

namespace ashl {

    template <typename T, typename... Rest>
    void _hashCombineHelper(size_t& seed, const T& v, const Rest&... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (_hashCombineHelper(seed, rest), ...);
    }

    template <typename... Rest>
    size_t hashCombine(const Rest&... rest)
    {
        size_t seed = 0;
        (_hashCombineHelper(seed,rest), ...);
        return seed;
    }


    std::vector<std::string> split(const std::string& data,const std::string& delimiter = "");

    bool isNumeric(const char& data);
    
    bool isInteger(const std::string& data);
    bool isBoolean(const std::string& data);
    bool isFloat(const std::string& data);

    int parseInt(const std::string& data);
    auto parseBoolean(const std::string& data) -> bool;
    float parseFloat(const std::string& data);

    template <typename T, typename... Args>
    std::set<T> setOf(T first, Args... args);

    template <typename T>
    std::set<T> setOf();

    template <typename T>
    std::set<T> setOf()
    {
        return std::set<T>{};
    }

    template <typename T, typename... Args>
    std::set<T> setOf(T first, Args... args) {
        return std::set<T>{first, args...};
    }


    template<typename E,typename T>
    std::vector<E> mapVector(const std::vector<T>& original,const std::function<E(const T&)>& transform);

    template <typename E, typename T>
    std::vector<E> mapVector(const std::vector<T>& original, const std::function<E(const T&)>& transform)
    {
        std::vector<E> result{};
        result.reserve(original.size());
        for (auto &data : original)
        {
            result.push_back(transform(data));
        }

        return result;
    }

    void resolveIncludes(const std::shared_ptr<NamedScopeNode>& node,std::set<std::string>& included);

    void resolveIncludes(const std::shared_ptr<NamedScopeNode>& node);
    
    void resolveIncludes(const std::shared_ptr<ModuleNode>& node,std::set<std::string>& included);
    
    void resolveIncludes(const std::shared_ptr<ModuleNode>& node);

    void walk(const std::shared_ptr<Node>& start,const std::function<bool(const std::shared_ptr<Node>&)>& callback);
    
    void resolveReferences(const std::shared_ptr<ModuleNode>& node);

    std::shared_ptr<ModuleNode> extractScope(const std::shared_ptr<ModuleNode>& node,const EScopeType &scopeType);
}
