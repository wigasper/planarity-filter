#include "utils.h"

#include <deque>

#define DIST 2

std::vector<node> node_bfs(const node &start_node, const adjacency_list &adj_list) {
    std::deque<node> queue;
    std::unordered_set<node> visited;

    std::vector<node> component;

    queue.push_back(start_node);

    while (!queue.empty()) {
        node current_node = queue.front();
        queue.pop_front();

        auto search = visited.find(current_node);
        if (search == visited.end()) {
            visited.insert(current_node);
            component.push_back(current_node);

            for (node adj : adj_list.at(current_node)) {
                search = visited.find(adj);
                if (search == visited.end()) {
                    queue.push_back(adj);
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
        node this_node = *unvisited_nodes.begin();
        std::vector<node> component = node_bfs(this_node, adj_list);
        components.push_back(component);

        for (node comp_node : component) {
            unvisited_nodes.erase(comp_node);
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

            for (node node_0 : components.at(current_comp)) {
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

// TODO: also note: returning a vec<node> here, this is basically an
// edge list or matrix of dim 2, this is not entirely clear. doing it
// this way just for speed
std::vector<node> propagate_from_x(const size_t x_node, const adjacency_list &adj_list) {
    //adjacency_list out;
    std::vector<node> out;
    std::unordered_set<node> nu;
    std::deque<node> active {x_node};

    for (auto &[key_node, _adjs] : adj_list) {
        nu.insert(key_node);
    }

    while (!active.empty()) {
    //while (!nu.empty()) {
        const node x = active.front();
        active.pop_front();

        std::vector<node> x_adjs = adj_list.at(x);
        std::unordered_set<node> aux(x_adjs.begin(), x_adjs.end());

        for (node y : x_adjs) {
            auto search = nu.find(y);
            if (search != nu.end()) {
                std::vector <node> y_adjs = adj_list.at(y);

                for (node z : y_adjs) {
                    search = nu.find(z);
                    auto aux_search = aux.find(z);
                    if (search != nu.end() && aux_search != nu.end()) {

                        // add the edges to out, again, this is not super
                        // clear right now and should be cleaned up. possibly
                        // use matrix abstraction
                        out.push_back(x);
                        out.push_back(y);
                        out.push_back(x);
                        out.push_back(z);
                        out.push_back(y);
                        out.push_back(z);

                        active.push_front(y);
                        active.push_front(z);

                        nu.erase(y);
                        nu.erase(z);

                        aux.erase(y);
                        aux.erase(z);

                        break;
                    }
                }
            }
        }
    }

    return out;
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

    const std::vector<node> active = init_active_set(adj_list);

    #pragma omp parallel for
    for (node x : active) {
        const std::vector<node> edges = propagate_from_x(x, adj_list);
        for (size_t idx; idx < edges.size(); idx += 2) {
            add_edge(out, edges.at(idx), edges.at(idx + 1));
        }
    }

    dedup(out);

    std::vector<std::vector<node>> components = get_components(out);

    if (components.size() > 1) {
        connect_components(out, components);
    }

    return out;
}