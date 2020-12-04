#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <tuple>
#include <algorithm>
#include <unordered_set>
#include <regex>

#include "boost/log/core.hpp"
#include "boost/log/expressions.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"
#include "boost/log/utility/setup/console.hpp"
#include "boost/log/utility/setup/file.hpp"

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/boyer_myrvold_planar_test.hpp"
#include "boost/graph/graph_traits.hpp"

#define MAX_DIST 7

typedef size_t node;
typedef std::vector<std::pair<node, node>> edge_list;
typedef std::tuple<edge_list, std::unordered_map<std::string, node>,
            std::unordered_map<node, std::string>> load_result;
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
                current_node_id++;
            }

            search = node_ids.find(elements.at(1));
            if (search == node_ids.end()) {
                node_ids[elements.at(1)] = current_node_id;
                node_ids_rev[current_node_id] = elements.at(1);
                current_node_id++;
            }

            edge_list_out.push_back(std::make_pair(node_ids.at(elements.at(0)),
                                                   node_ids.at(elements.at(1))));
        }
    }

    file_in.close();

    return std::make_tuple(edge_list_out, node_ids, node_ids_rev);
}

void add_node(adjacency_list &adj_list, const node key_node) {
    auto search = adj_list.find(key_node);
    if (search == adj_list.end()) {
        adj_list.insert({key_node, std::vector<node> {}});
    }

}

// currently will panic if the adj_list does not already have the
// nodes as keys. maybe checking here but this will slow things down
void add_edge(adjacency_list &adj_list, const node node_0, const node node_1) {
    auto search = adj_list.find(node_0);
    if (search == adj_list.end()) {
        adj_list.insert({node_0, std::vector<node> {}});
    }

    search = adj_list.find(node_1);
    if (search == adj_list.end()) {
        adj_list.insert({node_1, std::vector<node> {}});
    }

    adj_list.at(node_0).push_back(node_1);
    adj_list.at(node_1).push_back(node_0);
}

// this could also just be accomplished by using unordered_sets instead of
// vecs for adjacents
void dedup(adjacency_list &adj_list) {
    for (auto element : adj_list) {
	std::vector<node> *adjs = &adj_list.at(element.first);
	std::sort((*adjs).begin(), (*adjs).end());
    }

    for (auto element : adj_list) {
	std::vector<node> *adjs = &adj_list.at(element.first);
	auto last = std::unique((*adjs).begin(), (*adjs).end());
	(*adjs).erase(last, (*adjs).end());
    }
}

adjacency_list to_adj_list(const edge_list &edges) {
    adjacency_list adj_list;

    for (std::pair<node, node> edge : edges) {
        node node_0 = edge.first;
        node node_1 = edge.second;

        auto search = adj_list.find(node_0);
        if (search == adj_list.end()) {
            std::vector<node> this_vec {node_1};
            adj_list.insert({node_0, this_vec});
        } else {
            adj_list.at(node_0).push_back(node_1);
        }

        search = adj_list.find(node_1);
        if (search == adj_list.end()) {
            std::vector<node> this_vec {node_0};
            adj_list.insert({node_1, this_vec});
        } else {
            adj_list.at(node_1).push_back(node_0);
        }
    }

    dedup(adj_list);

    return adj_list;
}

// TODO NEED TO DEDUP
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
    const edge_list edges = to_edge_list(adj_list);
    const size_t n_nodes = adj_list.size();
    boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS> boost_graph(n_nodes);

    for (std::pair<node, node> edge : edges) {
        boost::add_edge(edge.first, edge.second, boost_graph);
    }

    return boyer_myrvold_planarity_test(boost_graph);
}

// returns the first node of maximum degree found
node get_max_degree_node(const adjacency_list &adj_list) {
    size_t max_deg = 0;
    // TODO this is bad should probably be properly initialized w/ a value
    node max_deg_node;

    for (auto &[key_node, adjs] : adj_list) {
        if (adjs.size() > max_deg) {
            max_deg = adjs.size();
            max_deg_node = key_node;
        }
    }
    return max_deg_node;
}

// returns the first node of maximum degree found
node get_max_degree_node(const std::unordered_set<node> &node_set, const adjacency_list &adj_list) {
    size_t max_deg = 0;
    // TODO this is bad should probably be properly initialized w/ a value
    node max_deg_node;

    for (node this_node : node_set) {
        std::vector<node> adjs = adj_list.at(this_node);
        if (adj_list.at(this_node).size() > max_deg) {
            max_deg = adj_list.at(this_node).size();
            max_deg_node = this_node;
        }
    }
    return max_deg_node;
}

// use BFS to get all nodes dist hops or more away
// maybe this should be in algo.h
std::unordered_set<node> get_distant_nodes(const node source, const size_t dist,
                                    const adjacency_list &adj_list) {
    std::unordered_set<node> nodes_out;

    std::deque<node> queue;
    std::unordered_set<node> visited;
    size_t current_dist = 0;

    queue.push_back(source);

    while (!queue.empty() && current_dist < MAX_DIST) {
        size_t queue_len = queue.size();
        current_dist++;

        for (size_t _idx = 0; _idx < queue_len; _idx++) {
            node current_node = queue.front();
            queue.pop_front();

            auto search = visited.find(current_node);
            if (search == visited.end()) {
                visited.insert(current_node);

                for (node adj : adj_list.at(current_node)) {
                    if (current_dist >= dist) {
                        nodes_out.insert(adj);
                    }

                    search = visited.find(adj);
                    if (search == visited.end()) {
                        queue.push_back(adj);
                    }
                }
            }
        }
    }

    return nodes_out;
}

// get the intersection of the sets, modifying set_a
void intersection(std::unordered_set<node> &set_a, const std::unordered_set<node> set_b) {
    for (auto iter = set_a.begin(); iter != set_a.end();) {
        auto search = set_b.find(*iter);
        if (search == set_b.end()) {
            iter = set_a.erase(iter);
        } else {
            ++iter;
        }
    }
}

size_t num_edges(const adjacency_list &adj_list) {
    size_t n_edges = 0;

    for (auto &[_k, adjs] : adj_list) {
        n_edges += adjs.size();
    }

    return n_edges;
}

