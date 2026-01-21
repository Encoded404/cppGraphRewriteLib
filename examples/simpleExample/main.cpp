#include <GraphLibrary/GraphLibrary.hpp>
#include <GraphLibrary/Rule.hpp>

#include <iostream>
#include <cstdint>
#include <string>

using namespace GraphLibrary;

enum class NodeType : uint8_t
{
    ENTRANCE = 0,
    ROOM = 1,
    LARGE_ROOM = 2,
    EXIT = 3
};

enum class EdgeType : uint8_t
{
    NONE = 0, // this is not needed but helps with understanding. should be used when a custom edge predicate is used
    DOOR = 1,
    HALLWAY = 2,
    LARGEHALLWAY = 3
};

static std::string EnumToString(NodeType type)
{
    switch (type)
    {
        case NodeType::ENTRANCE: return "ENTRANCE";
        case NodeType::ROOM: return "ROOM";
        case NodeType::LARGE_ROOM: return "LARGE_ROOM";
        case NodeType::EXIT: return "EXIT";
        default: return "UNKNOWN";
    }
}

static std::string EnumToString(EdgeType type)
{
    switch (type)
    {
        case EdgeType::DOOR: return "DOOR";
        case EdgeType::HALLWAY: return "HALLWAY";
        case EdgeType::LARGEHALLWAY: return "LARGEHALLWAY";
        default: return "UNKNOWN";
    }
}

struct NodeProp
{
    NodeType type{};
    bool operator==(const NodeProp& o) const { return type == o.type; }
};

struct EdgeProp
{
    EdgeType type{};
    bool operator==(const EdgeProp& o) const { return type == o.type; }
};

static void PrintGraph(Graph<NodeProp, EdgeProp>& g)
{
    std::cout << "Graph nodes:\n";
    for (auto v : g.GetVertices())
    {
        std::cout << " node type = " << EnumToString(g.GetNodeProperty(v, &NodeProp::type)) << "\n";
    }
    std::cout << "Graph edges:\n";
    for (auto e : g.GetEdges())
    {
        auto src = boost::source(e, g.GetInternalGraph());
        auto tgt = boost::target(e, g.GetInternalGraph());
        std::cout << " edge from " << EnumToString(g.GetNodeProperty(src, &NodeProp::type))
                  << " to " << EnumToString(g.GetNodeProperty(tgt, &NodeProp::type))
                  << " type = " << EnumToString(g.GetEdgeProperty(e, &EdgeProp::type)) << "\n";
    }
}

int main()
{
    Graph<NodeProp, EdgeProp> g;

    using Vertex = Graph<NodeProp, EdgeProp>::Vertex;
    using Edge = Graph<NodeProp, EdgeProp>::Edge;

    Vertex v1 = g.AddNode(NodeProp{NodeType::ENTRANCE});
    Vertex v2 = g.AddNode(NodeProp{NodeType::ROOM});
    Vertex v3 = g.AddNode(NodeProp{NodeType::LARGE_ROOM});
    Vertex v4 = g.AddNode(NodeProp{NodeType::EXIT});

    g.AddEdge(v1, v2, EdgeProp{EdgeType::DOOR});
    g.AddEdge(v2, v3, EdgeProp{EdgeType::DOOR});
    g.AddEdge(v3, v4, EdgeProp{EdgeType::HALLWAY});

    std::cout << "Initial graph:\n";
    PrintGraph(g);

    // Rule 1: if a "room" and a "large room" are connected by a "door", add another "room" connected to the "large room"
    auto rule1 = Rule<NodeProp, EdgeProp>();
    Graph<NodeProp, EdgeProp>& left_graph_1 = rule1.GetLGraph();
    Graph<NodeProp, EdgeProp>& right_graph_1 = rule1.GetRGraph();
    const Vertex left_node_1_1 = left_graph_1.AddNode(NodeProp{NodeType::ROOM});
    const Vertex left_node_1_2 = left_graph_1.AddNode(NodeProp{NodeType::LARGE_ROOM});

    const Edge left_edge_1_1 = left_graph_1.AddEdge(left_node_1_1, left_node_1_2, EdgeProp{EdgeType::DOOR});
    
    const Vertex right_node_1_1 = right_graph_1.AddNode(NodeProp{NodeType::ROOM});
    const Vertex right_node_1_2 = right_graph_1.AddNode(NodeProp{NodeType::LARGE_ROOM});

    const Vertex right_node_1_3 = right_graph_1.AddNode(NodeProp{NodeType::ROOM}); // add new room
    const Edge right_edge_1_1 = right_graph_1.AddEdge(right_node_1_1, right_node_1_2, EdgeProp{EdgeType::DOOR}); // existing connection
    right_graph_1.AddEdge(right_node_1_3, right_node_1_2, EdgeProp{EdgeType::DOOR}); // connect new room to large room
    right_graph_1.AddEdge(right_node_1_1, right_node_1_3, EdgeProp{EdgeType::DOOR}); // connect old room to new room
    rule1.LinkNodes(left_node_1_1, right_node_1_1); // map existing nodes
    rule1.LinkNodes(left_node_1_2, right_node_1_2);
    rule1.LinkEdges(left_edge_1_1, right_edge_1_1);

    // Rule 2: if 2 "room" are connected with a "door", change the edge to a "hallway"
    auto rule2 = Rule<NodeProp, EdgeProp>();
    Graph<NodeProp, EdgeProp>& left_graph_2 = rule2.GetLGraph();
    Graph<NodeProp, EdgeProp>& right_graph_2 = rule2.GetRGraph();
    const Vertex left_node_2_1 = left_graph_2.AddNode(NodeProp{NodeType::ROOM});
    const Vertex left_node_2_2 = left_graph_2.AddNode(NodeProp{NodeType::ROOM});
    const Edge left_edge_2_1 = left_graph_2.AddEdge(left_node_2_1, left_node_2_2, EdgeProp{EdgeType::DOOR});

    const Vertex right_node_2_1 = right_graph_2.AddNode(NodeProp{NodeType::ROOM});
    const Vertex right_node_2_2 = right_graph_2.AddNode(NodeProp{NodeType::ROOM});
    const Edge right_edge_2_1 = right_graph_2.AddEdge(right_node_2_1, right_node_2_2, EdgeProp{EdgeType::HALLWAY}); // changed edge type
    rule2.LinkNodes(left_node_2_1, right_node_2_1); // map existing nodes
    rule2.LinkNodes(left_node_2_2, right_node_2_2);
    rule2.LinkEdges(left_edge_2_1, right_edge_2_1);

    // Rule 3: if a "large room" is connected to a exit by any means add a "room" in between
    auto rule3 = Rule<NodeProp, EdgeProp>();
    Graph<NodeProp, EdgeProp>& left_graph_3 = rule3.GetLGraph();
    Graph<NodeProp, EdgeProp>& right_graph_3 = rule3.GetRGraph();
    const Vertex left_node_3_1 = left_graph_3.AddNode(NodeProp{NodeType::LARGE_ROOM});
    const Vertex left_node_3_2 = left_graph_3.AddNode(NodeProp{NodeType::EXIT});
    const Edge left_edge_3_1 = left_graph_3.AddEdge(left_node_3_1, left_node_3_2, EdgeProp{EdgeType::HALLWAY});
    rule3.SetEdgePredicate(left_edge_3_1, [](const EdgeProp& prop) {
        return prop.type == EdgeType::HALLWAY || prop.type == EdgeType::LARGEHALLWAY || prop.type == EdgeType::DOOR;
    }); // match any edge type

    const Vertex right_node_3_1 = right_graph_3.AddNode(NodeProp{NodeType::LARGE_ROOM});
    const Vertex right_node_3_2 = right_graph_3.AddNode(NodeProp{NodeType::EXIT});

    const Vertex right_node_3_3 = right_graph_3.AddNode(NodeProp{NodeType::ROOM}); // new room
    const Edge right_edge_3_1 = right_graph_3.AddEdge(right_node_3_1, right_node_3_3, EdgeProp{EdgeType::HALLWAY}); // connect large room to new room
    right_graph_3.AddEdge(right_node_3_3, right_node_3_2, EdgeProp{EdgeType::DOOR}); // connect new room to exit
    rule3.LinkNodes(left_node_3_1, right_node_3_1); // map existing nodes
    rule3.LinkNodes(left_node_3_2, right_node_3_2);
    rule3.LinkEdges(left_edge_3_1, right_edge_3_1); // map existing edge to first new edge

    RuleSet<NodeProp, EdgeProp> ruleset;
    ruleset.AddRule(rule1);
    ruleset.AddRule(rule2);
    ruleset.AddRule(rule3);

    // Apply rules one by one and print intermediate graphs for debugging
    std::cout << "After applying rule1:\n";
    auto g_after_r1 = g.ApplyRule(rule1);
    PrintGraph(g_after_r1);

    std::cout << "After applying rule2:\n";
    auto g_after_r2 = g_after_r1.ApplyRule(rule2);
    PrintGraph(g_after_r2);

    std::cout << "After applying rule3:\n";
    auto g_after_r3 = g_after_r2.ApplyRule(rule3);
    PrintGraph(g_after_r3);

    std::cout << "After applying rules (set):\n";
    auto out = g.ApplyRuleSet(ruleset);
    PrintGraph(out);

    return 0;
}
