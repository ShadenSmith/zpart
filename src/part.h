#ifndef ZPART_PART_H
#define ZPART_PART_H

/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <zoltan.h>
#include "graph.h"


/******************************************************************************
 * FUNCTIONS
 *****************************************************************************/

int * partition(
    hgraph * hg,
    MPI_Comm comm,
    int nparts);

void write_parts(
    MPI_Comm comm,
    int const * const parts,
    int nvtxs,
    char const * const fname);

#endif
