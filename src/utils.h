#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <tuple>
#include <algorithm>

#include "boost/log/core.hpp"
#include "boost/log/expressions.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"
#include "boost/log/utility/setup/console.hpp"
#include "boost/log/utility/setup/file.hpp"

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/boyer_myrvold_planar_test.hpp"
#include "boost/graph/graph_traits.hpp"

typedef size_t node;
typedef std::vector<std::pair<node, node>> edge_list;
typedef std::tuple<edge_list, std::unordered_map<std::string, int>,
            std::unordered_map<int, std::string>> load_result;
typedef std::unordered_map<node, std::vector<node>> adjacency_list;

void log_init() {
    std::string log_format = "[%TimeStamp%] [%Severity%] [%Message%]";
    std::string log_path = "planarity_filter.log";
    boost::log::add_file_log(log_path, boost::log::keywords::format = log_format,
                             boost::log::keywords::open_mode = std::ios_base::app);
    boost::log::add_console_log(std::cout, boost::log::keywords::format = log_format);
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    boost::log::add_common_attributes();
}

// trims the whitespace from a string
std::string trim_whitespace(std::string a_string) {
    size_t first = a_string.find_first_not_of(' ');
    if (first == std::string::npos) {
        return "";
    }
    size_t last = a_string.find_last_not_of(' ');
    return a_string.substr(first, (last - first + 1));
}

// parses a single line of a whitespace delimited input
// file
std::vector<std::string> parse_line(std::string line) {
    std::vector<std::string> vec_out;
    // this is a temporary delim, will replace any and all consecutive
    // whitespace chars with this
    std::string delim = "|";

    std::regex delim_regex("\\s+");
    line = std::regex_replace(line, delim_regex, delim);

    size_t index = 0;

    std::string element;
    std::string trimmed_element;

    while ((index = line.find(delim)) != std::string::npos) {
        element = line.substr(0, index);

        trimmed_element = trim_whitespace(element);
        if (!trimmed_element.empty()) {
            vec_out.push_back(trim_whitespace(element));
        }

        line.erase(0, index + delim.length());
    }

    trimmed_element = trim_whitespace(line);
    if (!trimmed_element.empty()) {
        vec_out.push_back(line);
    }

    return vec_out;
}

load_result load_edge_list(const std::string file_path) {
    edge_list edge_list_out;
    std::unordered_map<std::string, node> node_ids;
    std::unordered_map<node, std::string> node_ids_rev;

    std::fstream file_in;

    file_in.open(file_path, std::ios::in);

    std::string line;

    size_t current_node_id = 0;
    while (getline(file_in, line)) {
        std::vector <std::string> elements = parse_line(line);

        if (elements.size() > 1) {
            auto search = node_ids.find(elements.at(0));
            if (search == node_ids.end()) {
                node_ids[elements.at(0)] = current_node_id;
                node_ids_rev[current_node_id] = elements.at(0);
                current_node_id++
            }

            search = node_ids.find(elements.at(1));
            if (search == node_ids.end()) {
                node_ids[elements.at(1)] = current_node_id;
                node_ids_rev[current_node_id] = elements.at(1);
                current_node_id++
            }

            edge_list_out.push_back(std::make_pair(elements.at(0), elements_at(1)));
        }
    }

    file_in.close()

    return std::make_tuple(edge_list_out, node_ids, node_ids_rev);
}

void add_node(adjacency_list &adj_list, const node key_node) {
    adj_list.insert({key_node, std::vector<node>});
}

// currently will panic if the adj_list does not already have the
// nodes as keys. maybe checking here but this will slow things down
void add_edge(adjacency_list &adj_list, const std::pair<node, node> edge) {
    node node_0 = std::get<0>(edge);
    node node_1 = std::get<1>(edge);

    adj_list.at(node_0).push_back(node_1);
    adj_list.at(node_1).push_back(node_0);
}

// this could also just be accomplished by using unordered_sets instead of
// vecs for adjacents
void dedup(adjacency_list &adj_list) {
    for (auto &[key_node, adjs] : adj_list) {
        adjs.sort();
    }

    for (auto &[key_node, adjs] : adj_list) {
        adjs.unique();
    }
}

adjacency_list to_adj_list(const edge_list &edges) {
    adjacency_list adj_list;

    for (std::pair<node, node> edge : edges) {
        node node_0 = edge.first;
        node node_1 = edge.second;

        auto search = adj_list.find(node_0);
        if (seach == adj_list.end()) {
            std::vector<node> this_vec {node_1};
            adj_list.insert(node_0, this_vec);
        }

        search = adj_list.find(node_1);
        if (seach == adj_list.end()) {
            std::vector<node> this_vec {node_0};
            adj_list.insert(node_1, this_vec);
        }
    }

    dedup(adj_list);

    return adj_list;
}

edge_list to_edge_list(const adjacency_list &adj_list) {
    edge_list edges_out;

    for (auto &[key_node, adjs] : adj_list) {
        for (node adj : adjs) {
            edges_out.push_back(std::make_pair(key_node, adj));
        }
    }

    return edges_out;
}

bool boyer_myrvold_test(const adjacency_list &adj_list) {
    edge_list edges = to_edge_list(adj_list);
    size_t n_nodes = adj_list.size();
    boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS> boost_graph(n_nodes);

    for (std::pair<node, node> edge : edges) {
        boost::add_edge(edge.first, edge.second, boost_graph);
    }

    return boyer_myrvold_planarity_test(boost_graph);
}