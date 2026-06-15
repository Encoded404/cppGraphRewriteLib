module;

// ============================================================================
// Global Module Fragment (GMF)
// Non-modular #includes that must remain as text includes.
// ============================================================================

#ifndef GRAPHLIBRARY_USE_STD_MODULE
// Standard library headers — only needed when NOT using import std;
#include <string_view>
#include <string>
#include <vector>
#include <map>
#include <cstddef>
#include <algorithm>
#include <optional>
#include <functional>
#include <memory>
#include <chrono>
#endif

// Boost.Graph is not modularized — must be in GMF
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>

// Logging macros must remain as text includes (macros cannot be imported).
// This header has zero #include directives, safe in GMF alongside import std;
#include <logging/logging_macros.hpp>

// ============================================================================
// Module declaration
// ============================================================================
export module GraphLibrary;

#ifdef GRAPHLIBRARY_USE_STD_MODULE
import std;
#endif

// Get logging types via the module (not via #include)
import logiface;

// ============================================================================
// Module purview — include existing headers.
// Prevent GraphLibrary.hpp from re-including <logging/logging.hpp>
// since we already have those types via import logiface;
// ============================================================================
#define GRAPHLIBRARY_LOGGING_ALREADY_IMPORTED

#include <GraphLibrary/TypeDefs.hpp>
#include <GraphLibrary/SparseVector.hpp>
#include <GraphLibrary/GraphLibrary.hpp>
#include <GraphLibrary/Rule.hpp>
#include <GraphLibrary/Internal/ForwardDeclarations.hpp>
#include <GraphLibrary/Internal/Detail.hpp>
#include <GraphLibrary/Internal/GraphImpl.hpp>

// ============================================================================
// Re-export public API symbols so they are visible to consumers of
// import GraphLibrary;
//
// Templates and classes defined in module purview (via #include) have
// module-linkage. Without the export { using ...; } block below, they
// would NOT be visible to importing translation units.
// ============================================================================
export {
    using GraphLibrary::CmpOp;
    using GraphLibrary::SparseVector;
    using GraphLibrary::Graph;
    using GraphLibrary::Rule;
    using GraphLibrary::RuleSet;
}
