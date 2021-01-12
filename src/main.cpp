#include "algo.h"

#include <chrono>
#include <stdlib.h>
#include <version.h>

#include "boost/program_options.hpp"

#include "boost/log/core.hpp"
#include "boost/log/expressions.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"
#include "boost/log/utility/setup/console.hpp"
#include "boost/log/utility/setup/file.hpp"

// Initializes the logger
void log_init() {
      std::string log_format = "[%TimeStamp%] [%Message%]";
      std::string log_path = "thread_exp.log";
      boost::log::add_file_log(log_path, boost::log::keywords::format = log_format,
                               boost::log::keywords::open_mode = std::ios_base::app);
      boost::log::add_console_log(std::cout, boost::log::keywords::format = log_format);
      boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
      boost::log::add_common_attributes();
  }

int main(int argc, char *argv[]) {
    log_init();
    
    size_t num_replicates = 50; 
    size_t num_to_save = 10;
    // Get args
    namespace po = boost::program_options;

    po::options_description desc("Arguments");
    desc.add_options()("help,h", "display help message")
        ("input,i", po::value<std::string>()->required(), "input file path");

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

    // Log metadata about the run
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

    std::vector<int> thread_nums {1, 2, 4, 8, 16, 32, 64};

    for (int threads : thread_nums) {
	auto start = std::chrono::high_resolution_clock::now();
	adjacency_list result_graph = algo_routine(input_graph, threads);
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	BOOST_LOG_TRIVIAL(info) << "$" << threads << "," << elapsed.count() <<"$";	

    }
    BOOST_LOG_TRIVIAL(info) << "########################"
    adjacency_list result_graph = algo_routine(input_graph, num_threads);
    dedup(result_graph);

    if (!boyer_myrvold_test(result_graph)) {
	BOOST_LOG_TRIVIAL(error) << "Error: the result graph is not planar";
	exit(EXIT_FAILURE);
    }

    size_t input_n_edges = num_edges(input_graph);
    size_t result_n_edges = num_edges(result_graph);

    BOOST_LOG_TRIVIAL(info) << "Initial graph - " << "nodes: " << input_graph.size()
        << " edges: " << input_n_edges;
    BOOST_LOG_TRIVIAL(info) << "Result graph - " << "nodes: " << result_graph.size()
        << " edges: " << result_n_edges;
    BOOST_LOG_TRIVIAL(info) << "Percent edges retained: "
        << (float) result_n_edges / (float) input_n_edges * 100;
    
    write_graph(result_graph, node_labels, var_map["output"].as<std::string>());
    return 0;
}
