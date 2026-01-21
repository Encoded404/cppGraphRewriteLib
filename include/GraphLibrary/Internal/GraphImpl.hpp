#ifndef GRAPHLIBRARY_GRAPHIMPL_HPP
#define GRAPHLIBRARY_GRAPHIMPL_HPP

#include "ForwardDeclarations.hpp"

#include "Detail.hpp"

#include <vector>
#include <map>
#include <cstddef>
#include <algorithm>

namespace GraphLibrary
{
    template<typename NodeProperty, typename EdgeProperty>
    ::std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>> Graph<NodeProperty, EdgeProperty>::CollectMatches(
        const Rule<NodeProperty, EdgeProperty>& rule,
        const typename Graph<NodeProperty, EdgeProperty>::BoostGraphType& target_graph)
    {
        using Match = Detail::MatchResult<NodeProperty, EdgeProperty>;
        const Graph<NodeProperty, EdgeProperty>& pattern_graph = rule.GetLGraph();
        const auto& pattern_boost = pattern_graph.GetInternalGraph();

        ::std::vector<Match> matches;
        const Detail::MatchCollector<NodeProperty, EdgeProperty> collector(pattern_boost, target_graph, matches, &rule);

        // Build associative index maps so we can keep listS vertex storage (stable descriptors)
        std::map<typename BoostGraphType::vertex_descriptor, std::size_t> pattern_index_map;
        std::map<typename BoostGraphType::vertex_descriptor, std::size_t> target_index_map;

        std::size_t idx = 0;
        for (auto v : boost::make_iterator_range(vertices(pattern_boost)))
        {
            pattern_index_map[v] = idx++;
        }

        idx = 0;
        for (auto v : boost::make_iterator_range(vertices(target_graph)))
        {
            target_index_map[v] = idx++;
        }

        // Build a vertex order vector (sorted by multiplicity) without
        // relying on a built-in vertex_index property map.
        std::vector<typename BoostGraphType::vertex_descriptor> vertex_order;
        vertex_order.reserve(pattern_index_map.size());
        for (auto v : boost::make_iterator_range(vertices(pattern_boost)))
        {
            vertex_order.push_back(v);
        }
        std::sort(vertex_order.begin(), vertex_order.end(), [&](typename BoostGraphType::vertex_descriptor a, typename BoostGraphType::vertex_descriptor b){
            auto deg_a = out_degree(a, pattern_boost) + in_degree(a, pattern_boost);
            auto deg_b = out_degree(b, pattern_boost) + in_degree(b, pattern_boost);
            return deg_a > deg_b;
        });

        boost::vf2_subgraph_mono(
            pattern_boost,
            target_graph,
            collector,
            boost::make_assoc_property_map(pattern_index_map),
            boost::make_assoc_property_map(target_index_map),
            vertex_order,
            Detail::EdgeComparator<NodeProperty, EdgeProperty>(pattern_boost, target_graph, &rule),
            Detail::NodeComparator<NodeProperty, EdgeProperty>(pattern_boost, target_graph, &rule));
        return matches;
    }

    template<typename NodeProperty, typename EdgeProperty>
    static bool MatchesConflict(
        const Detail::MatchResult<NodeProperty, EdgeProperty>& existing,
        const Detail::MatchResult<NodeProperty, EdgeProperty>& candidate,
        const Rule<NodeProperty, EdgeProperty>& rule)
    {
        for (const auto& edge : existing.edges)
        {
            if (candidate.UsesHostEdge(edge))
            {
                return true;
            }
        }

        for (const auto& vertex_pair : existing.vertices)
        {
            auto candidate_pattern = candidate.PatternForHost(vertex_pair.host);
            if (!candidate_pattern)
            {
                continue;
            }

            if (rule.NodeWillBeModified(vertex_pair.pattern) ||
                rule.NodeWillBeModified(*candidate_pattern))
            {
                return true;
            }
        }

        return false;
    }

    template<typename NodeProperty, typename EdgeProperty>
    static ::std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>> RemoveOverlappingMatches(
        const ::std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>>& matches,
        const Rule<NodeProperty, EdgeProperty>& rule)
    {
        // use rule public API
        ::std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>> filtered;
        for (const auto& match : matches)
        {
            bool conflict = false;
            for (const auto& existing : filtered)
            {
                if (MatchesConflict(existing, match, rule))
                {
                    conflict = true;
                    break;
                }
            }

            if (!conflict)
            {
                filtered.push_back(match);
            }
        }
        return filtered;
    }

    template<typename NodeProperty, typename EdgeProperty>
    static bool MatchesConstraints(
        const Detail::MatchResult<NodeProperty, EdgeProperty>& match,
        const Rule<NodeProperty, EdgeProperty>& rule,
        const typename Graph<NodeProperty, EdgeProperty>::BoostGraphType& host_graph)
    {
        const auto& constraints = rule.GetNodeConstraints();
        for (const auto& constraint : constraints)
        {
            auto left_descriptor = constraint->GetLeftNode();
            auto right_descriptor = constraint->GetRightNode();

            auto left_host = match.HostForPattern(left_descriptor);
            auto right_host = match.HostForPattern(right_descriptor);

            if (!left_host || !right_host)
            {
                return false;
            }

            if (!constraint->Evaluate(host_graph[*left_host], host_graph[*right_host]))
            {
                return false;
            }
        }

        // Edge constraints
        const auto& edge_constraints = rule.GetEdgeConstraints();
        const auto& pattern_boost = rule.GetLGraph().GetInternalGraph();
        for (const auto& econstraint : edge_constraints)
        {
            auto left_e = econstraint->GetLeftEdge();
            auto right_e = econstraint->GetRightEdge();

            auto left_src = source(left_e, pattern_boost);
            auto left_tgt = target(left_e, pattern_boost);
            auto right_src = source(right_e, pattern_boost);
            auto right_tgt = target(right_e, pattern_boost);

            auto left_host_src = match.HostForPattern(left_src);
            auto left_host_tgt = match.HostForPattern(left_tgt);
            auto right_host_src = match.HostForPattern(right_src);
            auto right_host_tgt = match.HostForPattern(right_tgt);

            if (!left_host_src || !left_host_tgt || !right_host_src || !right_host_tgt)
            {
                return false;
            }

            // find corresponding host edges
            typename Graph<NodeProperty, EdgeProperty>::BoostEdgeDescriptor left_host_edge;
            typename Graph<NodeProperty, EdgeProperty>::BoostEdgeDescriptor right_host_edge;
            bool found_left = false;
            bool found_right = false;

            auto [lb, le] = boost::out_edges(*left_host_src, host_graph);
            for (auto it = lb; it != le; ++it)
            {
                if (boost::target(*it, host_graph) == *left_host_tgt)
                {
                    left_host_edge = *it;
                    found_left = true;
                    break;
                }
            }

            auto [rb, re] = boost::out_edges(*right_host_src, host_graph);
            for (auto it = rb; it != re; ++it)
            {
                if (boost::target(*it, host_graph) == *right_host_tgt)
                {
                    right_host_edge = *it;
                    found_right = true;
                    break;
                }
            }

            if (!found_left || !found_right)
            {
                return false;
            }

            if (!econstraint->Evaluate(host_graph[left_host_edge], host_graph[right_host_edge]))
            {
                return false;
            }
        }

        return true;
    }

    template<typename NodeProperty, typename EdgeProperty>
    static ::std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>> FilterMatchesByConstraints(
        const ::std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>>& matches,
        const Rule<NodeProperty, EdgeProperty>& rule,
        const typename Graph<NodeProperty, EdgeProperty>::BoostGraphType& host_graph)
    {
        ::std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>> filtered;
        for (const auto& match : matches)
        {
            if (MatchesConstraints(match, rule, host_graph))
            {
                filtered.push_back(match);
            }
        }
        return filtered;
    }

    template<typename VertexDescriptor>
    static std::optional<VertexDescriptor>
    ResolveHostForRNode(
        const std::vector<std::pair<VertexDescriptor, VertexDescriptor>>& mapping,
        VertexDescriptor node)
    {
        auto it = std::find_if(
            mapping.begin(),
            mapping.end(),
            [&node](const auto& entry) { return entry.first == node; });
        if (it != mapping.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    template<typename NodeProperty, typename EdgeProperty>
    static void ApplyMatches(
        typename Graph<NodeProperty, EdgeProperty>::BoostGraphType& host_graph,
        const ::std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>>& matches,
        const Rule<NodeProperty, EdgeProperty>& rule)
    {
        const auto& rhs_graph = rule.GetRGraph();
        const auto& rhs_impl_boost = rhs_graph.GetInternalGraph();

        for (const auto& match : matches)
        {
            for (const auto& edge : match.edges)
            {
                boost::remove_edge(edge, host_graph);
            }

            ::std::vector<typename Detail::MatchResult<NodeProperty, EdgeProperty>::BoostVertexDescriptor> nodes_to_delete;
            for (const auto& vertex_pair : match.vertices)
            {
                if (!rule.HasNodeMapping(vertex_pair.pattern))
                {
                    nodes_to_delete.push_back(vertex_pair.host);
                }
            }

            for (const auto& node : nodes_to_delete)
            {
                boost::clear_vertex(node, host_graph);
                boost::remove_vertex(node, host_graph);
            }

            for (const auto& vertex_pair : match.vertices)
            {
                auto mapped_node = rule.GetMappedNode(vertex_pair.pattern);
                if (!mapped_node)
                {
                    continue;
                }

                auto host_descriptor = vertex_pair.host;
                host_graph[host_descriptor] = rhs_impl_boost[*mapped_node];
            }

            ::std::vector<std::pair<typename Detail::MatchResult<NodeProperty, EdgeProperty>::BoostVertexDescriptor,
                                  typename Detail::MatchResult<NodeProperty, EdgeProperty>::BoostVertexDescriptor>>
                r_node_to_host;

            for (const auto& vertex_pair : match.vertices)
            {
                if (auto mapped_node = rule.GetMappedNode(vertex_pair.pattern))
                {
                    r_node_to_host.emplace_back(*mapped_node, vertex_pair.host);
                }
            }

            for (auto vertex : boost::make_iterator_range(vertices(rhs_impl_boost)))
            {
                if (!ResolveHostForRNode(r_node_to_host, vertex))
                {
                    auto added = boost::add_vertex(rhs_impl_boost[vertex], host_graph);
                    r_node_to_host.emplace_back(vertex, added);
                }
            }

            for (auto edge : boost::make_iterator_range(edges(rhs_impl_boost)))
            {
                auto source_vertex = source(edge, rhs_impl_boost);
                auto target_vertex = target(edge, rhs_impl_boost);
                auto host_source = ResolveHostForRNode(r_node_to_host, source_vertex);
                auto host_target = ResolveHostForRNode(r_node_to_host, target_vertex);

                if (!host_source || !host_target)
                {
                    continue;
                }

                boost::add_edge(*host_source, *host_target, rhs_impl_boost[edge], host_graph);
            }
        }
    }

    template<typename NodeProperty, typename EdgeProperty>
    typename Graph<NodeProperty, EdgeProperty>::BoostGraphType& Graph<NodeProperty, EdgeProperty>::ApplyRuleInternal(
        const Rule<NodeProperty, EdgeProperty>& rule,
        BoostGraphType& graph) const
    {
        const std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>> matches = CollectMatches(rule, graph);
        const std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>> non_overlapping = RemoveOverlappingMatches(matches, rule);
        const std::vector<Detail::MatchResult<NodeProperty, EdgeProperty>> valid_matches = FilterMatchesByConstraints(non_overlapping, rule, graph);
        ApplyMatches(graph, valid_matches, rule);
        return graph;
    }

    template<typename NodeProperty, typename EdgeProperty>
    Graph<NodeProperty, EdgeProperty> Graph<NodeProperty, EdgeProperty>::ApplyRule(const Rule<NodeProperty, EdgeProperty>& rule) const
    {
        BoostGraphType working_graph = boostGraph_;
        ApplyRuleInternal(rule, working_graph);

        Graph<NodeProperty, EdgeProperty> result;
        result.GetInternalGraph() = std::move(working_graph);
        return result;
    }

    template<typename NodeProperty, typename EdgeProperty>
    Graph<NodeProperty, EdgeProperty> Graph<NodeProperty, EdgeProperty>::ApplyRuleSet(const RuleSet<NodeProperty, EdgeProperty>& rule_set) const
    {
        BoostGraphType working_graph = boostGraph_;

        for (const auto& rptr : rule_set.GetRules())
        {
            ApplyRuleInternal(*rptr, working_graph);
        }

        Graph<NodeProperty, EdgeProperty> result;
        result.GetInternalGraph() = std::move(working_graph);
        return result;
    }

    template<typename NodeProperty, typename EdgeProperty>
    Graph<NodeProperty, EdgeProperty> Graph<NodeProperty, EdgeProperty>::ApplyRuleSets(const std::vector<RuleSet<NodeProperty, EdgeProperty>>& rule_sets) const
    {
        BoostGraphType working_graph = boostGraph_;

        for (const auto& set : rule_sets)
        {
            for (const auto& rptr : set.GetRules())
            {
                ApplyRuleInternal(*rptr, working_graph);
            }
        }

        Graph<NodeProperty, EdgeProperty> result;
        result.GetInternalGraph() = std::move(working_graph);
        return result;
    }
} // namespace GraphLibrary

#endif // GRAPHLIBRARY_GRAPHIMPL_HPP