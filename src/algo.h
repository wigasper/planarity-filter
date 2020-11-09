#include "utils.h"

#include <deque>
#include <unordered_set>

#define DIST 2

std::vector<node> node_bfs(const node &start_node, const adjacency_list &adj_list) {
    std::deque<node> queue;
    std::unordered_set<node> visited;

    std::vector<node> component;

    queue.push_back(start_node);

    while (!queue.empty()) {
        node current_node = queue.front();
        queue.pop_front();

        auto search = visited.find(current_node)
        if (search == visited.end()) {
            visited.insert(current_node);
            component_push_back(current_node);

            for (node adj : adj_list.at(current_node)) {
                search = visited.find(adj);
                if (search == visited.end()) {
                    queue.push_back();
                }
            }
        }
    }

    return component;
}

std::vector<std::vector<node>> get_components(const adjacency_list adj_list) {
    std::unordered_set<node> unvisited_nodes;

    for (auto &[key_node, _adjs] : adj_list) {
        unvisited_nodes.insert(key_node);
    }

    std::vector<std::vector<node>> components;

    while (!unvisited_nodes.empty()) {
        node this_node = unvisited_nodes.extract(unvisited_nodes.begin());
        std::vector<node> component = node_bfs(this_node, adj_list);
        components.push_back(component);

        for (node comp_node : component) {
            auto posit = unvisited_nodes.find(comp_node);
            unvisited_nodes.erase(posit);
        }
    }

    return components;
}

// connects components with 1 edge, if possible
void connect_components(adjacency_list &adj_list, const std::vector<std::vector<node>> &components) {
    std::unordered_set<node> unvisited;
    std::unordered_map<node, size_t> node_to_comp;

    for (size_t idx = 0; idx < components.size(); idx++) {
        for (node this_node : components.at(idx)) {
            node_to_comp.insert({this_node, idx});
        }
        unvisited.insert(idx);
    }

    std::deque<size_t> queue;
    edge_list edges;
    queue.push_back(0);

    while (!queue.empty()) {
        size_t current_comp = queue.front();
        queue.pop_front();

        auto search = unvisited.find(current_comp);
        if (search != unvisited.end()) {
            unvisited.erase(search);

            for (node node_0 : components.at(current_component)) {
                std::vector<node> adjs = adj_list.at(node_0);

                for (node node_1 : adjs) {
                    size_t node_1_comp = node_to_comp.at(node_1);

                    search = unvisited.find(node_1_comp);
                    if (search != unvisited.end() && current_comp != node_1_comp) {
                        edges.push_back(std::make_pair(node_0, node_1));
                        queue.push_back(node_1_comp);
                    }
                }
            }
        }
    }

    for (std::pair<node, node> edge : edges) {
        add_edge(adj_list, edge.first, edge.second);
    }
}

void propagate_from_x() {

}

// TODO: test to see if vecs are faster than hash sets here
std::vector<node> init_active_set(const adjacency_list &adj_list) {
    const node init_node = get_max_degree_node(adj_list);
    std::vector<node> active_set {init_node};

    std::unordered_set<node> putative_nodes = get_distant_nodes(init_node, DIST, adj_list);

    while (putative_nodes.size() > 0) {
        // select next node
        // NOTE: may pick this differently to save time
        node next_node = get_max_degree_node(putative_nodes, adj_list);
        active_set.push_back(next_node);
        std::unordered_set<node> next_node_dists = get_distant_nodes(next_node, DIST, adj_list);

        intersection(putative_nodes, next_node_dists);
    }

    return active_set;
}

adjacency_list algo_routine(const adjacency_list &adj_list) {
    adjacency_list out;

    for (auto &[key_node, _adjs] : adj_list) {
        add_node(out, key_node);
    }
}