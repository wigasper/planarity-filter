// This work is heavily comprised of Nathan Cornelius's original code from
// github.com/ndcornelius/PlanarSubgraphApproximationAlgorithm

#include "AdjacencyList.h"
#include "PlanarGraphHeuristic.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <version.h>

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/boyer_myrvold_planar_test.hpp"
#include "boost/graph/graph_traits.hpp"
#include "boost/log/core.hpp"
#include "boost/log/expressions.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"
#include "boost/log/utility/setup/console.hpp"
#include "boost/log/utility/setup/file.hpp"
#include "boost/program_options.hpp"

//////
//
///
#include <omp.h>
////
enum input_type { weighted, unweighted, invalid };

typedef int node;

typedef std::pair<node, node> unweighted_edge;
typedef std::tuple<node, node, double> weighted_edge;
typedef std::list<unweighted_edge> unweighted_edge_list;
typedef std::list<weighted_edge> weighted_edge_list;

typedef std::tuple<weighted_edge_list, std::unordered_map<std::string, int>,
		   std::unordered_map<int, std::string>>
    load_result;

// is_planar is a wrapper for the boost library Boyer-Myrvold planarity test
// algorithm Parameters: an edge list pointer and the number of nodes in the
// graph Returns: 'true' if planar and 'false' if not planar
bool is_planar(Graph &graph) {

    unweighted_edge_list edge_list = graph.toEdgeList(false);
    boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS> boost_graph(
	graph.getNumNodes());

    for (unweighted_edge edge : edge_list) {
	boost::add_edge(edge.first, edge.second, boost_graph);
    }

    return boyer_myrvold_planarity_test(boost_graph);
}

// TODO: input validation step
// input_type check_input(std::string file_path) {
//    std::ifstream in_stream(file_path);
//    if (in_stream) {
//	std::string line;
//	if (std::getline(in_stream, line)) {
//	    // std::vector<string> line_vec;
//	    // size_t pos = 0;
//	    std::cout << line;
//	    // while ((position = line.find(" "))
//	    std::regex weighted_regex("^\\w+\\s+\\w+\\s+\\w+\\s$");
//	    std::regex unweighted_regex("^\\w+\\s+\\w+\\s$");
//
//	    if (std::regex_match(line, weighted_regex)) {
//		std::cout << "weighted";
//		return weighted;
//	    } else if (std::regex_match(line, unweighted_regex)) {
//		std::cout << "unweighted";
//		return unweighted;
//	    } else {
//		std::cout << "no match";
//	    }
//	}
//   }
//}

// load_from_file loads a graph file with edges in the form 'node node weight'
// seperated by whitespace Parameters: A string with the path to the file
load_result load_from_file(std::string file_path) {
    weighted_edge_list edge_list;
    std::unordered_map<std::string, int> nodeIDs;
    std::unordered_map<int, std::string> nodeIDsReverse;

    // input_type input_format = check_input(file_path);

    std::string node_0, node_1; //, weight;
    double weight;
    std::string line;
    std::ifstream in_stream(file_path);

    int currentNodeID = 0;
    if (in_stream) {
	while (in_stream >> node_0 >> node_1 >> weight) {
	    auto search = nodeIDs.find(node_0);
	    if (search == nodeIDs.end()) {
		nodeIDs[node_0] = currentNodeID;
		nodeIDsReverse[currentNodeID] = node_0;
		currentNodeID++;
	    }

	    search = nodeIDs.find(node_1);
	    if (search == nodeIDs.end()) {
		nodeIDs[node_1] = currentNodeID;
		nodeIDsReverse[currentNodeID] = node_1;
		currentNodeID++;
	    }
	    edge_list.push_back(std::make_tuple(nodeIDs[node_0], nodeIDs[node_1], weight));
	}
	in_stream.close();

    } else {
	std::cout << "\nCannot load " << file_path;
	exit(EXIT_FAILURE);
    }
    return std::make_tuple(edge_list, nodeIDs, nodeIDsReverse);
}

// Write output to file
void write_to_file(weighted_edge_list edge_list, std::string file_path,
		   std::unordered_map<int, std::string> nodeIDsReverse) {
    BOOST_LOG_TRIVIAL(info) << "Writing result to file...";

    std::ofstream file(file_path);

    for (weighted_edge edge : edge_list) {
	std::string node_0 = nodeIDsReverse[std::get<0>(edge)];
	std::string node_1 = nodeIDsReverse[std::get<1>(edge)];
	double weight = std::get<2>(edge);
	file << node_0 << " " << node_1 << " " << weight << "\n";
    }

    file.close();
}

void log_init() {
    std::string log_format = "[%TimeStamp%] [%Severity%] [%Message%]";
    std::string log_path = "planarity_filter.log";
    boost::log::add_file_log(log_path, boost::log::keywords::format = log_format,
			     boost::log::keywords::open_mode = std::ios_base::app);
    boost::log::add_console_log(std::cout, boost::log::keywords::format = log_format);
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    boost::log::add_common_attributes();
}

int main(int argc, char *argv[]) {
    // Initialization, get arguments, log details
    log_init();

    int num_threads = 1;

    namespace po = boost::program_options;

    po::options_description desc("Arguments");
    desc.add_options()("help,h", "display help message")(
	"input,i", po::value<std::string>()->required(),
	"input file path")("output,o", po::value<std::string>()->required(), "output file path")(
	"threads,t", po::value<int>(&num_threads), "number of threads to use");

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

    if (num_threads < 1) {
	// TODO: invalid value provided, do something in logger or stdout here
	num_threads = 1;
    }
    std::cout << "num: " << omp_get_num_procs();
    BOOST_LOG_TRIVIAL(info) << "#######################################";
    BOOST_LOG_TRIVIAL(info) << "New run, options listed below";
    BOOST_LOG_TRIVIAL(info) << "git branch: " << GIT_BRANCH;
    BOOST_LOG_TRIVIAL(info) << "abbrev. commit hash: " << GIT_COMMIT_HASH;
    BOOST_LOG_TRIVIAL(info) << "Input: " << var_map["input"].as<std::string>();
    BOOST_LOG_TRIVIAL(info) << "Output: " << var_map["output"].as<std::string>();

    unsigned int supported_concurrency = std::thread::hardware_concurrency();
    if (num_threads > supported_concurrency) {
	BOOST_LOG_TRIVIAL(warning)
	    << "std::thread::hardware_concurrency() reported " << supported_concurrency
	    << " threads supported by hardware. This number may be "
	    << "incorrect, if it is correct it may not be beneficial to "
	    << "spawn more threads than supported because the algorithm "
	    << "will run all threads at 100%";
    }

    BOOST_LOG_TRIVIAL(info) << "Loading input";

    // Deal with input, initialize related vars
    unweighted_edge_list unweighted_initial_edge_list;
    Graph input_graph = AdjacencyList::Graph();

    load_result input_data = load_from_file(var_map["input"].as<std::string>());
    weighted_edge_list input_edge_list = std::get<0>(input_data);
    std::unordered_map<std::string, int> nodeIDs = std::get<1>(input_data);
    std::unordered_map<int, std::string> nodeIDsReverse = std::get<2>(input_data);

    weighted_edge_list temp = input_edge_list;

    for (weighted_edge edge : input_edge_list) {
	input_graph.addEdge(std::get<0>(edge), std::get<1>(edge));
	unweighted_initial_edge_list.push_back(
	    unweighted_edge(std::get<0>(edge), std::get<1>(edge)));
    }

    input_graph.trim();

    if (input_edge_list.size() < 1) {
	BOOST_LOG_TRIVIAL(error) << "No edges were read from the provided file";
	exit(EXIT_FAILURE);
    }

    BOOST_LOG_TRIVIAL(info) << "Checking to see if graph is already planar";
    if (is_planar(input_graph)) {
	BOOST_LOG_TRIVIAL(info) << "The provided graph is already planar, exiting";
	exit(EXIT_SUCCESS);
    }

    auto start = std::chrono::high_resolution_clock::now();
    // Planar graph filter
    Graph result_graph = multipleComponentAlgorithmA(&input_graph, num_threads);
    auto finish = std::chrono::high_resolution_clock::now();

    weighted_edge_list result_weighted_edge_list;
    unweighted_edge_list result_edge_list = result_graph.toEdgeList(false);

    if (!is_planar(result_graph)) {
	BOOST_LOG_TRIVIAL(error) << "Error: the result graph is not planar";
	exit(EXIT_FAILURE);
    }

    int input_num_edges = input_graph.getNumEdges();
    int result_num_edges = result_graph.getNumEdges();

    std::chrono::duration<double> elapsed = finish - start;
    BOOST_LOG_TRIVIAL(info) << "Execution time: " << elapsed.count() << "s";
    BOOST_LOG_TRIVIAL(info) << "Initial graph - "
			    << "nodes: " << input_graph.getNumNodes();
    BOOST_LOG_TRIVIAL(info) << "Result graph - "
			    << "nodes: " << result_graph.getNumNodes();
    BOOST_LOG_TRIVIAL(info) << "Initial graph - "
			    << "nodes w/ edges: " << input_graph.getNumNodesWithEdges()
			    << " edges: " << input_num_edges;
    BOOST_LOG_TRIVIAL(info) << "Result graph - "
			    << "nodes w/ edges: " << result_graph.getNumNodesWithEdges()
			    << " edges: " << result_num_edges;
    BOOST_LOG_TRIVIAL(info) << "Percent edges retained: "
			    << (float)result_num_edges / (float)input_num_edges * 100 << "%";

    std::unordered_set<std::string> result_set;

    for (unweighted_edge edge : result_edge_list) {
	result_set.insert(std::to_string(std::get<0>(edge)) + " " +
			  std::to_string(std::get<1>(edge)));
    }

    for (weighted_edge edge : input_edge_list) {
	std::string input_edge_unweighted =
	    std::to_string(std::get<0>(edge)) + " " + std::to_string(std::get<1>(edge));
	auto search = result_set.find(input_edge_unweighted);
	if (search != result_set.end()) {
	    result_weighted_edge_list.push_back(edge);
	}
    }

    write_to_file(result_weighted_edge_list, var_map["output"].as<std::string>(), nodeIDsReverse);

    return 0;
}
