ZPart: A Simple Zoltan Frontend
===============================
`ZPart` is a thin frontend around [Zoltan](http://www.cs.sandia.gov/Zoltan/), a
hypergraph partitioner parallelized with MPI. This codebase provides a
commandline frontend for partitioning hypergraphs in
[hMetis](http://glaros.dtc.umn.edu/gkhome/metis/hmetis/overview) format in
parallel.

This source code was developed for the experiments in the 2016 IPDPS paper *A
Medium-Grained Algorithm for Distributed Sparse Tensor Factorization* (paper
[here](http://shaden.io/pub-files/smith2016medium.pdf)).


Requirements
------------
  * An MPI compiler (tested with OpenMPI)
  * CMake >= 2.6.0
  * [Zoltan](http://www.cs.sandia.gov/Zoltan/)

Building
--------
Though supported, we do not recommend an in-source build. You can build `ZPart`
via:

  $ mkdir build && cd build
  $ cmake <PATH_TO_ZPART>
  $ make

Running
-------
After building `ZPart`, an executable is found in location `bin/zpart`. You
can run via:

  $ mpirun -np <NUM_PROCS> ./bin/zpart [hgraph] [nparts] [output]

After running, `output` will store the assigned partition for each vertex in
the hypergraph (0-indexed).


Configuration
-------------
`Zoltan` is highly configurable. The source code in `ZPart` uses some values
which were used in our experimental evaluation and we felt were sane.  These
can be changed in the source code in `src/part.c`, in the function
`__init_zoltan()`.
