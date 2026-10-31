#ifndef GZ_FOREST_SNAPSHOT_H
#define GZ_FOREST_SNAPSHOT_H
#include "heap_graph.h"
/* Explicit algorithm receipt, not a native exclusion or lifetime lease.
 * generation must change whenever ANY member heap is destroyed/recreated.
 * No native binding currently supplies this receipt. */
typedef struct {AgDomain graph;AgU32 generation,reserved[3];} AfDomain;
AgU32 _gz_af_capture(const AfDomain*,void*,AgU32 capacity,AgU32*written);
AgU32 _gz_af_validate(const AfDomain*,const void*,AgU32 bytes);
AgU32 _gz_af_restore(const AfDomain*,const void*,AgU32 bytes,
                     void*undo,AgU32 undo_capacity,AgU32*undo_bytes);
#endif
