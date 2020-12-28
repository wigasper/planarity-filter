#include "algo.h"

#include <chrono>
#include <stdlib.h>
#include <version.h>

#include "boost/program_options.hpp"

int main(int argc, char *argv[]) {
    log_init();
    
    namespace po = boost::program_options;

    po::options_description desc("Arguments");
    desc.add_options()("help,h", "display help message")
        ("input,i", po::value<std::string>()->required(), "input file path");
    
    //size_t num_replicates = 10;
    size_t num_replicates = 1;

    po::variables_map var_map;

    try {
        po::store(po::parse_command_line(argc, argv, desc), var_map);
        if (var_map.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }
        po::notify(var_map);
    } catch (po::error &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        std::cerr << desc << "\n";
        return 1;
    } catch (std::exception &e) {
        std::cerr << "Unhandled exception: " << e.what() << "\n";
        return 2;
    }

    BOOST_LOG_TRIVIAL(info) << "#######################################";
    BOOST_LOG_TRIVIAL(info) << "New run, options listed below";
    BOOST_LOG_TRIVIAL(info) << "git branch: " << GIT_BRANCH;
    BOOST_LOG_TRIVIAL(info) << "abbrev. commit hash: " << GIT_COMMIT_HASH;
    BOOST_LOG_TRIVIAL(info) << "Input: " << var_map["input"].as<std::string>();
    BOOST_LOG_TRIVIAL(info) << "Num. replicates: " << num_replicates;
    BOOST_LOG_TRIVIAL(info) << "Loading input";

    const load_result lr = load_edge_list(var_map["input"].as<std::string>());
    adjacency_list input_graph = to_adj_list(std::get<0>(lr));

    // node_labels is a map node: string
    const std::unordered_map<node, std::string> node_labels = std::get<2>(lr);

    BOOST_LOG_TRIVIAL(info) << "Checking to see if graph is already planar";

    if (boyer_myrvold_test(input_graph)) {
        BOOST_LOG_TRIVIAL(info) << "The provided graph is already planar";
        exit(EXIT_SUCCESS);
    }
    
    // dedup input graph
    dedup(input_graph);
    
    std::vector<int> thread_nums {8};
    //std::vector<int> thread_nums {1, 2, 4, 8, 16, 32, 64};
     
    for (int threads : thread_nums) {
	BOOST_LOG_TRIVIAL(info) << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";
	BOOST_LOG_TRIVIAL(info) << "Num. threads: " << threads;
	for (size_t _ = 0; _ < num_replicates; _++) {
	    BOOST_LOG_TRIVIAL(info) << "Running algo_routine";
	    auto start = std::chrono::high_resolution_clock::now();
	    adjacency_list result_graph = algo_routine(input_graph, threads);
	    auto finish = std::chrono::high_resolution_clock::now();
	    std::chrono::duration<double> elapsed = finish - start;

	    if (!boyer_myrvold_test(result_graph)) {
		BOOST_LOG_TRIVIAL(error) << "Error: the result graph is not planar";
		exit(EXIT_FAILURE);
	    }

	    size_t input_n_edges = num_edges(input_graph);
	    size_t result_n_edges = num_edges(result_graph);
            size_t result_n_nodes = result_graph.size();
            float edge_retention = (float) result_n_edges / (float) input_n_edges;
            float maximal_edge_retention = (float) maximal_planar(input_graph, 
                    result_graph) / float(input_n_edges);
            
            float input_density = (float) (2 * input_n_edges) / (float) (input_graph.size() * (input_graph.size() - 1));

	    BOOST_LOG_TRIVIAL(info) << "Execution time: " << elapsed.count() << "s"
	    BOOST_LOG_TRIVIAL(info) << "Initial graph - " << "nodes: " << input_graph.size()
		<< " edges: " << input_n_edges;
	    BOOST_LOG_TRIVIAL(info) << "Result graph - " << "nodes: " << result
		<< " edges: " << result_n_edges;
	    BOOST_LOG_TRIVIAL(info) << "Percent edges retained: "
		<< edge_retention;
            BOOST_LOG_TRIVIAL(info) << "$nodes|" << input_graph.size() << "|edge_retention|" << edge_retention << "|maximal_planar_edge_retention|" << maximal_edge_retention << "|input_density|" << input_density << "$";
	}
    }
    return 0;
}
