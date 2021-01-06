#include "utils.h"

#include <deque>
#include <random>

#define DIST 4
#define MAX_ACTIVE_SIZE 5000

enum visited_state { UNVISITED, VISITED, QUEUED };

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

// connects components with 1 edge from the original graph, if possible
// TODO fix this
void connect_components(adjacency_list &adj_list, const std::vector<std::vector<node>> &components,
                        const adjacency_list &original_graph) {
    std::unordered_map<size_t, visited_state> state;
    std::unordered_map<node, size_t> node_to_comp;

    for (size_t idx = 0; idx < components.size(); idx++) {
        for (node this_node : components.at(idx)) {
            node_to_comp.insert({this_node, idx});
        }
        state.insert({idx, UNVISITED});
    }

    std::deque<size_t> queue;
    edge_list edges;
    queue.push_back(0);

    while (!queue.empty()) {
        size_t current_comp = queue.front();
        queue.pop_front();

        if (state.at(current_comp) != VISITED) {
            state.at(current_comp) = VISITED;

            for (node node_0 : components.at(current_comp)) {
                std::vector<node> adjs = original_graph.at(node_0);

                for (node node_1 : adjs) {
                    size_t node_1_comp = node_to_comp.at(node_1);

                    if (state.at(node_1_comp) == UNVISITED && current_comp != node_1_comp) {
                        state.at(node_1_comp) = QUEUED;
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

void add_houses(const node x, const adjacency_list &adj_list,
	std::unordered_set<node> &nu, std::vector<node> &out, std::deque<node> &active) {
    
    std::vector<node> x_adjs = adj_list.at(x);
    std::unordered_set<node> aux(x_adjs.begin(), x_adjs.end());
     for (node y : x_adjs) {
	auto search = nu.find(y);
	if (search != nu.end()) {
	    std::vector<node> y_adjs = adj_list.at(y);
	    
	    bool found = false;

	    for (node z : y_adjs) {
		search = nu.find(z);
		auto aux_search = aux.find(z);
		if (search != nu.end() && aux_search != aux.end()) {
		    std::vector<node> z_adjs = adj_list.at(z);
		    for (node w : z_adjs) {
			search = nu.find(w);
			aux_search = aux.find(w);
			
			if (search != nu.end() && aux_search != aux.end()) {
			    for (node v : y_adjs) {
				if (v != z && v != w) {
				    search = nu.find(v);
				    aux_search = aux.find(v);
				    if (search != nu.end() && aux_search != aux.end()) {
					out.push_back(x);
					out.push_back(y);
					out.push_back(x);
					out.push_back(z);
					out.push_back(y);
					out.push_back(z);
					out.push_back(x);
					out.push_back(w);
					out.push_back(z);
					out.push_back(w);
					out.push_back(y);
					out.push_back(v);
					out.push_back(x);
					out.push_back(v);

					active.push_front(y);
					active.push_front(z);
					active.push_front(w);
					active.push_front(v);

					nu.erase(y);
					nu.erase(z);
					nu.erase(w);
					nu.erase(v);

					aux.erase(y);
					aux.erase(z);
					aux.erase(w);
					aux.erase(v);

					found = true;

					break;
				    }
				}
			    }
			}
			if (found) {break;}
		    } 		
		}
		if (found) {break;}
	    }
	}
    }
}

void add_diamonds_alt(const node x, const adjacency_list &adj_list,
	std::unordered_set<node> &nu, std::vector<node> &out, std::deque<node> &active) {
    
    std::vector<node> x_adjs = adj_list.at(x);
    std::unordered_set<node> aux(x_adjs.begin(), x_adjs.end());
     for (node y : x_adjs) {
	auto search = nu.find(y);
	if (search != nu.end()) {
	    std::vector<node> y_adjs = adj_list.at(y);
	    
	    bool found = false;

	    for (node z : y_adjs) {
		search = nu.find(z);
		auto aux_search = aux.find(z);
		if (search != nu.end() && aux_search != aux.end()) {
		    std::vector<node> z_adjs = adj_list.at(z);
		    for (node w : z_adjs) {
			search = nu.find(w);
			auto aux_search = std::find(y_adjs.begin(), y_adjs.end(), w);
			if (search != nu.end() && aux_search != y_adjs.end()) {
			    out.push_back(x);
			    out.push_back(y);
			    out.push_back(x);
			    out.push_back(z);
			    out.push_back(y);
			    out.push_back(z);
			    out.push_back(x);
			    out.push_back(w);
			    out.push_back(z);
			    out.push_back(w);

			    active.push_front(y);
			    active.push_front(z);
			    active.push_front(w);

			    nu.erase(y);
			    nu.erase(z);
			    nu.erase(w);

			    aux.erase(y);
			    aux.erase(z);
			    aux.erase(w);

			    found = true;

			    break;
			}	    
		    } 		
		}
		if (found) {break;}
	    }
	}
    }
} 

void add_diamonds(const node x, const adjacency_list &adj_list,
	std::unordered_set<node> &nu, std::vector<node> &out, std::deque<node> &active) {
    
    std::vector<node> x_adjs = adj_list.at(x);
    std::unordered_set<node> aux(x_adjs.begin(), x_adjs.end());
     for (node y : x_adjs) {
	auto search = nu.find(y);
	if (search != nu.end()) {
	    std::vector<node> y_adjs = adj_list.at(y);
	    
	    bool found = false;

	    for (node z : y_adjs) {
		search = nu.find(z);
		auto aux_search = aux.find(z);
		if (search != nu.end() && aux_search != aux.end()) {
		    std::vector<node> z_adjs = adj_list.at(z);
		    for (node w : z_adjs) {
			search = nu.find(w);
			aux_search = aux.find(w);
			if (search != nu.end() && aux_search != aux.end()) {
			    out.push_back(x);
			    out.push_back(y);
			    out.push_back(x);
			    out.push_back(z);
			    out.push_back(y);
			    out.push_back(z);
			    out.push_back(x);
			    out.push_back(w);
			    out.push_back(z);
			    out.push_back(w);

			    active.push_front(y);
			    active.push_front(z);
			    active.push_front(w);

			    nu.erase(y);
			    nu.erase(z);
			    nu.erase(w);

			    aux.erase(y);
			    aux.erase(z);
			    aux.erase(w);

			    found = true;

			    break;
			}	    
		    } 		
		}
		if (found) {break;}
	    }
	}
    }
}  

void add_triangles(const node x, const adjacency_list &adj_list, 
    std::unordered_set<node> &nu, std::vector<node> &out, std::deque<node> &active) {
    std::vector<node> x_adjs = adj_list.at(x);
    std::unordered_set<node> aux(x_adjs.begin(), x_adjs.end());

    for (node y : x_adjs) {
	auto search = nu.find(y);
	if (search != nu.end()) {
	    std::vector<node> y_adjs = adj_list.at(y);

	    for (node z : y_adjs) {
		search = nu.find(z);
		auto aux_search = aux.find(z);
		if (search != nu.end() && aux_search != aux.end()) {

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

// TODO: also note: returning a vec<node> here, this is basically an
// edge list or matrix of dim 2, this is not entirely clear. doing it
// this way just for speed
std::vector<node> propagate_from_x(const size_t x_node, const adjacency_list &adj_list) {
    std::vector<node> out;
    std::unordered_set<node> nu;
    std::deque<node> active {x_node};

    for (auto &[key_node, _adjs] : adj_list) {
        nu.insert(key_node);
    }

    while (!nu.empty()) {
	if (active.empty()) {
	    node temp = *nu.begin();
	    active.push_front(temp);
	    nu.erase(temp);	    
	}

        const node x = active.front();
        active.pop_front();

	add_triangles(x, adj_list, nu, out, active);

    }

    return out;
}

std::vector<adjacency_list> partition_nodes(const adjacency_list &adj_list, 
	const size_t num_partitions) {
    std::vector<adjacency_list> partitions;

    std::unordered_set<node> node_set;
    size_t num_nodes = node_set.size();

    for (auto &[key_node, _adjs] : adj_list) {
	node_set.insert(key_node);
    }
    
    std::mt19937 generator(42);

    for (size_t _ = 0; _ < num_partitions; _++) {
	std::uniform_int_distribution<> distribution(0, node_set.size() - 1);
	auto iter = node_set.begin();
	std::advance(iter, distribution(generator));

	adjacency_list new_adj_list;
	add_node(new_adj_list, *iter);
	node_set.erase(iter);
	partitions.push_back(new_adj_list);
    } 

    // initialize partitions with high degree nodes
    // 
    // TODO it would be even better to initialize w/ distant high 
    // degree nodes
/*    for (size_t _n = 0; _n < num_partitions; _n++) {
	node max_deg_node = get_max_degree_node(node_set, adj_list);
	adjacency_list new_adj_list;
	add_node(new_adj_list, max_deg_node);
	node_set.erase(max_deg_node);
	partitions.push_back(new_adj_list);
    }*/
    
    // add neighbors first
    for (size_t idx = 0; idx < partitions.size(); idx++) {
	node node_0 = partitions.at(idx).begin()->first;
	for (node node_1 : adj_list.at(node_0)) {
	    auto search = node_set.find(node_1);
	    if (search != node_set.end()) {
		add_node(partitions.at(idx), node_1);
		//add_edge(partitions.at(idx), node_0, node_1);
		
		// neighbors that are in the partition need their
		// edges
		for (node adj : adj_list.at(node_1)) {
		    auto search = partitions.at(idx).find(adj);
		    if (search != partitions.at(idx).end()) {
			add_edge(partitions.at(idx), node_1, adj);
		    }
		}
		node_set.erase(node_1);
	    }	    
	}
    }
    
    // start adding nodes to partitions with BFS
    for (size_t idx = 0; idx < partitions.size(); idx++) {
	size_t num_nodes_added = 0;
	
	std::deque<node> queue;
	std::unordered_set<node> visited;
	queue.push_back(partitions.at(idx).begin()->first);

	while (!queue.empty() && num_nodes_added < num_nodes / num_partitions) {
	    const size_t queue_len = queue.size();

	    for (size_t _ = 0; _ < queue_len; _++) {
		node current_node = queue.front();
		queue.pop_front();

		auto search = visited.find(current_node);
		if (search == visited.end()) {
		    visited.insert(current_node);
		    std::vector<node> adjs = adj_list.at(current_node);
		    for (node node_0 : adjs) {
			auto search = node_set.find(node_0);
			if (search != node_set.end()) {
			    add_node(partitions.at(idx), node_0);
			    
			    search = visited.find(node_0);
			    if (search == visited.end()) {
				queue.push_back(node_0);
			    }

			    num_nodes_added++;
			    // for each neighbor from the original graph,
			    // if the neighbor is in the partition, edges should
			    // be added
			    for (node node_1 : adj_list.at(node_0)) {
				auto search = partitions.at(idx).find(node_1);
				if (search != partitions.at(idx).end()) {
				    add_edge(partitions.at(idx), node_0, node_1);
				}
			    }
			    node_set.erase(node_0);

			}
		    }
		}
	    }
	}
    }
    
    size_t idx = 0;
    while (!node_set.empty()) {
	if (idx >= partitions.size()) {
	    idx = 0;
	}

	node this_node = *node_set.begin();
	add_node(partitions.at(idx), this_node);
	for (node node_1 : adj_list.at(this_node)) {
	    auto search = partitions.at(idx).find(node_1);
	    if (search != partitions.at(idx).end()) {
		add_edge(partitions.at(idx), this_node, node_1);
	    }
	}
	node_set.erase(this_node);
	idx++;
    }

    return partitions;
} 

adjacency_list algo_routine(const adjacency_list &adj_list, const int threads) {
    adjacency_list out;

    for (auto &[key_node, _adjs] : adj_list) {
        add_node(out, key_node);
    }
    std::vector<adjacency_list> partitions = partition_nodes(adj_list, threads);

#pragma omp parallel for num_threads(threads)
    for (adjacency_list partition : partitions) {
	const node init_x = get_max_degree_node(partition);
	const std::vector<node> edges = propagate_from_x(init_x, partition);
	
#pragma omp critical(out)
	{
	    for (size_t idx = 0; idx < edges.size(); idx += 2) {
		add_edge(out, edges.at(idx), edges.at(idx + 1));
	    }
	}

    }
    
    dedup(out);

    std::vector<std::vector<node>> components = get_components(out);
    
    // TODO while loop this? can there still be disconnected components after running?
    if (components.size() > 1) {
        connect_components(out, components, adj_list);
    }

    return out;
}
