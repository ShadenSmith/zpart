

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>


/******************************************************************************
 * TYPES & CONSTANTS
 *****************************************************************************/
/* just to make life easier */
#define idx_t ZOLTAN_ID_TYPE

static int const DEF_TAG = 0;


/******************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/


static struct Zoltan_Struct * __init_zoltan(
    MPI_Comm comm,
    hgraph * hg,
    int nparts)
{
  float ver;
  int rc = Zoltan_Initialize(0, NULL, &ver);
  if(rc != ZOLTAN_OK) {
    fprintf(stderr, "ZPART: Zoltan_Init() returned %d\n", rc);
    MPI_Finalize();
    exit(1);
  }

  struct Zoltan_Struct * zz = Zoltan_Create(MPI_COMM_WORLD);

  /* General parameters */

  Zoltan_Set_Param(zz, "DEBUG_LEVEL", "0");
  Zoltan_Set_Param(zz, "LB_METHOD", "HYPERGRAPH");
  Zoltan_Set_Param(zz, "LB_APPROACH", "PARTITION");
  Zoltan_Set_Param(zz, "HYPERGRAPH_PACKAGE", "PHG");
  Zoltan_Set_Param(zz, "NUM_GID_ENTRIES", "1");
  Zoltan_Set_Param(zz, "NUM_LID_ENTRIES", "1");
  Zoltan_Set_Param(zz, "RETURN_LISTS", "ALL");

  /* default weights */
  Zoltan_Set_Param(zz, "OBJ_WEIGHT_DIM", "0");
  Zoltan_Set_Param(zz, "EDGE_WEIGHT_DIM", "0");

  /* set number of partitions */
  char * np;
  asprintf(&np, "%d", nparts);
  Zoltan_Set_Param(zz, "NUM_GLOBAL_PARTS", np);
  free(np);

  /* Application defined query functions */
  Zoltan_Set_Num_Obj_Fn(zz, hg_get_nvtx, hg);
  Zoltan_Set_Obj_List_Fn(zz, hg_get_vlist, &hg);
  Zoltan_Set_HG_Size_CS_Fn(zz, hg_get_netsizes, &hg);
  Zoltan_Set_HG_CS_Fn(zz, hg_get_hlist, &hg);

  return zz;
}


/******************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/
void partition(
    hgraph * hg,
    MPI_Comm comm,
    int nparts)
{
  printf("partitioning into %d\n", nparts);

  /* initialize zoltan and set parameters */
  struct Zoltan_Struct * zz = __init_zoltan(comm, hg, nparts);

  /* do the partitioning */

  Zoltan_Destroy(&zz);
}
