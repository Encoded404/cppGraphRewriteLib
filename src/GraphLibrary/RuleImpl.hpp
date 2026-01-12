#ifndef GRAPHLIBRARY_RULE_PRIVATE_HPP
#define GRAPHLIBRARY_RULE_PRIVATE_HPP

#include <GraphLibrary/TypeDefs.hpp>
#include <GraphLibrary/SparseVector.hpp>
#include <GraphLibrary/Rule.hpp>

#include <vector>
#include <unordered_map>
#include <functional>

namespace GraphLibrary
{
    template<typename NodeProperty, typename EdgeProperty>
    class Rule<NodeProperty, EdgeProperty>::Impl
    {
        public:
            using Constraint = std::function<bool(const Context&)>;
        private:
            Graph<NodeProperty, EdgeProperty> LGraph;
            Graph<NodeProperty, EdgeProperty> RGraph;

            SparseVector<Constraint> constraints;

            std::unordered_map<Vertex, Vertex> nodeMapping; // Maps nodes from LGraph to RGraph
            std::unordered_map<Edge, Edge> edgeMapping; // Maps edges from LGraph to RGraph
        public:
            const Graph<NodeProperty, EdgeProperty>& getLGraph() const { return LGraph; }
            const Graph<NodeProperty, EdgeProperty>& getRGraph() const { return RGraph; }

            int AddConstraint(const Constraint& constraint)
            {
                return constraints.Add(constraint);
            }
            void RemoveConstraint(int index)
            {
                constraints.Remove(index);
            }

            void LinkNodes(Vertex LNode, Vertex RNode)
            {
                // Implementation for linking nodes between LGraph and RGraph
            }
    };
} // namespace GraphLibrary

#endif // GRAPHLIBRARY_RULE_PRIVATE_HPP
