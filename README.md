![](https://github.com/wigasper/planarityfilter/workflows/build/badge.svg) ![](https://github.com/wigasper/planarityfilter/workflows/deploy/badge.svg)

# planarityfilter

`planarityfilter` provides an algorithm for obtaining planar-approximations of 
large graphs through edge addition via graphlets. Currently only undirected, unweighted
graphs are supported.

Currently requires Boost for C++ for graph libraries, argument parsing, and logging. 
The dynamic linking associated with logging may not be strictly necessary, so if this is an issue
this dependency can be eliminated --- just open an issue.

A Linux binary is included in the releases, if additional target OSes are needed please just 
open an issue and I will add this to the CD.

This work builds on the code from [Nathan Cornelius](https://github.com/ndcornelius/PlanarSubgraphApproximationAlgorithm), who is a co-author, and is used with permission.

To build:

```bash
git clone git@github.com:wigasper/planarityfilter.git
cd planarityfilter
mkdir build && cd build
cmake ..
make
```

To run:

```bash
$ build/planarityfilter -h
Arguments:
  -h [ --help ]         display help message
  -i [ --input ] arg    input file path
  -o [ --output ] arg   output file path
  -t [ --threads ] arg  number of threads to use
```

Input and output are simple edge lists, where each line contains the two 
nodes of the edge separated by whitespace. 
