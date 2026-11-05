#ifndef GZ_OWNED_HEAP_PROCESS_DIRECTORY_H
#define GZ_OWNED_HEAP_PROCESS_DIRECTORY_H
#include "owned_heap_image_version.h"
#include "process.h"
#include "lifetime.h"
#include "owned_heap_save.h"
/* AS7 diagnostic directory only. All records are observations. This header
 * supplies no lock, native pointer follow, replay or writer permission. */
enum { GHCP_MAGIC=0x47504331u,GHCP_VERSION=1,GHCP_IMAGE_VERSION=GZ_CURRENT_HEAP_IMAGE_VERSION,
 GHCP_BIND_NONE=0,GHCP_BIND_LIVE=1,GHCP_BIND_OUTSIDE=2,
 GHCP_UNBOUND=1,GHCP_PO_GAP=2,GHCP_BUDGET=4,GHCP_WORK=1048576 };
typedef struct {
 PoEntry process;HlAllocation allocation;HlHeap owner;HmScope scope;
 u32 po_index,binding,heap_index,reserved;
} GhcProcessRow;
typedef struct {
 u32 magic,bytes,version,row_bytes,count,live,ending,ended,bound,outside,unbound,flags;
 PoHeader po_before,po_after;HlControl hl_before,hl_after;GhcSaveScene scene;
 u32 row_offset,row_checksum,work_used,reserved;
} GhcProcessDirectory;
typedef char GhcProcessRow256[sizeof(GhcProcessRow)==256?1:-1];
typedef char GhcProcessDirectory352[sizeof(GhcProcessDirectory)==352?1:-1];
/* Directory rows are every occupied PO slot in ascending 0-based po_index.
 * LIVE means a genuine current metadata binding inside the captured domain;
 * OUTSIDE retains its genuine HL tuple but has no captured payload. NONE has
 * zero allocation/owner/scope/heap_index. ENDED and ENDING are always NONE.
 * PID0 and hashes0 remain valid opaque values. Before/after headers retain
 * busy1 as observed while THIS metadata lock is held, never as authority.
 * Unbound/budget/gap flags preserve incomplete identity coverage explicitly.
 * missing255 and ordinary Source limits are unchanged. */
/* Pure scalar validation, shared by the native adapter and ROOT host cases.
 * A numerically valid historical address is never followed by this helper. */
static inline u32 ghcp_scene_hash(const GhcSaveScene*s){u32 i,h=2166136261u;
 for(i=0;i<6;i++)h=(h^((const u32*)s)[i])*16777619u;return h;}
static inline u32 ghcp_process_format(const PoEntry*r){return r&&r->object&&r->generation&&
 r->state>=PO_LIVE&&r->state<=PO_ENDED&&r->profile<484&&r->pid<0xfffffffeu&&!r->init_state&&
 r->base_bytes>=0xb8u&&!(r->base_bytes&3u)&&!(r->other_bytes&3u)&&
 r->other_bytes<=0xffffffffu-r->base_bytes&&r->allocation_bytes==r->base_bytes+r->other_bytes&&
 !(r->object&3u)&&r->object>=0x10000000u&&r->object<0xc0000000u&&r->allocation_bytes&&
 r->allocation_bytes<=0xc0000000u-r->object;}
#endif
