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

void partition(
    hgraph * hg,
    MPI_Comm comm,
    int nparts);

#endif
