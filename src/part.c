

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
  /* initialize Zoltan */
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
  Zoltan_Set_Param(zz, "PHG_OUTPUT_LEVEL", "0");
  Zoltan_Set_Param(zz, "FINAL_OUTPUT", "1");
  Zoltan_Set_Param(zz, "LB_METHOD", "HYPERGRAPH");
  Zoltan_Set_Param(zz, "LB_APPROACH", "PARTITION");
  Zoltan_Set_Param(zz, "HYPERGRAPH_PACKAGE", "PHG");
  Zoltan_Set_Param(zz, "NUM_GID_ENTRIES", "1");
  Zoltan_Set_Param(zz, "NUM_LID_ENTRIES", "1");
  Zoltan_Set_Param(zz, "RETURN_LISTS", "ALL");
  Zoltan_Set_Param(zz, "CHECK_HYPERGRAPH", "1");

  /* default weights */
  Zoltan_Set_Param(zz, "OBJ_WEIGHT_DIM", "0");
  Zoltan_Set_Param(zz, "EDGE_WEIGHT_DIM", "0");

  /* set number of partitions */
  char * np = NULL;
  asprintf(&np, "%d", nparts);
  Zoltan_Set_Param(zz, "NUM_GLOBAL_PARTS", np);
  free(np);

  /* Application defined query functions */
  Zoltan_Set_Num_Obj_Fn(zz, hg_get_nvtx, hg);
  Zoltan_Set_Obj_List_Fn(zz, hg_get_vlist, hg);
  Zoltan_Set_HG_Size_CS_Fn(zz, hg_get_netsizes, hg);
  Zoltan_Set_HG_CS_Fn(zz, hg_get_hlist, hg);

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
  printf("nv: %d nh: %d ncon: %d\n", hg->nlocal_v, hg->nlocal_h, hg->nlocal_con);

  /* initialize zoltan and set parameters */
  struct Zoltan_Struct * zz = __init_zoltan(comm, hg, nparts);

  /* zoltan output vars */
  int changes;
  int gid_size, lid_size;
  int nimport, nexport;
  int myRank, numProcs;
  ZOLTAN_ID_PTR import_gids, import_lids;
  ZOLTAN_ID_PTR export_gids, export_lids;
  int * import_ranks, * import_part;
  int * export_ranks, * export_part;

#if 1
  /* do the partitioning */
  int rc = Zoltan_LB_Partition(zz,
        &changes,
        &gid_size, &lid_size,
        &nimport, &import_gids, &import_lids, &import_ranks, &import_part,
        &nexport, &export_gids, &export_lids, &export_ranks, &export_part);
  if (rc != ZOLTAN_OK){
    fprintf(stderr, "ZPART: Zoltan_LB_Partition() returned %d\n", rc);
    MPI_Finalize();
    exit(1);
    Zoltan_Destroy(&zz);
  }
#endif

  /* cleanup */
  Zoltan_Destroy(&zz);
}

