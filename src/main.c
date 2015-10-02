
/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "graph.h"
#include "part.h"


/******************************************************************************
 * PROGRAM ENTRY
 *****************************************************************************/
int main(
    int argc,
    char ** argv)
{
  MPI_Init(&argc, &argv);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(argc < 4) {
    if(rank == 0) {
      printf("usage: %s [hmetis graph] [nparts] [out]\n", argv[0]);
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
  }

  /* load and distribute graph */
  char const * const gfname = argv[1];
  hgraph * hg = distribute_hgraph(gfname, MPI_COMM_WORLD);
  if(hg == NULL) {
    MPI_Finalize();
    return EXIT_FAILURE;
  }

  char * endptr;
  int const nparts = (int) strtol(argv[2], &endptr, 10);
  if(endptr == argv[2]) {
    printf("ZPART: integer expected for #partitions\n");
    MPI_Finalize();
    return EXIT_FAILURE;
  }

  partition(hg, MPI_COMM_WORLD, nparts);

  hgraph_free(hg);

  MPI_Finalize();
  return EXIT_SUCCESS;
}
