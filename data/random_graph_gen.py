import argparse

from networkx.generators.random_graphs import gnm_random_graph

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-n", "--nodes", help="Approximate (subject to floor division) "  
                "total number of nodes in graph", required=True, type=int)
    args = parser.parse_args()
    
    densities = [x / 100.0 for x in range(100)]

    for density in densities:
        edges = int(density * args.nodes * (args.nodes - 1) * 1/2)

        graph = gnm_random_graph(args.nodes, edges)

        with open(f"density_trial/{int(density * 100)}_density_graph", "w") as out:

            for edge in graph.edges:
                out.write(" ".join([str(edge[0]), str(edge[1]), "\n"]))

if __name__ == "__main__":
    main()
