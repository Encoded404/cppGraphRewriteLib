#ifndef GRAPHLIBRARY_RULE_HPP
#define GRAPHLIBRARY_RULE_HPP

#include "TypeDefs.hpp"
#include "GraphLibrary.hpp"
#include <functional>
#include <memory>

namespace GraphLibrary
{
    template<typename NodeProperty, typename EdgeProperty>
    class Rule
    {
    public:
        using Constraint = std::function<bool(const Context&)>;

        Rule();
        ~Rule();

        const Graph<NodeProperty, EdgeProperty>& getLGraph() const;
        const Graph<NodeProperty, EdgeProperty>& getRGraph() const;

        int AddConstraint(const Constraint& constraint);
        void RemoveConstraint(int index);

        void LinkNodes(Vertex LNode, Vertex RNode);

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };

    template<typename NodeProperty, typename EdgeProperty>
    Rule<NodeProperty, EdgeProperty>::Rule()
        : impl(std::make_unique<typename Rule<NodeProperty, EdgeProperty>::Impl>())
    {
    }

    template<typename NodeProperty, typename EdgeProperty>
    Rule<NodeProperty, EdgeProperty>::~Rule() = default;

    template<typename NodeProperty, typename EdgeProperty>
    const Graph<NodeProperty, EdgeProperty>&
    Rule<NodeProperty, EdgeProperty>::getLGraph() const
    {
        return impl->getLGraph();
    }

    template<typename NodeProperty, typename EdgeProperty>
    const Graph<NodeProperty, EdgeProperty>&
    Rule<NodeProperty, EdgeProperty>::getRGraph() const
    {
        return impl->getRGraph();
    }

    template<typename NodeProperty, typename EdgeProperty>
    int Rule<NodeProperty, EdgeProperty>::AddConstraint(const Constraint& constraint)
    {
        return impl->AddConstraint(constraint);
    }

    template<typename NodeProperty, typename EdgeProperty>
    void Rule<NodeProperty, EdgeProperty>::RemoveConstraint(int index)
    {
        impl->RemoveConstraint(index);
    }

    template<typename NodeProperty, typename EdgeProperty>
    void Rule<NodeProperty, EdgeProperty>::LinkNodes(Vertex LNode, Vertex RNode)
    {
        impl->LinkNodes(LNode, RNode);
    }

} // namespace GraphLibrary

#endif // GRAPHLIBRARY_RULE_HPP