#ifndef GRAPHLIBRARY_GRAPH_HPP
#define GRAPHLIBRARY_GRAPH_HPP

#include <GraphLibrary/GraphLibrary.hpp>

#include <boost/graph/adjacency_list.hpp>

namespace GraphLibrary
{
    template<
        typename NodeProperty,
        typename EdgeProperty
    >
    class Graph<NodeProperty, EdgeProperty>::Impl
    {
    public: using BoostGraphType = boost::adjacency_list<
                boost::listS,
                boost::listS,
                boost::bidirectionalS,
                NodeProperty,
                EdgeProperty
            >;

    private:
        
        BoostGraphType boostGraph;
        
    public:
        using BoostVertexDescriptor = typename boost::graph_traits<BoostGraphType>::vertex_descriptor;
        using BoostEdgeDescriptor = typename boost::graph_traits<BoostGraphType>::edge_descriptor;
        
        Vertex addNode(const NodeProperty& property)
        {
            BoostVertexDescriptor desc = boost::add_vertex(property, boostGraph);
            return Vertex(static_cast<void*>(desc));
        }
        Vertex addNode()
        {
            BoostVertexDescriptor desc = boost::add_vertex(boostGraph);
            return Vertex(static_cast<void*>(desc));
        }

        Edge addEdge(
            Vertex u,
            Vertex v,
            const EdgeProperty& property)
        {
            BoostVertexDescriptor desc_u = static_cast<BoostVertexDescriptor>(u.handle);
            BoostVertexDescriptor desc_v = static_cast<BoostVertexDescriptor>(v.handle);
            auto e_pair = boost::add_edge(desc_u, desc_v, property, boostGraph);
            return Edge(static_cast<void*>(e_pair.first)); // first is the edge descriptor second is a bool indicating if it was added or already existed
        }
        Edge addEdge(
            Vertex u,
            Vertex v)
        {
            BoostVertexDescriptor desc_u = static_cast<BoostVertexDescriptor>(u.handle);
            BoostVertexDescriptor desc_v = static_cast<BoostVertexDescriptor>(v.handle);
            auto e_pair = boost::add_edge(desc_u, desc_v, boostGraph);
            return Edge(static_cast<void*>(e_pair.first)); // first is the edge descriptor second is a bool indicating if it was added or already existed
        }

        // property accessors
        template <typename T>
        T& get_node_property(Vertex v, T NodeProperty::*member) {
            return boostGraph[v].*member;
        }
        template <typename T>
        T& get_edge_property(Edge e, T EdgeProperty::*member) {
            return boostGraph[e].*member;
        }
        // Public member functions to manipulate the graph can be added here
    };
} // namespace GraphLibrary

#endif // GRAPHLIBRARY_GRAPH_HPP