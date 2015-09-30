
/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "graph.h"

#include <stdlib.h>
#include <mpi.h>


/******************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/


/******************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/

hgraph * hgraph_alloc(
    int local_vtxs,
    int local_hedges,
    int local_connections)
{
  hgraph * hg = (hgraph *) malloc(sizeof(hgraph));

  hg->eptr = (int *) malloc((local_hedges+1) * sizeof(int));
  hg->v_gids = (ZOLTAN_ID_TYPE *) malloc(local_vtxs * sizeof(ZOLTAN_ID_TYPE));
  hg->h_gids = (ZOLTAN_ID_TYPE *) malloc(local_hedges * \
      sizeof(ZOLTAN_ID_TYPE));
  hg->eind = (ZOLTAN_ID_TYPE *) malloc(local_connections * \
      sizeof(ZOLTAN_ID_TYPE));

  return hg;
}


void hgraph_free(
    hgraph * const hg)
{
  free(hg->eptr);
  free(hg->v_gids);
  free(hg->h_gids);
  free(hg->eind);
  free(hg);
}
