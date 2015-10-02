
/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
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

/**
* @brief Remove any whitespace from the end of a C-string and replace with
*        terminals.
*
* @param line The C-string to edit.
*/
static void __rstrip_line(
    char * const line)
{
  for(size_t i=strlen(line) - 1; i > 0; --i) {
    if(isspace(line[i])) {
      line[i] = '\0';
    } else {
      break;
    }
  }
}


/**
* @brief Read from 'fin' until a non-commented line is reached, split the line
*        into an array of idx_t, and return the array.
*
* @param fin The file to read from.
* @param nvals [OUT] Pointer to the length of the returned array.
*
* @return The allocated array of idx_t, which must be freed!
*/
static idx_t * __split_line(
    FILE * fin,
    int * nvals)
{
  ssize_t nread;
  size_t len;
  char * line = NULL;

  /* grab the line */
  do {
    free(line);
    nread = getline(&line, &len, fin);
    if(nread == -1) {
      fprintf(stderr, "ZPART: unexpected end of input.\n");
      MPI_Finalize();
      exit(1);
    }
  } while(line[0] == '#' || line[0] == '%'); /* skip comment lines */

  /* strip whitespace from end */
  __rstrip_line(line);

  /* make a backup of line */
  char * line_tmp = (char *) malloc((strlen(line) + 1) * sizeof(char));
  strcpy(line_tmp, line);

  /* now count values */
  int count = 0;
  char * ptr = strtok(line_tmp, " \t");
  while(ptr != NULL) {
    count += 1;
    ptr = strtok(NULL, " \t");
  }
  free(line_tmp);

  ptr = line;
  /* allocate and fill array */
  idx_t * arr = (idx_t *) malloc(count * sizeof(idx_t));
  for(int i=0; i < count; ++i) {
    arr[i] = (idx_t) strtoll(ptr, &ptr, 10);
  }
  free(line);

  *nvals = count;
  return arr;
}


/**
* @brief Read the next line, split, and accumulate entries into buf.
*
* @param fin The file to read from
* @param buf The buffer to add to. Resized if necessary.
* @param next_len The number of added entries.
* @param bsize The new size of buf (in #entries).
* @param ncon We add the number of connections (#entries read).
*
* @return The (possibly resized) buffer.
*/
static idx_t *  __accum_line(
    FILE * fin,
    idx_t * buf,
    int * next_len,
    size_t * bsize,
    idx_t * ncon)
{
  /* get next array */
  int len;
  idx_t * arr = __split_line(fin, &len);

  /* store length */
  *next_len = len;
  *ncon += len;

  buf = (idx_t *) realloc(buf, (*bsize+len) * sizeof(idx_t));
  memcpy(buf + (*bsize), arr, len * sizeof(idx_t));
  *bsize += (size_t) len;

  free(arr);
  return buf;
}


/**
* @brief Do a distribution of a hypergraph and send chunks to other ranks.
*
* @param fname The file to read from.
* @param comm The communicator to distribute among.
*
* @return My own chunk of the hypergraph.
*/
static hgraph * __send_graph(
    char const * const fname,
    MPI_Comm comm)
{
  FILE * fin;
  if((fin = fopen(fname, "r")) == NULL) {
    fprintf(stderr, "ZPART: failed to open '%s'\n", fname);
    MPI_Finalize();
    exit(1);
  }

  int npes;
  MPI_Comm_size(comm, &npes);

  /* get global dims */
  int len;
  idx_t * dims = __split_line(fin, &len);
  if(len != 2) {
    fprintf(stderr, "ZPART: only unweighted graphs supported right now.\n");
    MPI_Finalize();
    exit(1);
  }
  /* only handle unweighted right now, otherwise dims[3] would be fmt */
  assert(len == 2);

  /* send global dims */
  MPI_Bcast(&len, 1, MPI_INT, 0, comm);
  MPI_Bcast(dims, len, ZOLTAN_ID_MPI_TYPE, 0, comm);

  idx_t const nhedges = dims[0];
  idx_t const nvtxs   = dims[1];

  idx_t * buf = NULL;
  size_t bsize = 0;

  int const vtarget = (int) (nvtxs / (idx_t)npes);
  int const htarget = (int) (nhedges / (idx_t)npes);
  idx_t * vids = (idx_t *) malloc(vtarget * sizeof(idx_t));
  idx_t * hids = (idx_t *) malloc(htarget * sizeof(idx_t));
  int * lengths = (int *) malloc(htarget * sizeof(int));
  /* read a chunk, send a chunk */
  for(int p=1; p < npes; ++p) {
    idx_t ncon = 0;
    /* accumulate each row into buf */
    for(int h=0; h < htarget; ++h) {
      buf = __accum_line(fin, buf, lengths + h, &bsize, &ncon);
    }

    idx_t start;

    /* send counts */
    MPI_Send(&vtarget, 1, MPI_INT, p, DEF_TAG, comm);
    MPI_Send(&htarget, 1, MPI_INT, p, DEF_TAG, comm);
    MPI_Send(&ncon, 1, ZOLTAN_ID_MPI_TYPE, p, DEF_TAG, comm);

    /* send vertex info */
    start = (p-1) * vtarget;
    for(idx_t v = start; v < start + vtarget; ++v) {
      vids[v - start] = v;
    }
    MPI_Send(vids, vtarget, ZOLTAN_ID_MPI_TYPE, p, DEF_TAG, comm);

    /* send hedge info */
    start = (p-1) * htarget;
    for(idx_t h=start; h < start + htarget; ++h) {
      hids[h - start] = h;
    }
    MPI_Send(hids, htarget, ZOLTAN_ID_MPI_TYPE, p, DEF_TAG, comm);

    /* send sparsity structure */
    MPI_Send(lengths, htarget, MPI_INT, p, DEF_TAG, comm);
    MPI_Send(buf, ncon, ZOLTAN_ID_MPI_TYPE, p, DEF_TAG, comm);

    /* reset buffer for accumulation */
    bsize = 0;
  }

  free(vids);
  free(hids);

  /* root takes the rest */
  idx_t ncon;
  idx_t vstart = (npes-1) * vtarget;
  idx_t hstart = (npes-1) * htarget;
  for(idx_t h=hstart; h < nhedges; ++h) {
    buf = __accum_line(fin, buf, lengths + (h-hstart), &bsize, &ncon);
  }


  int local_vtxs   = nvtxs - ((npes-1) * vtarget);
  int local_hedges = nhedges - ((npes-1) * htarget);

  hgraph * hg = hgraph_alloc(local_vtxs, local_hedges, ncon);

  /* fill in vids and hids */
  for(idx_t v=vstart; v < nvtxs; ++v) {
    hg->v_gids[v - vstart] = v;
  }
  for(idx_t h=hstart; h < nhedges; ++h) {
    hg->h_gids[h - hstart] = h;
  }

  free(hg->eptr);
  free(hg->eind);
  hg->eptr = lengths;
  hg->eind = buf;

  /* do a prefix sum on eptr to get proper pointer structure */
  int saved = hg->eptr[0];
  hg->eptr[0] = 0;
  for(int i=1; i <= local_hedges; ++i) {
    int tmp = hg->eptr[i];
    hg->eptr[i] = saved + hg->eptr[i-1];
    saved = tmp;
  }

  fclose(fin);

  return hg;
}


/**
* @brief Receive my own chunk of a distributed hypergraph.
*
* @param rank My rank in the communicator.
* @param comm The communicator I am in.
*
* @return My chunk of the hypergraph.
*/
static hgraph * __recv_graph(
    int rank,
    MPI_Comm comm)
{
  /* receive global dims */
  int len;
  MPI_Bcast(&len, 1, MPI_INT, 0, comm);
  idx_t * dims = (idx_t *) malloc(len * sizeof(idx_t));
  MPI_Bcast(dims, len, ZOLTAN_ID_MPI_TYPE, 0, comm);

  idx_t nhedges = dims[0];
  idx_t nvtxs = dims[1];
  free(dims);

  MPI_Status status;

  /* receive sizes */
  int local_vtxs;
  int local_hedges;
  idx_t ncon;
  MPI_Recv(&local_vtxs, 1, MPI_INT, 0, DEF_TAG, comm, &status);
  MPI_Recv(&local_hedges, 1, MPI_INT, 0, DEF_TAG, comm, &status);
  MPI_Recv(&ncon, 1, ZOLTAN_ID_MPI_TYPE, 0, DEF_TAG, comm, &status);

  /* store everything in a structure */
  hgraph * hg = hgraph_alloc(local_vtxs, local_hedges, ncon);

  /* receive vertex and hedge ids */
  MPI_Recv(hg->v_gids, local_vtxs, ZOLTAN_ID_MPI_TYPE, 0, DEF_TAG, comm,
      &status);
  MPI_Recv(hg->h_gids, local_hedges, ZOLTAN_ID_MPI_TYPE, 0, DEF_TAG, comm,
      &status);

  /* receive sparsity structure */
  MPI_Recv(hg->eptr, local_hedges, MPI_INT, 0, DEF_TAG, comm, &status);
  MPI_Recv(hg->eind, ncon, ZOLTAN_ID_MPI_TYPE, 0, DEF_TAG, comm, &status);

  /* do a prefix sum on eptr to get proper pointer structure */
  int saved = hg->eptr[0];
  hg->eptr[0] = 0;
  for(int i=1; i <= local_hedges; ++i) {
    int tmp = hg->eptr[i];
    hg->eptr[i] = saved + hg->eptr[i-1];
    saved = tmp;
  }

  hg->nglobal_v = nvtxs;
  hg->nglobal_h = nhedges;

  return hg;
}



/******************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/
hgraph * distribute_hgraph(
    char const * const fname,
    MPI_Comm comm)
{
  int rank;
  MPI_Comm_rank(comm, &rank);

  if(rank == 0) {
    return __send_graph(fname, comm);
  } else {
    return __recv_graph(rank, comm);
  }

  return NULL;
}

hgraph * hgraph_alloc(
    int local_vtxs,
    int local_hedges,
    int local_connections)
{
  hgraph * hg = (hgraph *) malloc(sizeof(hgraph));

  hg->v_gids = (idx_t *) malloc(local_vtxs * sizeof(idx_t));
  hg->h_gids = (idx_t *) malloc(local_hedges * \
      sizeof(idx_t));

  hg->eptr = (int *) malloc((local_hedges+1) * sizeof(int));
  hg->eind = (idx_t *) malloc(local_connections * \
      sizeof(idx_t));

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



/******************************************************************************
 * QUERY FUNCTIONS
 *****************************************************************************/
int hg_get_nvtx(
    void * data,
    int * ierr)
{
  hgraph const * const hg = (hgraph *) data;
  *ierr = ZOLTAN_OK;
  return hg->nlocal_v;
}

void hg_get_vlist(
    void * data,
    int gid_size,
    int lid_size,
    ZOLTAN_ID_PTR gids,
    ZOLTAN_ID_PTR lids,
    int wt_size,
    float * vtx_wts,
    int * ierr)
{
  hgraph const * const hg = (hgraph *) data;
  *ierr = ZOLTAN_OK;

  /* fill in global ids */
  memcpy(gids, hg->v_gids, hg->nlocal_v * sizeof(ZOLTAN_ID_TYPE));

  /* local ids are optional */
  if(lid_size > 0 && lids != NULL) {
    for(int i=0; i < hg->nlocal_v; ++i) {
      lids[i] = i;
    }
  }
}


void hg_get_netsizes(
    void * data,
    int * num_lists,
    int * num_nonzeroes,
    int * format,
    int * ierr)
{
  hgraph const * const hg = (hgraph *) data;
  *ierr = ZOLTAN_OK;

  /* fill in hedge sizes */
  *num_lists = hg->nlocal_h;
  *num_nonzeroes = hg->nlocal_con;
  *format = ZOLTAN_COMPRESSED_EDGE;
}


void hg_get_hlist(
    void * data,
    int gid_size,
    int nhedges,
    int ncon,
    int format,
    ZOLTAN_ID_PTR h_gids,
    int * eptr,
    ZOLTAN_ID_PTR eind,
    int * ierr)
{
  hgraph const * const hg = (hgraph *) data;
  *ierr = ZOLTAN_OK;

  assert(gid_size == 1);

  /* sanity check */
  if((nhedges != hg->nlocal_h) || (ncon != hg->nlocal_con) ||
       (format != ZOLTAN_COMPRESSED_EDGE)) {
    *ierr = ZOLTAN_FATAL;
    return;
  }

  /* fill in hyperedge pointer info */
  memcpy(h_gids, hg->h_gids, nhedges * sizeof(ZOLTAN_ID_TYPE));
  memcpy(eptr, hg->eptr, nhedges * sizeof(int));

  /* fill in eind */
  memcpy(eind, hg->eind, ncon * sizeof(ZOLTAN_ID_TYPE));
}


