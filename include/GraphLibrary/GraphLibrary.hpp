#ifndef GRAPHLIBRARY_GRAPH_HPP
#define GRAPHLIBRARY_GRAPH_HPP

#include <GraphLibrary/TypeDefs.hpp>

#ifndef GRAPHLIBRARY_LOGGING_ALREADY_IMPORTED
#include <logging/logging.hpp>
#endif

// Forward declarations to avoid circular include with Rule.hpp
namespace GraphLibrary {
    template<typename NodeProperty, typename EdgeProperty>
    class Rule;
    template<typename NodeProperty, typename EdgeProperty>
    class RuleSet;
    namespace Detail { template<typename NodeProperty, typename EdgeProperty> struct MatchResult; }
}

#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <vector>

namespace GraphLibrary
{
    template<
        typename NodeProperty,
        typename EdgeProperty
    >
    class Graph
    {
    public:
        using BoostGraphType = boost::adjacency_list<
            boost::listS,
            boost::listS,
            boost::bidirectionalS,
                NodeProperty,
                EdgeProperty
            >;
        using BoostVertexDescriptor = typename boost::graph_traits<BoostGraphType>::vertex_descriptor;
        using BoostEdgeDescriptor = typename boost::graph_traits<BoostGraphType>::edge_descriptor;

        // Expose descriptor aliases for users
        using Vertex = BoostVertexDescriptor;
        using Edge = BoostEdgeDescriptor;

        // Allow Rule to access internals where needed
        friend class Rule<NodeProperty, EdgeProperty>;

        const BoostGraphType& GetInternalGraph() const { return boostGraph_; }
        BoostGraphType& GetInternalGraph() { return boostGraph_; }

    private:
        boost::adjacency_list<
            boost::listS,
            boost::listS,
            boost::bidirectionalS,
                NodeProperty,
                EdgeProperty
            > boostGraph_;

        boost::adjacency_list<
            boost::listS,
            boost::listS,
            boost::bidirectionalS,
                NodeProperty,
                EdgeProperty
            >& ApplyRuleInternal(const Rule<NodeProperty, EdgeProperty>& rule,
            boost::adjacency_list<
            boost::listS,
            boost::listS,
            boost::bidirectionalS,
                NodeProperty,
                EdgeProperty
            >& graph) const;

        static std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>> CollectMatches(
            const Rule<NodeProperty, EdgeProperty>& rule,
            const BoostGraphType& target_graph);

    public:
        Vertex AddNode(const NodeProperty& property)
        {
            BoostVertexDescriptor desc = boost::add_vertex(property, boostGraph_);
            LOGIFACE_LOG(trace, "AddNode(prop): total vertices = " + std::to_string(boost::num_vertices(boostGraph_)));
            return desc;
        }
        Vertex AddNode()
        {
            BoostVertexDescriptor desc = boost::add_vertex(boostGraph_);
            LOGIFACE_LOG(trace, "AddNode(): total vertices = " + std::to_string(boost::num_vertices(boostGraph_)));
            return desc;
        }

        Edge AddEdge(
            Vertex u,
            Vertex v,
            const EdgeProperty& property)
        {
            auto e_pair = boost::add_edge(u, v, property, boostGraph_);
            LOGIFACE_LOG(trace, "AddEdge(prop): added=" + std::string(e_pair.second ? "true" : "false") + ", total edges = " + std::to_string(boost::num_edges(boostGraph_)));
            return e_pair.first; // first is the edge descriptor second is a bool indicating if it was added or already existed
        }
        Edge AddEdge(
            Vertex u,
            Vertex v)
        {
            auto e_pair = boost::add_edge(u, v, boostGraph_);
            LOGIFACE_LOG(trace, "AddEdge(): added=" + std::string(e_pair.second ? "true" : "false") + ", total edges = " + std::to_string(boost::num_edges(boostGraph_)));
            return e_pair.first; // first is the edge descriptor second is a bool indicating if it was added or already existed
        }

        /// @brief Remove an edge by descriptor from the underlying boost graph.
        void RemoveEdge(Edge e)
        {
            LOGIFACE_LOG(trace, "RemoveEdge: total edges before = " + std::to_string(boost::num_edges(boostGraph_)));
            boost::remove_edge(e, boostGraph_);
        }

        /// @brief Remove a vertex (and all incident edges) by descriptor from the underlying boost graph.
        void RemoveNode(Vertex v)
        {
            LOGIFACE_LOG(trace, "RemoveNode: total vertices before = " + std::to_string(boost::num_vertices(boostGraph_)));
            boost::clear_vertex(v, boostGraph_);
            boost::remove_vertex(v, boostGraph_);
        }

        /// @brief Returns all vertices in the underlying boost graph as descriptors.
        std::vector<Vertex> GetVertices() const
        {
            std::vector<Vertex> out;
            for (auto v : boost::make_iterator_range(vertices(boostGraph_)))
            {
                out.push_back(v);
            }
            return out;
        }

        /// @brief Returns all edges in the underlying boost graph as descriptors.
        std::vector<Edge> GetEdges() const
        {
            std::vector<Edge> out;
            for (auto e : boost::make_iterator_range(edges(boostGraph_)))
            {
                out.push_back(e);
            }
            return out;
        }

        // property accessors
        template <typename T>
        T& GetNodeProperty(Vertex v, T NodeProperty::*member) {
            return boostGraph_[v].*member;
        }
        
        template <typename T>
        T& GetEdgeProperty(Edge e, T EdgeProperty::*member) {
            return boostGraph_[e].*member;
        }

        Graph<NodeProperty, EdgeProperty> ApplyRule(const Rule<NodeProperty, EdgeProperty>& rule) const;
        Graph<NodeProperty, EdgeProperty> ApplyRuleSet(const RuleSet<NodeProperty, EdgeProperty>& rule_set) const;
        Graph<NodeProperty, EdgeProperty> ApplyRuleSets(const std::vector<RuleSet<NodeProperty, EdgeProperty>>& rule_sets) const;
    };
} // namespace GraphLibrary

#include "Internal/GraphImpl.hpp"

#endif // GRAPHLIBRARY_GRAPH_HPP