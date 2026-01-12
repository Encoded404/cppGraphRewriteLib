#ifndef GRAPHLIBRARY_HPP
#define GRAPHLIBRARY_HPP

#include "TypeDefs.hpp"

namespace GraphLibrary
{
    template<typename NodeProperty, typename EdgeProperty>
    class Graph
    {
    public:
        Vertex addNode(const NodeProperty& property);
        Vertex addNode();
        void addEdge(Vertex u, Vertex v, const EdgeProperty& property);
        void addEdge(Vertex u, Vertex v);
        
        template <typename T>
        T& get_node_property(Vertex v, T NodeProperty::*member);
        template <typename T>
        T& get_edge_property(Edge e, T EdgeProperty::*member);
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;
    };
} // namespace GraphLibrary

#endif // GRAPHLIBRARY_HPP