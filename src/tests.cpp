#include "algo.h"
#include <gtest/gtest.h>

TEST(trim_whitespace_tests, trim_0) {
    std::string a = "  oh hello    ";
    std::string expected = "oh hello";
    ASSERT_EQ(trim_whitespace(a), expected);
}

TEST(parse_line_tests, parse_0) {
    std::string line = "a   b	c   d	e";
    std::vector<std::string> expected {"a", "b", "c", "d", "e"};
    ASSERT_EQ(parse_line(line), expected);
}

TEST(add_node_tests, add_node_0) {
    adjacency_list g;
    add_node(g, 3, 5);
    add_node(g, 10, 5);

    auto three_search = g.find(3);
    ASSERT_NE(three_search, g.end());

    auto ten_search = g.find(10);
    ASSERT_NE(ten_search, g.end());
}

TEST(add_edge_tests, add_edge_0) {
    adjacency_list g;
    add_edge(g, 3, 5);
    auto three_search = g.find(3);
    ASSERT_NE(three_search, g.end());

    auto five_search = g.find(5);
    ASSERT_NE(five_search, g.end());

    ASSERT_EQ(g.at(3).size(), 1);
    ASSERT_EQ(g.at(5).size(), 1);

    auto three_edge_search = std::find(g.at(3).begin(), g.at(3).end(), 5);
    ASSERT_NE(three_edge_search, g.at(3).end());

    auto five_edge_search = std::find(g.at(5).begin(), g.at(5).end(), 3);
    ASSERT_NE(five_edge_search, g.at(5).end());
}

TEST(to_adj_list_tests, no_self_loops_0) {
    edge_list e;
    e.push_back(std::make_pair(0, 1));
    e.push_back(std::make_pair(0, 0));

    adjacency_list a = to_adj_list(e);

    ASSERT_EQ(a.at(0).size(), 1);

    auto self_loop_search = std::find(a.at(0).begin(), a.at(0).end(), 0);
    ASSERT_EQ(self_loop_search, a.at(0).end());
}

TEST(to_adj_list_tests, to_adj_list_0) {
    edge_list e;
    e.push_back(std::make_pair(0, 1));
    e.push_back(std::make_pair(0, 2));
    e.push_back(std::make_pair(1, 2));
    e.push_back(std::make_pair(3, 5));
    e.push_back(std::make_pair(3, 6));

    adjacency_list a = to_adj_list(e);

    ASSERT_EQ(a.size(), 6);
    ASSERT_EQ(a.at(0).size(), 2);
    ASSERT_EQ(a.at(1).size(), 2);
    ASSERT_EQ(a.at(3).size(), 2);
    ASSERT_EQ(a.at(5).size(), 1);
    ASSERT_EQ(a.at(6).size(), 1);

    auto zero_edge_search = std::find(a.at(0).begin(), a.at(0).end(), 1);
    ASSERT_NE(zero_edge_search, a.at(0).end());

    auto five_edge_search = std::find(a.at(5).begin(), a.at(5).end(), 3);
    ASSERT_NE(five_edge_search, a.at(5).end());
}

TEST(to_edge_list_tests, to_edge_list_0) {
    adjacency_list g;
    add_edge(g, 0, 1);
    add_edge(g, 0, 2);
    add_edge(g, 1, 2);

    edge_list edges = to_edge_list(g);

    ASSERT_EQ(edges.size(), 3);
}

TEST(boyer_myrvold_tests, boyer_myrvold_0) {
    adjacency_list g;
    add_edge(g, 0, 1);
    add_edge(g, 1, 2);
    add_edge(g, 0, 2);

    ASSERT_EQ(boyer_myrvold_test(g), true);
}

TEST(boyer_myrvold_tests, boyer_myrvold_1) {
    adjacency_list g;
    add_edge(g, 0, 1);
    add_edge(g, 0, 2);
    add_edge(g, 0, 3);
    add_edge(g, 0, 4);
    add_edge(g, 1, 2);
    add_edge(g, 1, 3);
    add_edge(g, 1, 4);
    add_edge(g, 2, 3);
    add_edge(g, 2, 4);
    add_edge(g, 3, 4);

    ASSERT_EQ(boyer_myrvold_test(g), false);
}

TEST(get_max_deg_node_tests, get_max_0) {
    adjacency_list g;
    add_edge(g, 0, 1);
    add_edge(g, 0, 2);
    add_edge(g, 0, 3);
    add_edge(g, 1, 2);
    add_edge(g, 3, 5);

    ASSERT_EQ(get_max_degree_node(g), 0);
}

TEST(num_edges_tests, num_edges_test_0) { 
    adjacency_list g;
    add_edge(g, 0, 1);
    add_edge(g, 0, 2);
    add_edge(g, 2, 9);
    add_edge(g, 3, 10);
    add_edge(g, 5, 6);
    ASSERT_EQ(num_edges(g), 5);
}

TEST(dedup_tests, dedup_0) {
    adjacency_list g;
    add_edge(g, 0, 1);
    add_edge(g, 0, 1);
    add_edge(g, 1, 0);
    add_edge(g, 3, 2);
    add_edge(g, 2, 3);
    add_edge(g, 5, 6);
    dedup(g);
    ASSERT_EQ(num_edges(g), 3);   
}

TEST(get_components_tests, get_comps_0) {
    adjacency_list g;

    add_edge(g, 0, 1);
    add_edge(g, 0, 2);
    add_edge(g, 1, 2);
    add_edge(g, 3, 4);
    add_edge(g, 3, 5);
    add_edge(g, 4, 5);
    add_edge(g, 6, 7);
    add_edge(g, 10, 11);
    add_edge(g, 11, 12);
    
    auto comps = get_components(g);
    ASSERT_EQ(comps.size(), 4);
}

TEST(connect_comp_tests, connect_comp_0) {
    adjacency_list g;
    adjacency_list g_og;

    add_edge(g, 0, 1);
    add_edge(g, 0, 2);
    add_edge(g, 1, 2);
    add_edge(g, 3, 4);
    add_edge(g, 3, 5);
    add_edge(g, 4, 5);

    add_edge(g_og, 0, 1);
    add_edge(g_og, 0, 2);
    add_edge(g_og, 1, 2);
    add_edge(g_og, 2, 3);
    add_edge(g_og, 3, 4);
    add_edge(g_og, 3, 5);
    add_edge(g_og, 4, 5);

    auto comps = get_components(g);
    connect_components(g, comps, g_og);

    auto two_search = std::find(g.at(2).begin(), g.at(2).end(), 3);
    ASSERT_NE(two_search, g.at(2).end());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
