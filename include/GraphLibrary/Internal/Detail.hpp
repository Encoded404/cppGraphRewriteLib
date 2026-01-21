#ifndef GRAPHLIBRARY_DETAIL_HPP
#define GRAPHLIBRARY_DETAIL_HPP

#include "ForwardDeclarations.hpp"

#include <boost/graph/vf2_sub_graph_iso.hpp>

#include <optional>
#include <algorithm>

namespace GraphLibrary::Detail
{
    template<typename NodeProperty, typename EdgeProperty>
    struct MatchResult
    {
        using Impl = typename ::GraphLibrary::Graph<NodeProperty, EdgeProperty>;
        using BoostVertexDescriptor = typename Impl::BoostVertexDescriptor;
        using BoostEdgeDescriptor = typename Impl::BoostEdgeDescriptor;

        struct VertexPair
        {
            BoostVertexDescriptor pattern;
            BoostVertexDescriptor host;
        };
        
        std::vector<VertexPair> vertices;
        std::vector<BoostEdgeDescriptor> edges;
        
        std::optional<BoostVertexDescriptor> HostForPattern(BoostVertexDescriptor descriptor) const;
        std::optional<BoostVertexDescriptor> PatternForHost(BoostVertexDescriptor descriptor) const;
        bool UsesHostEdge(BoostEdgeDescriptor descriptor) const;
    };

    /// @brief Get the host graph vertex corresponding to the given pattern graph vertex.
    /// @param descriptor The vertex descriptor in the pattern graph.
    /// @return The corresponding vertex descriptor in the host graph, or std::nullopt if not found.
    template<typename NodeProperty, typename EdgeProperty>
    std::optional<typename MatchResult<NodeProperty, EdgeProperty>::BoostVertexDescriptor>
    MatchResult<NodeProperty, EdgeProperty>::HostForPattern(MatchResult<NodeProperty, EdgeProperty>::BoostVertexDescriptor descriptor) const
    {
        for (const auto& entry : vertices)
        {
            if (entry.pattern == descriptor)
            {
                return entry.host;
            }
        }
        return std::nullopt;
    }

    /// @brief Get the pattern graph vertex corresponding to the given host graph vertex.
    /// @param descriptor The vertex descriptor in the host graph.
    /// @return The corresponding vertex descriptor in the pattern graph, or std::nullopt if not found.
    template<typename NodeProperty, typename EdgeProperty>
    std::optional<typename MatchResult<NodeProperty, EdgeProperty>::BoostVertexDescriptor>
    MatchResult<NodeProperty, EdgeProperty>::PatternForHost(MatchResult<NodeProperty, EdgeProperty>::BoostVertexDescriptor descriptor) const
    {
        for (const auto& entry : vertices)
        {
            if (entry.host == descriptor)
            {
                return entry.pattern;
            }
        }
        return std::nullopt;
    }

    template<typename NodeProperty, typename EdgeProperty>
    bool Detail::MatchResult<NodeProperty, EdgeProperty>::UsesHostEdge(MatchResult<NodeProperty, EdgeProperty>::BoostEdgeDescriptor descriptor) const
    {
        // any_of returns true if any of the elements between begin and end satisfy the predicate
        return std::any_of(edges.begin(), edges.end(), [descriptor](const BoostEdgeDescriptor& existing) {
            return existing == descriptor;
        });
    }

    template<typename NodeProperty, typename EdgeProperty>
    class MatchCollector
    {
    public:
        using Match = MatchResult<NodeProperty, EdgeProperty>;
        using BoostGraphType = typename ::GraphLibrary::Graph<NodeProperty, EdgeProperty>::BoostGraphType;
        using BoostVertexDescriptor = typename Match::BoostVertexDescriptor;
        using BoostEdgeDescriptor = typename Match::BoostEdgeDescriptor;

        MatchCollector(const BoostGraphType& pattern, const BoostGraphType& host, std::vector<Match>& matches, const ::GraphLibrary::Rule<NodeProperty, EdgeProperty>* rule = nullptr)
            : pattern_(pattern)
            , host_(host)
            , matches_(matches)
            , rule_(rule)
        {
        }

        template<typename Map1To2, typename Map2To1>
        bool operator()(Map1To2 map1to2, Map2To1) // this is called by vf2 algorithm when a match is found.
        {
            Match match;
            for (auto vertex : boost::make_iterator_range(vertices(pattern_)))
            {
                auto host_vertex = boost::get(map1to2, vertex);
                match.vertices.push_back({ vertex, host_vertex });
            }

            for (auto edge : boost::make_iterator_range(edges(pattern_)))
            {
                auto source_vertex = source(edge, pattern_);
                auto target_vertex = target(edge, pattern_);

                // the reason we can use .value() here is that vf2 guarantees that edges are only matched if their source and target vertices are matched and we set the vertex mapping above
                auto source_host = match.HostForPattern(source_vertex).value(); // NOLINT(bugprone-unchecked-optional-access)
                auto target_host = match.HostForPattern(target_vertex).value(); // NOLINT(bugprone-unchecked-optional-access)

                BoostEdgeDescriptor mapped_edge;
                if (FindMatchingHostEdge(source_host, target_host, edge, mapped_edge))
                {
                    match.edges.push_back(mapped_edge);
                }
            }

            matches_.push_back(std::move(match));
            return true;
        }

    private:
        bool FindMatchingHostEdge(
            BoostVertexDescriptor source,
            BoostVertexDescriptor target,
            BoostEdgeDescriptor pattern_edge,
            BoostEdgeDescriptor& out_edge) const
        {
            auto [begin, end] = boost::out_edges(source, host_);
            for (auto it = begin; it != end; ++it)
            {
                if (boost::target(*it, host_) != target) continue;
                if (rule_)
                {
                    auto pred = rule_->GetEdgePredicate(pattern_edge);
                    if (pred)
                    {
                        if ((*pred)(host_[*it]))
                        {
                            out_edge = *it;
                            return true;
                        }
                        continue;
                    }
                }

                if (host_[*it] == pattern_[pattern_edge])
                {
                    out_edge = *it;
                    return true;
                }
            }
            return false;
        }

        const BoostGraphType& pattern_;
        const BoostGraphType& host_;
        std::vector<Match>& matches_;
        const ::GraphLibrary::Rule<NodeProperty, EdgeProperty>* rule_ = nullptr;
    };

    template<typename NodeProperty, typename EdgeProperty>
    struct NodeComparator
    {
        using GraphType = typename ::GraphLibrary::Graph<NodeProperty, EdgeProperty>::BoostGraphType;
        using VertexDescriptor = typename GraphType::vertex_descriptor;

        NodeComparator(const GraphType& pattern, const GraphType& host, const ::GraphLibrary::Rule<NodeProperty, EdgeProperty>* rule = nullptr)
            : pattern(pattern)
            , host(host)
            , rule(rule)
        {
        }

        bool operator()(VertexDescriptor lhs, VertexDescriptor rhs) const
        {
            if (rule)
            {
                auto pred = rule->GetNodePredicate(lhs);
                if (pred)
                {
                    return (*pred)(host[rhs]);
                }
            }
            return pattern[lhs] == host[rhs];
        }

        const GraphType& pattern;
        const GraphType& host;
        const ::GraphLibrary::Rule<NodeProperty, EdgeProperty>* rule = nullptr;
    };

    template<typename NodeProperty, typename EdgeProperty>
    struct EdgeComparator
    {
        using GraphType = typename ::GraphLibrary::Graph<NodeProperty, EdgeProperty>::BoostGraphType;
        using EdgeDescriptor = typename GraphType::edge_descriptor;

        EdgeComparator(const GraphType& pattern, const GraphType& host, const ::GraphLibrary::Rule<NodeProperty, EdgeProperty>* rule = nullptr)
            : pattern(pattern)
            , host(host)
            , rule(rule)
        {
        }

        bool operator()(EdgeDescriptor lhs, EdgeDescriptor rhs) const
        {
            if (rule)
            {
                auto pred = rule->GetEdgePredicate(lhs);
                if (pred)
                {
                    return (*pred)(host[rhs]);
                }
            }
            return pattern[lhs] == host[rhs];
        }

        const GraphType& pattern;
        const GraphType& host;
        const ::GraphLibrary::Rule<NodeProperty, EdgeProperty>* rule = nullptr;
    };
} // namespace GraphLibrary::Detail
    
#endif // GRAPHLIBRARY_DETAIL_HPP