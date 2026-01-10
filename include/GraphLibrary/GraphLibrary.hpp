#ifndef GRAPHLIBRARY_HPP
#define GRAPHLIBRARY_HPP

namespace GraphLibrary
{
    
    enum class CmpOp { EQ, NE, GT, GE, LT, LE };

    // Opaque handle types, so users don't see the Boost types and have to include Boost headers
    class Vertex
    {
    private:
        void* handle;
        Vertex(void* h) : handle(h) {}
        friend class Graph;
        template<typename, typename> friend class Graph;
    public:
        Vertex() : handle(nullptr) {}
    };

    class Edge
    {
    private:
        void* handle;
        Edge(void* h) : handle(h) {}
        friend class Graph;
        template<typename, typename> friend class Graph;
    public:
        Edge() : handle(nullptr) {}
    };

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
    };
} // namespace GraphLibrary

#endif // GRAPHLIBRARY_HPP