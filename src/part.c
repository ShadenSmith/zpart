

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "graph.h"
#include "timer.h"

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
  Zoltan_Set_Param(zz, "RETURN_LISTS", "PARTS");
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
int * partition(
    hgraph * hg,
    MPI_Comm comm,
    int nparts)
{
  /* initialize zoltan and set parameters */
  struct Zoltan_Struct * zz = __init_zoltan(comm, hg, nparts);

  int rank;
  MPI_Comm_rank(comm, &rank);

  /* zoltan output vars */
  int changes;
  int gid_size, lid_size;
  int nimport, nexport;
  ZOLTAN_ID_PTR import_gids, import_lids;
  ZOLTAN_ID_PTR export_gids, export_lids;
  int * import_ranks, * import_part;
  int * export_ranks, * export_part;

  MPI_Barrier(comm);
  zp_timer_t part_time;
  timer_fstart(&part_time);

  /* do the partitioning */
  int rc = Zoltan_LB_Partition(zz,
        &changes,
        &gid_size, &lid_size,
        &nimport, &import_gids, &import_lids, &import_ranks, &import_part,
        &nexport, &export_gids, &export_lids, &export_ranks, &export_part);
  if (rc != ZOLTAN_OK){
    fprintf(stderr, "ZPART: Zoltan_LB_Partition() returned %d\n", rc);
    MPI_Finalize();
    Zoltan_Destroy(&zz);
    exit(1);
  }

  MPI_Barrier(comm);
  timer_stop(&part_time);
  if(rank == 0) {
    printf("Zoltan/PHG partitioning time: %0.3fs\n", part_time.seconds);
  }

  /* process part lists */
  int * parts = (int *) malloc(hg->nlocal_v * sizeof(int));
  for(int v=0; v < hg->nlocal_v; ++v) {
    parts[export_lids[v]] = export_part[v];
  }

  /* cleanup */
  Zoltan_LB_Free_Part(&import_gids, &import_lids, &import_ranks, &import_part);
  Zoltan_LB_Free_Part(&export_gids, &export_lids, &export_ranks, &export_part);
  Zoltan_Destroy(&zz);

  return parts;
}


void write_parts(
    MPI_Comm comm,
    int const * const parts,
    int nvtxs,
    char const * const fname)
{
  int rank, npes;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &npes);

  FILE * fout;
  if(rank == 0) {
    if((fout = fopen(fname, "w")) == NULL) {
      fprintf(stderr, "ZPART: failed to open '%s'\n", fname);
      MPI_Finalize();
      exit(1);
    }
  }


  if(rank == 0) {
    int * part_buf = NULL;
    int buf_size = 0;
    MPI_Status status;
    /* receive from each rank */
    for(int p=1; p < npes; ++p) {
      /* receive partition info */
      int newsize;
      MPI_Recv(&newsize, 1, MPI_INT, p, DEF_TAG, comm, &status);
      part_buf = realloc(part_buf, newsize * sizeof(int));
      MPI_Recv(part_buf, newsize, MPI_INT, p, DEF_TAG, comm, &status);

      /* now write to file */
      for(int v=0; v < newsize; ++v) {
        fprintf(fout, "%d\n", part_buf[v]);
      }
    }
    free(part_buf);

    /* now write own vertices */
    for(int v=0; v < nvtxs; ++v) {
      fprintf(fout, "%d\n", parts[v]);
    }

  } else {
    /* just send part info */
    MPI_Send(&nvtxs, 1, MPI_INT, 0, DEF_TAG, comm);
    MPI_Send(parts, nvtxs, MPI_INT, 0, DEF_TAG, comm);
  }

  if(rank == 0) {
    fclose(fout);
  }
}



