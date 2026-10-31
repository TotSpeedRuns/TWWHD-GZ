#ifndef GZ_GENERIC_HEAP_DOMAIN_CUT_H
#define GZ_GENERIC_HEAP_DOMAIN_CUT_H
#include "lifetime.h"
#include "object_binding_try.h"
enum { GHD_ROOTS=3, GHD_NODES=16, GHD_DEPTH=32, GHD_WORK=1048576 };
typedef struct { u32 game0,game1,zelda,reserved; } GhdLabels;
/* Capacities are sums of observed allocation rows, not unique physical bytes:
 * a parent child-backing allocation and its descendants overlap. No fit,
 * compression or native byte-readable conclusion follows from these totals. */
typedef struct {
 u32 label,node,heaps,live,ending,non_cpu,capacity_lo,capacity_hi;
 u32 requested_lo,requested_hi,managed_lo,managed_hi,reserved[2],world,read;
} GhdRoot;
typedef struct {
 HlHeap heap;
 HmScope scope;
 HlAllocation backing;
 u32 slot,parent_node,root_mask,backing_row;
} GhdNode;
typedef struct {
 u32 magic,bytes,version,nodes,global_heaps,global_allocations,selected_heaps,selected_allocations;
 u32 unknown_chain_heaps,unknown_chain_indexed,serial,release_serial,frame,flags,missing,session;
 GhdRoot root[GHD_ROOTS];
 GhdNode node[GHD_NODES];
} GhdCut;
typedef char GhdLabels16[sizeof(GhdLabels)==16?1:-1];
typedef char GhdRoot64[sizeof(GhdRoot)==64?1:-1];
typedef char GhdNode192[sizeof(GhdNode)==192?1:-1];
typedef char GhdCut3328[sizeof(GhdCut)==3328?1:-1];
/* Source proposal: metadata-only snapshot. One actual HL try lock. Caller owns
 * disjoint fixed high-DATA input16/output3328 and excludes other writers.
 * All failures leave output unchanged. Success is not native root origin,
 * a parentless SDK certificate, no-free/no-reuse, payload or WORLD/READ grant.
 * The fixed native producer must separately compare real resident labels
 * sampled OUTSIDE the gates before/after this cut. */
ObU32 _gz_ob_try_actor_heap_cut(const GhdLabels *,GhdCut *);
#endif
