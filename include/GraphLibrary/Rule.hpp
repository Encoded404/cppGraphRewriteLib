#ifndef GRAPHLIBRARY_RULE_HPP
#define GRAPHLIBRARY_RULE_HPP

#include "TypeDefs.hpp"
#include "GraphLibrary.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>

namespace GraphLibrary
{
    /// @brief Represents a graph transformation rule.
    /// @tparam NodeProperty The type of properties associated with nodes.
    /// @tparam EdgeProperty The type of properties associated with edges.
    template<typename NodeProperty, typename EdgeProperty>
    class Rule
    {
    public:
        class NodeConstraintBase;
        class EdgeConstraintBase;
    private:
        Graph<NodeProperty, EdgeProperty> LGraph_;
        Graph<NodeProperty, EdgeProperty> RGraph_;
        std::vector<std::shared_ptr<NodeConstraintBase>> node_constraints_;
        std::vector<std::shared_ptr<EdgeConstraintBase>> edge_constraints_;

        using BoostVertexDescriptor = typename Graph<NodeProperty, EdgeProperty>::BoostVertexDescriptor;
        using BoostEdgeDescriptor = typename Graph<NodeProperty, EdgeProperty>::BoostEdgeDescriptor;

        std::vector<std::pair<BoostVertexDescriptor, BoostVertexDescriptor>> nodeMapping_;
        std::vector<std::pair<BoostVertexDescriptor, bool>> modifiedNodes_;
        
        std::vector<std::pair<BoostEdgeDescriptor, BoostEdgeDescriptor>> edgeMapping_;
        std::vector<std::pair<BoostEdgeDescriptor, bool>> modifiedEdges_;

        std::vector<std::pair<BoostVertexDescriptor, std::function<bool(const NodeProperty&)>>> nodePredicates_;
        std::vector<std::pair<BoostEdgeDescriptor, std::function<bool(const EdgeProperty&)>>> edgePredicates_;
    public:

        class NodeConstraintBase
        {
        public:
            virtual ~NodeConstraintBase() = default;
            virtual BoostVertexDescriptor GetLeftNode() const = 0;
            virtual BoostVertexDescriptor GetRightNode() const = 0;
            virtual bool Evaluate(const NodeProperty& left_node, const NodeProperty& right_node) const = 0;
        };
        
        template<typename MemberType>
        class ConcreteNodeConstraint : public NodeConstraintBase
        {
        public:
            ConcreteNodeConstraint(
                BoostVertexDescriptor left,
                BoostVertexDescriptor right,
                MemberType NodeProperty::*left_member,
                MemberType NodeProperty::*right_member,
                std::function<bool(const MemberType&, const MemberType&)> comparator)
                : left_(left)
                , right_(right)
                , left_member_(left_member)
                , right_member_(right_member)
                , comparator_(std::move(comparator))
            {
            }

            BoostVertexDescriptor GetLeftNode() const override
            {
                return left_;
            }

            BoostVertexDescriptor GetRightNode() const override
            {
                return right_;
            }

            bool Evaluate(const NodeProperty& lhs, const NodeProperty& rhs) const override
            {
                return comparator_(
                    lhs.*left_member_,
                    rhs.*right_member_
                );
            }

        private:
            BoostVertexDescriptor left_;
            BoostVertexDescriptor right_;
            MemberType NodeProperty::*left_member_;
            MemberType NodeProperty::*right_member_;
            std::function<bool(const MemberType&, const MemberType&)> comparator_;
        };

        class EdgeConstraintBase
        {
        public:
            virtual ~EdgeConstraintBase() = default;
            virtual BoostEdgeDescriptor GetLeftEdge() const = 0;
            virtual BoostEdgeDescriptor GetRightEdge() const = 0;
            virtual bool Evaluate(const EdgeProperty& left_edge, const EdgeProperty& right_edge) const = 0;
        };

        template<typename MemberType>
        class ConcreteEdgeConstraint : public EdgeConstraintBase
        {
        public:
            ConcreteEdgeConstraint(
                BoostEdgeDescriptor left,
                BoostEdgeDescriptor right,
                MemberType EdgeProperty::*left_member,
                MemberType EdgeProperty::*right_member,
                std::function<bool(const MemberType&, const MemberType&)> comparator)
                : left_(left)
                , right_(right)
                , left_member_(left_member)
                , right_member_(right_member)
                , comparator_(std::move(comparator))
            {
            }

            BoostEdgeDescriptor GetLeftEdge() const override
            {
                return left_;
            }

            BoostEdgeDescriptor GetRightEdge() const override
            {
                return right_;
            }

            bool Evaluate(const EdgeProperty& lhs, const EdgeProperty& rhs) const override
            {
                return comparator_(
                    lhs.*left_member_,
                    rhs.*right_member_
                );
            }

        private:
            BoostEdgeDescriptor left_;
            BoostEdgeDescriptor right_;
            MemberType EdgeProperty::*left_member_;
            MemberType EdgeProperty::*right_member_;
            std::function<bool(const MemberType&, const MemberType&)> comparator_;
        };

        Rule() = default;
        ~Rule() = default;

        // non-copyable: Boost descriptors inside Rule are not safe to copy
        Rule(const Rule& other) = delete;
        Rule& operator=(const Rule& other) = delete;

        // move semantics
        Rule(Rule&&) noexcept = default;
        Rule& operator=(Rule&&) noexcept = default;
                
        int AddNodeConstraint(std::shared_ptr<NodeConstraintBase> constraint)
        {
            node_constraints_.push_back(std::move(constraint));
            const int idx = static_cast<int>(node_constraints_.size() - 1);
            LOGIFACE_LOG(debug, "Rule: added node constraint at index " + std::to_string(idx));
            return idx;
        }

        void RemoveNodeConstraint(int index)
        {
            if (index >= 0 && static_cast<size_t>(index) < node_constraints_.size())
            {
                node_constraints_.erase(node_constraints_.begin() + index);
                LOGIFACE_LOG(debug, "Rule: removed node constraint at index " + std::to_string(index));
            }
        }

        int AddEdgeConstraint(std::shared_ptr<EdgeConstraintBase> constraint)
        {
            edge_constraints_.push_back(std::move(constraint));
            int idx = static_cast<int>(edge_constraints_.size() - 1);
            LOGIFACE_LOG(debug, "Rule: added edge constraint at index " + std::to_string(idx));
            return idx;
        }

        void RemoveEdgeConstraint(int index)
        {
            if (index >= 0 && static_cast<size_t>(index) < edge_constraints_.size())
            {
                edge_constraints_.erase(edge_constraints_.begin() + index);
                LOGIFACE_LOG(debug, "Rule: removed edge constraint at index " + std::to_string(index));
            }
        }

        using NodePred = std::function<bool(const NodeProperty&)>;
        using EdgePred = std::function<bool(const EdgeProperty&)>;

        void SetNodePredicate(BoostVertexDescriptor desc, NodePred pred)
        {
            auto it = std::find_if(nodePredicates_.begin(), nodePredicates_.end(), [&desc](const auto& e){ return e.first == desc; });
            if (it != nodePredicates_.end()) it->second = std::move(pred);
            else nodePredicates_.emplace_back(desc, std::move(pred));
            LOGIFACE_LOG(debug, "Rule: set node predicate for vertex");
        }

        void SetEdgePredicate(BoostEdgeDescriptor desc, EdgePred pred)
        {
            auto it = std::find_if(edgePredicates_.begin(), edgePredicates_.end(), [&desc](const auto& e){ return e.first == desc; });
            if (it != edgePredicates_.end()) it->second = std::move(pred);
            else edgePredicates_.emplace_back(desc, std::move(pred));
            LOGIFACE_LOG(debug, "Rule: set edge predicate for edge");
        }

        bool HasNodePredicate(BoostVertexDescriptor desc) const
        {
            return std::any_of(nodePredicates_.begin(), nodePredicates_.end(), [&desc](const auto& e){ return e.first == desc; });
        }

        bool HasEdgePredicate(BoostEdgeDescriptor desc) const
        {
            return std::any_of(edgePredicates_.begin(), edgePredicates_.end(), [&desc](const auto& e){ return e.first == desc; });
        }

        std::optional<NodePred> GetNodePredicate(BoostVertexDescriptor desc) const
        {
            auto it = std::find_if(nodePredicates_.begin(), nodePredicates_.end(), [&desc](const auto& e){ return e.first == desc; });
            if (it == nodePredicates_.end()) return std::nullopt;
            return it->second;
        }

        std::optional<EdgePred> GetEdgePredicate(BoostEdgeDescriptor desc) const
        {
            auto it = std::find_if(edgePredicates_.begin(), edgePredicates_.end(), [&desc](const auto& e){ return e.first == desc; });
            if (it == edgePredicates_.end()) return std::nullopt;
            return it->second;
        }

        void LinkNodes(BoostVertexDescriptor l_desc, BoostVertexDescriptor r_desc)
        {
            auto mapping_it = std::find_if(nodeMapping_.begin(), nodeMapping_.end(), [&l_desc](const auto& entry) {
                return entry.first == l_desc;
            });

            if (mapping_it != nodeMapping_.end())
            {
                mapping_it->second = r_desc;
            }
            else
            {
                nodeMapping_.emplace_back(l_desc, r_desc);
            }

            const auto& lhs_impl_boost = LGraph_.GetInternalGraph();
            const auto& rhs_impl_boost = RGraph_.GetInternalGraph();
            const NodeProperty& lhs_prop = lhs_impl_boost[l_desc];
            const NodeProperty& rhs_prop = rhs_impl_boost[r_desc];

            const bool is_modified = !(lhs_prop == rhs_prop);

            auto modified_it = std::find_if(modifiedNodes_.begin(), modifiedNodes_.end(), [&l_desc](const auto& entry) {
                return entry.first == l_desc;
            });

            if (modified_it != modifiedNodes_.end())
            {
                modified_it->second = is_modified;
            }
            else
            {
                modifiedNodes_.emplace_back(l_desc, is_modified);
            }

            LOGIFACE_LOG(debug, "Rule: linked L-node -> R-node (modified=" + std::string(is_modified ? "true" : "false") + ")");
        }

        void LinkEdges(BoostEdgeDescriptor l_desc, BoostEdgeDescriptor r_desc)
        {
            auto mapping_it = std::find_if(edgeMapping_.begin(), edgeMapping_.end(), [&l_desc](const auto& entry) {
                return entry.first == l_desc;
            });

            if (mapping_it != edgeMapping_.end())
            {
                mapping_it->second = r_desc;
            }
            else
            {
                edgeMapping_.emplace_back(l_desc, r_desc);
            }

            const auto& lhs_impl_boost = LGraph_.GetInternalGraph();
            const auto& rhs_impl_boost = RGraph_.GetInternalGraph();
            const EdgeProperty& lhs_prop = lhs_impl_boost[l_desc];
            const EdgeProperty& rhs_prop = rhs_impl_boost[r_desc];

            const bool is_modified = !(lhs_prop == rhs_prop);

            auto modified_it = std::find_if(modifiedEdges_.begin(), modifiedEdges_.end(), [&l_desc](const auto& entry) {
                return entry.first == l_desc;
            });

            if (modified_it != modifiedEdges_.end())
            {
                modified_it->second = is_modified;
            }
            else
            {
                modifiedEdges_.emplace_back(l_desc, is_modified);
            }

            LOGIFACE_LOG(debug, "Rule: linked L-edge -> R-edge (modified=" + std::string(is_modified ? "true" : "false") + ")");
        }

        bool HasEdgeMapping(BoostEdgeDescriptor descriptor) const
        {
            return std::any_of(edgeMapping_.begin(), edgeMapping_.end(), [&descriptor](const auto& entry) {
                return entry.first == descriptor;
            });
        }

        std::optional<BoostEdgeDescriptor> GetMappedEdge(BoostEdgeDescriptor descriptor) const
        {
            auto it = std::find_if(edgeMapping_.begin(), edgeMapping_.end(), [&descriptor](const auto& entry) {
                return entry.first == descriptor;
            });

            if (it == edgeMapping_.end())
            {
                return std::nullopt;
            }

            return it->second;
        }

        bool EdgeWillBeModified(BoostEdgeDescriptor descriptor) const
        {
            auto it = std::find_if(modifiedEdges_.begin(), modifiedEdges_.end(), [&descriptor](const auto& entry) {
                return entry.first == descriptor;
            });

            if (it != modifiedEdges_.end())
            {
                return it->second;
            }

            return true;
        }

        bool HasNodeMapping(BoostVertexDescriptor descriptor) const
        {
            return std::any_of(nodeMapping_.begin(), nodeMapping_.end(), [&descriptor](const auto& entry) {
                return entry.first == descriptor;
            });
        }

        std::optional<BoostVertexDescriptor> GetMappedNode(BoostVertexDescriptor descriptor) const
        {
            auto it = std::find_if(nodeMapping_.begin(), nodeMapping_.end(), [&descriptor](const auto& entry) {
                return entry.first == descriptor;
            });

            if (it == nodeMapping_.end())
            {
                return std::nullopt;
            }

            return it->second;
        }

        bool NodeWillBeModified(BoostVertexDescriptor descriptor) const
        {
            auto it = std::find_if(modifiedNodes_.begin(), modifiedNodes_.end(), [&descriptor](const auto& entry) {
                return entry.first == descriptor;
            });

            if (it != modifiedNodes_.end())
            {
                return it->second;
            }

            return true;
        }

        /// @brief Retrieves the left graph of the rule.
        /// @return A reference to the left graph.
        Graph<NodeProperty, EdgeProperty>& GetLGraph() { return LGraph_; }
        /// @brief Retrieves the left graph of the rule.
        /// @return A constant reference to the left graph.
        const Graph<NodeProperty, EdgeProperty>& GetLGraph() const { return LGraph_; }

        /// @brief Retrieves the right graph of the rule.
        /// @return A reference to the right graph.
        Graph<NodeProperty, EdgeProperty>& GetRGraph() { return RGraph_; }
        /// @brief Retrieves the right graph of the rule.
        /// @return A constant reference to the right graph.
        const Graph<NodeProperty, EdgeProperty>& GetRGraph() const { return RGraph_; }

        const std::vector<std::shared_ptr<NodeConstraintBase>>& GetNodeConstraints() const { return node_constraints_; }
        const std::vector<std::shared_ptr<EdgeConstraintBase>>& GetEdgeConstraints() const { return edge_constraints_; }
    };

    template<typename NodeProperty, typename EdgeProperty>
    class RuleSet
    {
    private:
        std::vector<std::shared_ptr<Rule<NodeProperty, EdgeProperty>>> rules_;
    public:
        /// @brief Adds a rule to the RuleSet by reference (non-owning).
        /// @details The RuleSet will store a non-owning shared_ptr that does not delete
        /// the pointed object. The caller is responsible for ensuring the referenced
        /// `rule` remains alive for the lifetime of the RuleSet entry.
        void AddRule(const Rule<NodeProperty, EdgeProperty>& rule)
        {
            rules_.push_back(std::shared_ptr<Rule<NodeProperty, EdgeProperty>>(
                const_cast<Rule<NodeProperty, EdgeProperty>*>(&rule),
                [](Rule<NodeProperty, EdgeProperty>*){}
            ));
        }

        /// @brief Adds an owning shared_ptr to a rule.
        void AddRule(std::shared_ptr<Rule<NodeProperty, EdgeProperty>> rule_ptr)
        {
            rules_.push_back(std::move(rule_ptr));
        }

        /// @brief Adds a rule to the RuleSet by value and moves it into storage (owning).
        void AddRule(Rule<NodeProperty, EdgeProperty>&& rule)
        {
            rules_.push_back(std::make_shared<Rule<NodeProperty, EdgeProperty>>(std::move(rule)));
        }

        /// @brief Removes a rule from the RuleSet at the given index.
        /// @param index The index of the rule to remove.
        void RemoveRule(size_t index)
        {
            if (index < rules_.size()) {
                rules_.erase(rules_.begin() + index);
            }
        }

        /// @brief Retrieves a rule from the RuleSet at the given index.
        /// @param index The index of the rule to retrieve.
        /// @return A reference to the rule at the specified index.
        std::shared_ptr<Rule<NodeProperty, EdgeProperty>>& GetRule(size_t index)
        {
            return rules_.at(index);
        }

        /// @brief Removes and returns a rule from the RuleSet at the given index.
        /// @param index The index of the rule to remove and return.
        /// @return The rule that was removed.
        std::shared_ptr<Rule<NodeProperty, EdgeProperty>> PopRule(size_t index)
        {
            auto rule = std::move(rules_.at(index));
            rules_.erase(rules_.begin() + index);
            return rule;
        }
        
        /// @brief Retrieves all rules in the RuleSet.
        /// @return A vector containing all rules.
        std::vector<std::shared_ptr<Rule<NodeProperty, EdgeProperty>>>& GetRules()
        {
            return rules_;
        }

        const std::vector<std::shared_ptr<Rule<NodeProperty, EdgeProperty>>>& GetRules() const
        {
            return rules_;
        }

        size_t Size() const
        {
            return rules_.size();
        }
    };
} // namespace GraphLibrary

// Template implementations

#endif // GRAPHLIBRARY_RULE_HPP