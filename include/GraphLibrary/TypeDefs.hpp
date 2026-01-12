#ifndef GRAPHLIBRARY_TYPEDEFS_HPP
#define GRAPHLIBRARY_TYPEDEFS_HPP


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

} // namespace GraphLibrary

#endif // GRAPHLIBRARY_TYPEDEFS_HPP