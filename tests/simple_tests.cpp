#include <gtest/gtest.h>

#include <logging/logging.hpp>
#include <logging/logging_macros.hpp>

#include <GraphLibrary/GraphLibrary.hpp>
#include <GraphLibrary/Rule.hpp>

#include "test_logging.hpp"

#include <cstdint>
#include <memory> // provides make_shared

//using modern_cmake_library::example;

namespace {
// Install the per-test file logger once for the test binary.
[[maybe_unused]] const bool kLoggerInstalled = [] {
    TestLogging::InstallPerTestFileLogger();
    return true;
}();
}  // namespace

/*TEST(ExampleTest, ExampleFunctionDoublesInput) {
    example ex;

    LOGIFACE_LOG(info, "[ExampleFunctionDoublesInput] calling with 0");
    EXPECT_EQ(ex.exampleFunction(0), 0);

    LOGIFACE_LOG(info, "[ExampleFunctionDoublesInput] calling with 1");
    EXPECT_EQ(ex.exampleFunction(1), 2);

    LOGIFACE_LOG(info, "[ExampleFunctionDoublesInput] calling with 5");
    EXPECT_EQ(ex.exampleFunction(5), 10);

    LOGIFACE_LOG(info, "[ExampleFunctionDoublesInput] calling with -3");
    EXPECT_EQ(ex.exampleFunction(-3), -6);
*/

// Minimal test properties used for unit tests
enum class TestNodeType : uint8_t { A = 0, B = 1 };
enum class TestEdgeType : uint8_t { X = 0, Y = 1 };

struct TestNodeProp {
    int value{};
    bool operator==(const TestNodeProp& o) const { return value == o.value; }
};

struct TestEdgeProp {
    int weight{};
    TestEdgeType type{};
    bool operator==(const TestEdgeProp& o) const { return weight == o.weight && type == o.type; }
};

using TestGraph = GraphLibrary::Graph<TestNodeProp, TestEdgeProp>;

TEST(GraphBasic, AddNodesEdges) {
    TestGraph g;

    auto v1 = g.AddNode(TestNodeProp{1});
    auto v2 = g.AddNode(TestNodeProp{2});

    EXPECT_EQ(g.GetVertices().size(), 2u);

    g.AddEdge(v1, v2, TestEdgeProp{10, TestEdgeType::X});
    EXPECT_EQ(g.GetEdges().size(), 1u);

    // second call: add another edge (opposite direction)
    g.AddEdge(v2, v1, TestEdgeProp{10, TestEdgeType::X});
    EXPECT_EQ(g.GetEdges().size(), 2u);

    LOGIFACE_LOG(info, "test: \"AddNodesEdges\" passed");
}

TEST(GraphModify, NodeEdgeRemoval) {
    TestGraph g;
    auto v1 = g.AddNode(TestNodeProp{5});
    auto v2 = g.AddNode(TestNodeProp{6});
    auto e = g.AddEdge(v1, v2, TestEdgeProp{3, TestEdgeType::X});

    EXPECT_EQ(g.GetVertices().size(), 2u);
    EXPECT_EQ(g.GetEdges().size(), 1u);

    // remove edge using boost on the internal graph
    // use Graph API to remove edge
    g.RemoveEdge(e);
    EXPECT_EQ(g.GetEdges().size(), 0u);

    // remove a vertex
    // use Graph API to remove node
    g.RemoveNode(v1);
    EXPECT_EQ(g.GetVertices().size(), 1u);
    LOGIFACE_LOG(info, "test: \"NodeEdgeRemoval\" passed");
}

TEST(RuleEdgeOps, RemoveEdgeViaRuleAndTransferProperties) {
    TestGraph g;

    auto v1 = g.AddNode(TestNodeProp{1});
    auto v2 = g.AddNode(TestNodeProp{2});
    g.AddEdge(v1, v2, TestEdgeProp{5, TestEdgeType::X});
    EXPECT_EQ(g.GetEdges().size(), 1u);

    using RuleType = GraphLibrary::Rule<TestNodeProp, TestEdgeProp>;
    RuleType rule;
    auto& l = rule.GetLGraph();
    auto& r = rule.GetRGraph();

    // pattern: two nodes with an edge
    auto l1 = l.AddNode(TestNodeProp{1});
    auto l2 = l.AddNode(TestNodeProp{2});
    l.AddEdge(l1, l2, TestEdgeProp{5, TestEdgeType::X});

    // replacement: same two nodes but with different node properties and no edge
    auto r1 = r.AddNode(TestNodeProp{10});
    auto r2 = r.AddNode(TestNodeProp{20});

    // ensure nodes are linked so they are not removed during application
    rule.LinkNodes(l1, r1);
    rule.LinkNodes(l2, r2);

    // apply rule and verify edge removed and node properties transferred
    auto result = g.ApplyRule(rule);
    EXPECT_EQ(result.GetEdges().size(), 0u);

    // verify node properties were replaced by r's node properties
    auto res_vertices = result.GetVertices();
    ASSERT_EQ(res_vertices.size(), 2u);
    // find mapped host vertices by matching values from original host mapping (we know there are two)
    bool found_r1 = false;
    bool found_r2 = false;
    for (auto vv : res_vertices) {
        const int val = result.GetNodeProperty(vv, &TestNodeProp::value);
        if (val == 10) found_r1 = true;
        if (val == 20) found_r2 = true;
    }
    EXPECT_TRUE(found_r1);
    EXPECT_TRUE(found_r2);

    LOGIFACE_LOG(info, "test: \"RemoveEdgeViaRuleAndTransferProperties\" passed");
}

TEST(RuleEdgeOps, AddEdgeViaRuleAndEdgeProperties) {
    TestGraph g;

    g.AddNode(TestNodeProp{3});
    g.AddNode(TestNodeProp{4});
    EXPECT_EQ(g.GetEdges().size(), 0u);

    using RuleType = GraphLibrary::Rule<TestNodeProp, TestEdgeProp>;
    RuleType rule;
    auto& l = rule.GetLGraph();
    auto& r = rule.GetRGraph();

    // pattern: two nodes, no edge
    auto l1 = l.AddNode(TestNodeProp{3});
    auto l2 = l.AddNode(TestNodeProp{4});

    // replacement: same two nodes, but with an added edge with a distinct property
    auto r1 = r.AddNode(TestNodeProp{3});
    auto r2 = r.AddNode(TestNodeProp{4});
    r.AddEdge(r1, r2, TestEdgeProp{77, TestEdgeType::Y});

    // link nodes so nodes aren't removed and mapping is established
    rule.LinkNodes(l1, r1);
    rule.LinkNodes(l2, r2);

    auto result = g.ApplyRule(rule);
    // expect one new edge added
    EXPECT_EQ(result.GetEdges().size(), 1u);

    // verify the added edge has the expected properties
    bool found = false;
    for (auto e : result.GetEdges()) {
        if (result.GetEdgeProperty(e, &TestEdgeProp::weight) == 77 && result.GetEdgeProperty(e, &TestEdgeProp::type) == TestEdgeType::Y) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);

    LOGIFACE_LOG(info, "test: \"AddEdgeViaRuleAndEdgeProperties\" passed");
}

TEST(GraphProps, PropertyModification) {
    TestGraph g;
    auto v = g.AddNode(TestNodeProp{7});

    // modify node property
    g.GetNodeProperty(v, &TestNodeProp::value) = 42;
    EXPECT_EQ(g.GetNodeProperty(v, &TestNodeProp::value), 42);

    // edge property modification
    auto u = g.AddNode(TestNodeProp{8});
    auto e = g.AddEdge(v, u, TestEdgeProp{11, TestEdgeType::X});
    g.GetEdgeProperty(e, &TestEdgeProp::weight) = 99;
    EXPECT_EQ(g.GetEdgeProperty(e, &TestEdgeProp::weight), 99);

    LOGIFACE_LOG(info, "test: \"PropertyModification\" passed");
}

TEST(RuleConstraintsAndPredicates, ComparatorAndPredicateBehavior) {
    using RuleType = GraphLibrary::Rule<TestNodeProp, TestEdgeProp>;
    RuleType rule;

    // access left/right graphs to create pattern nodes/edges
    auto& l = rule.GetLGraph();
    auto& r = rule.GetRGraph();

    // add nodes to left and right
    auto lnode = l.AddNode(TestNodeProp{10});
    auto rnode = r.AddNode(TestNodeProp{10});

    // add a node constraint comparing the 'value' member for equality
    auto node_constraint = std::make_shared<RuleType::ConcreteNodeConstraint<int>>(
        lnode, rnode, &TestNodeProp::value, &TestNodeProp::value,
        [](const int& a, const int& b){ return a == b; }
    );
    const int idx = rule.AddNodeConstraint(node_constraint);
    EXPECT_GE(idx, 0);
    EXPECT_EQ(rule.GetNodeConstraints().size(), 1u);

    // direct Evaluate via constraint
    EXPECT_TRUE(rule.GetNodeConstraints()[0]->Evaluate(TestNodeProp{10}, TestNodeProp{10}));
    EXPECT_FALSE(rule.GetNodeConstraints()[0]->Evaluate(TestNodeProp{10}, TestNodeProp{11}));

    // remove constraint
    rule.RemoveNodeConstraint(0);
    EXPECT_EQ(rule.GetNodeConstraints().size(), 0u);

    // test edge predicate set/overwrite
    auto ledge = l.AddEdge(lnode, lnode, TestEdgeProp{1, TestEdgeType::X});
    rule.SetEdgePredicate(ledge, [](const TestEdgeProp& p){ return p.weight == 1; });
    EXPECT_TRUE(rule.HasEdgePredicate(ledge));
    auto opt = rule.GetEdgePredicate(ledge);
    ASSERT_TRUE(opt.has_value());
    // we know opt exists because of the has_value check
    EXPECT_TRUE((*opt)(TestEdgeProp{1, TestEdgeType::X})); //NOLINT(bugprone-unchecked-optional-access)

    // overwrite predicate
    rule.SetEdgePredicate(ledge, [](const TestEdgeProp& p){ return p.weight == 2; });
    auto opt2 = rule.GetEdgePredicate(ledge);
    ASSERT_TRUE(opt2.has_value());
    // we know opt2 exists because of the has_value check
    EXPECT_FALSE((*opt2)(TestEdgeProp{1, TestEdgeType::X})); //NOLINT(bugprone-unchecked-optional-access)

    LOGIFACE_LOG(info, "test: \"ComparatorAndPredicateBehavior\" passed");
}

TEST(RuleMapping, LinkNodesEdgesModification) {
    using RuleType = GraphLibrary::Rule<TestNodeProp, TestEdgeProp>;
    RuleType rule;
    GraphLibrary::Graph<TestNodeProp, TestEdgeProp>& l = rule.GetLGraph();
    GraphLibrary::Graph<TestNodeProp, TestEdgeProp>& r = rule.GetRGraph();

    // create two nodes with differing values
    auto lnode = l.AddNode(TestNodeProp{1});
    auto rnode = r.AddNode(TestNodeProp{2});

    // link nodes and verify mapping and modification detection
    rule.LinkNodes(lnode, rnode);
    EXPECT_TRUE(rule.HasNodeMapping(lnode));
    auto mapped = rule.GetMappedNode(lnode);
    ASSERT_TRUE(mapped.has_value());
    // we know mapped value exists because of the has_value check
    EXPECT_EQ(*mapped, rnode); //NOLINT(bugprone-unchecked-optional-access)
    EXPECT_TRUE(rule.NodeWillBeModified(lnode)); // properties differ

    // now link nodes with same property and expect not modified
    auto l2 = l.AddNode(TestNodeProp{7});
    auto r2 = r.AddNode(TestNodeProp{7});
    rule.LinkNodes(l2, r2);
    EXPECT_FALSE(rule.NodeWillBeModified(l2));

    LOGIFACE_LOG(info, "test: \"LinkNodesEdgesModification\" passed");
}
