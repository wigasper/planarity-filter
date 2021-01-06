![](https://github.com/wigasper/planarity-filter/workflows/build/badge.svg)

# planarity-filter

`planarity-filter` provides an algorithm for obtaining planar-approximations of 
large graphs through edge addition via graphlets. Currently only undirected, unweighted
graphs are supported.

To build:

```bash
git clone git@github.com:wigasper/planarity-filter.git
cd planarity-filter
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
