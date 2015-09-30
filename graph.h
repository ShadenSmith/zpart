#ifndef ZPART_HGRAPH_H
#define ZPART_HGRAPH_H


/******************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <zoltan.h>


/******************************************************************************
 * STRUCTURES
 *****************************************************************************/

#define hgraph zpart_hgraph
/**
* @brief Distributed hypergraph structure used by Zoltan (PHG) for
*        partitioning.
*/
typedef struct
{
  ZOLTAN_ID_TYPE nglobal_v;   /** Number of vertices in the global hgraph. */
  ZOLTAN_ID_TYPE nglobal_h;   /** Number of hedges in the global hgraph. */
  ZOLTAN_ID_TYPE nglobal_con; /** Sum of vertices in all hedges, globally. */

  int nlocal_v;   /** Number of vertices in the local hgraph. */
  int nlocal_h;   /** Number of hedges in the local hgraph. */
  int nlocal_con; /** Sum of vertices in all hedges, locally. */
  int * eptr;     /** eptr[h]:eptr[h+1] index into eind for hedge 'h' */

  ZOLTAN_ID_TYPE * v_gids;  /** Global id's of local vertices. */
  ZOLTAN_ID_TYPE * h_gids;  /** Global id's of local hedges. */
  ZOLTAN_ID_TYPE * eind;    /** Global id's of local vertices, per hedge. */

#if 0
  int numMyVertices;  /* number of vertices that I own initially */
  ZOLTAN_ID_TYPE *vtxGID;        /* global ID of these vertices */

  int numMyHEdges;    /* number of my hyperedges */
  int numAllNbors; /* number of vertices in my hyperedges */
  ZOLTAN_ID_TYPE *edgeGID;       /* global ID of each of my hyperedges */
  int *nborIndex;     /* index into nborGID array of edge's vertices */
  ZOLTAN_ID_TYPE *nborGID;  /* Vertices of edge edgeGID[i] begin at nborGID[nborIndex[i]] */
#endif
} hgraph;



/******************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/

#define hgraph_alloc zpart_hgraph_alloc
/**
* @brief Allocate structures for a distributed hypergraph.
*        NOTE: No fields are filled, only pointers allocated!
*
* @param local_vtxs The number of vertices to store locally.
* @param local_hedges The number of hyperedges to store locally.
* @param local_connections The number of local connections.
*
* @return An allocated hgraph structure which must be freed with hgraph_free().
*/
hgraph * hgraph_alloc(
    int local_vtxs,
    int local_hedges,
    int local_connections);


#define hgraph_free zpart_hgraph_free
/**
* @brief Free all memory allocated from hgraph_alloc().
*
* @param hg The hypergraph to free.
*/
void hgraph_free(
    hgraph * const hg);


#endif
