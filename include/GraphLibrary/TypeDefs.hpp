#ifndef GRAPHLIBRARY_TYPEDEFS_HPP
#define GRAPHLIBRARY_TYPEDEFS_HPP


namespace GraphLibrary
{
    enum class CmpOp { EQ, NE, GT, GE, LT, LE };
    // Previously this header contained opaque handle types for Vertex/Edge.
    // The library now exposes Boost descriptors directly via the Graph template.

} // namespace GraphLibrary

#endif // GRAPHLIBRARY_TYPEDEFS_HPP