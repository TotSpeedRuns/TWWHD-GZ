#ifndef GZ_OWNED_HEAP_COMMON_DOMAIN_H
#define GZ_OWNED_HEAP_COMMON_DOMAIN_H
#include "owned_heap_image_version.h"
#include "heap_domain_cut.h"
#include "world_global_cut.h"
/* AS7 is a three-resident-root COMMON native domain, not old AS6/Game1.
 * These are PRIVATE codec checks. No successful scalar predicate supplies
 * an HG/Gate/CPU-read/native-writer certificate. */
enum { GHC_COMMON_VERSION=GZ_CURRENT_HEAP_IMAGE_VERSION,GHC_COMMON_ROOT_MASK=7,GHC_ENROLLED_HEAP=8,
 GHC_ALLOC_MAGIC=0x47484137u,GHC_ALLOC_VERSION=1,GHC_ALLOC_HEADER=64,
 GHC_ALLOC_ROW=88,GHC_ALLOC_ENDING=1 };
typedef struct {
 u32 magic,bytes,version,row_bytes,count,live,ending,flags;
 u32 root_node,root_slot,serial,release_serial,row_checksum,heap_count,bank_offset,bank_bytes;
} GhcAllocationDirectory;
typedef struct {HlAllocation allocation;u32 index,owner_slot;} GhcAllocationRow;
typedef char GhcAllocationHeader64[sizeof(GhcAllocationDirectory)==64?1:-1];
typedef char GhcAllocationRecord88[sizeof(GhcAllocationRow)==88?1:-1];
/* Validate every root path and its exact masks, then choose the first common
 * node while walking from genuine Game0 toward its parent. No address, size,
 * actor profile, host suggestion or first-fitting allocator selects a domain.
 * A common parentless SDK terminal is deliberately ineligible. */
static inline u32 ghc_common_node(const GhdCut*c,const GhdLabels*l){
 u32 i,r,at,seen,masks[GHD_NODES],roots=0;
 const u32*labels=(const u32*)l;
 if(!c||!l||l->reserved||!l->game0||!l->game1||!l->zelda||
  l->game0==l->game1||l->game0==l->zelda||l->game1==l->zelda||
  c->magic!=0x47484431u||c->bytes!=sizeof(*c)||c->version!=1||
  !c->nodes||c->nodes>GHD_NODES||c->flags!=2||c->missing!=31||c->session||
  c->unknown_chain_heaps||c->unknown_chain_indexed||!c->serial)return 0;
 for(i=0;i<GHD_NODES;i++)((volatile u32*)masks)[i]=0;
 for(i=0;i<c->nodes;i++){
  const GhdNode*n=c->node+i;u32 j;
  if(!n->slot||n->slot>HL_HEAPS||n->parent_node>c->nodes||n->parent_node==i+1||
   n->heap.state!=HL_LIVE||n->heap.kind!=HL_EXP||!n->heap.generation||
   !n->root_mask||(n->root_mask&~7u))return 0;
  for(j=0;j<i;j++)if(n->slot==c->node[j].slot||n->heap.heap==c->node[j].heap.heap)return 0;
  if(n->parent_node){const HlHeap*p=&c->node[n->parent_node-1].heap;
   if(n->heap.parent!=p->heap||n->heap.parent_generation!=p->generation||
    n->scope.parent_epoch!=p->epoch)return 0;
  }else{if(n->heap.parent||n->heap.parent_generation)return 0;++roots;}
 }
 if(roots!=1)return 0;
 for(r=0;r<3;r++){
  at=c->root[r].node;seen=0;
  if(!at||at>c->nodes||c->root[r].label!=labels[r]||
   c->node[at-1].heap.heap!=labels[r]||c->root[r].world||c->root[r].read)return 0;
  while(at){if(at>c->nodes||(seen&(1u<<(at-1))))return 0;
   seen|=1u<<(at-1);masks[at-1]|=1u<<r;at=c->node[at-1].parent_node;}
 }
 for(i=0;i<c->nodes;i++)if(masks[i]!=c->node[i].root_mask)return 0;
 at=c->root[0].node;
 while(at){if(masks[at-1]==7u)return c->node[at-1].parent_node?at:0;
  at=c->node[at-1].parent_node;}
 return 0;
}
static inline u32 ghc_allocation_offset(u32 heaps,u32 processes){
 u32 at=ghc_process_data(heaps,processes);return at&&at<=0xffffffe0u?(at+31u)&~31u:0;
}
static inline u32 ghc_common_legacy_data(u32 heaps,u32 processes,u32 allocations){
 u32 at=ghc_allocation_offset(heaps,processes),bytes;
 if(!at||allocations>HL_ALLOCS)return 0;
 bytes=GHC_ALLOC_HEADER+allocations*GHC_ALLOC_ROW;
 return at<=0xffffffe0u-bytes?(at+bytes+31u)&~31u:0;
}
static inline u32 ghc_common_data(u32 heaps,u32 processes,u32 allocations){
 u32 at=ghc_common_legacy_data(heaps,processes,allocations);
 return at&&at<=0x40000000u-WGC_BYTES?at+WGC_BYTES:0;
}
static inline u32 ghc_common_capacity(u32 raw,u32 processes,u32 allocations){
 u32 fixed=ghc_common_data(HL_HEAPS,processes,allocations),scratch=ghc_batch_bytes(processes),overhead;
 if(!fixed||!scratch||!raw||(raw&3u)||raw>0x40000000u)return 0;
 overhead=64u+(((raw-1u)>>16)+1u)*63u;
 return fixed<=0x40000000u-scratch&&overhead<=0x40000000u-fixed-scratch&&
  raw<=0x40000000u-fixed-scratch-overhead?fixed+raw+scratch+overhead:0;
}
static inline u32 ghc_common_bytes_equal(const void*a,const void*b,u32 bytes){
 const unsigned char*x=a,*y=b;u32 i;for(i=0;i<bytes;i++)if(x[i]!=y[i])return 0;return 1;
}
static inline u32 ghc_common_checksum(const void*p,u32 bytes){
 const unsigned char*b=p;u32 i,h=2166136261u;for(i=0;i<bytes;i++)h=(h^b[i])*16777619u;return h;
}
/* Heap rows are ordered by their genuine HL slot. AS7 uses the final word as
 * its immediate enrolled-parent slot (root has0); no native pointer follow. */
static inline const GhcHeapRow*ghc_common_heap_slot(const GhcHeapRow*r,u32 count,u32 slot){
 u32 lo=0,hi=count;while(lo<hi){u32 mid=lo+(hi-lo)/2;if(r[mid].slot<slot)lo=mid+1;else hi=mid;}
 return lo<count&&r[lo].slot==slot?r+lo:0;
}
static inline u32 ghc_common_image_format(const GhcHeader*h,const GhdCut*c,u32 selected){
 const unsigned char*p=(const unsigned char*)h;const GhcProcessDirectory*pd;
 const GhcAllocationDirectory*d;const GhcAllocationRow*r;const GhcHeapRow*heaps;
 u32 at,i,live=0,ending=0,last=0,root_rows=0;
 if(!h||(h->version!=GHC_COMMON_VERSION&&h->version!=GHC_LEGACY_COMMON_VERSION&&h->version!=GHC_LEGACY_PACKED_VERSION)||h->root_mask!=GHC_COMMON_ROOT_MASK||!selected||selected>c->nodes||
  !h->count||h->count>HL_HEAPS||h->table>h->bytes||h->count>(h->bytes-h->table)/sizeof(*heaps)||
  h->reserved[0]>h->bytes||sizeof(*pd)>h->bytes-h->reserved[0]||h->data>h->bytes)return 0;
 pd=(const GhcProcessDirectory*)(p+h->reserved[0]);heaps=(const GhcHeapRow*)(p+h->table);
 at=ghc_allocation_offset(h->count,pd->count);
 if(!at||at>h->bytes||sizeof(*d)>h->bytes-at)return 0;d=(const GhcAllocationDirectory*)(p+at);
 if(d->magic!=GHC_ALLOC_MAGIC||d->version!=GHC_ALLOC_VERSION||d->row_bytes!=sizeof(*r)||
  !d->count||d->count>HL_ALLOCS||d->bytes!=sizeof(*d)+d->count*sizeof(*r)||d->bytes>h->bytes-at||
  d->root_node!=selected||d->root_slot!=c->node[selected-1].slot||d->serial!=c->serial||d->release_serial!=c->release_serial||
  d->heap_count!=h->count||d->flags&~GHC_ALLOC_ENDING)return 0;
 if(h->version==GHC_COMMON_VERSION){
  u32 bank_at=ghc_common_legacy_data(h->count,pd->count,d->count);
  if(!bank_at||d->bank_offset!=bank_at||d->bank_bytes!=WGC_BYTES||
   h->data!=ghc_common_data(h->count,pd->count,d->count)||bank_at>h->bytes||WGC_BYTES>h->bytes-bank_at||
   _gz_wgc_validate(p+bank_at,WGC_BYTES,(const GhcSaveProducer*)(p+128),h->domain.heap,h->domain.heap_generation))return 0;
 }else if(d->bank_offset||d->bank_bytes||h->data!=ghc_common_legacy_data(h->count,pd->count,d->count))return 0;
 for(i=0;i<h->count;i++){
  const GhcHeapRow*row=heaps+i,*walk=row;u32 depth,bits=0;
  if(!row->slot||row->slot>HL_HEAPS||(i&&row->slot<=heaps[i-1].slot)||
   !(row->mask&GHC_ENROLLED_HEAP)||(row->mask&~15u)||row->heap[2]!=HL_LIVE||
   !row->heap[1]||!ghc_policy_contains(h->domain.start,h->domain.span,row->heap[0],row->header_bytes)||
   !ghc_policy_contains(h->domain.start,h->domain.span,row->heap[4],row->heap[5]))return 0;
  for(depth=0;depth<GHD_DEPTH;depth++){
   u32 label;
   for(label=0;label<3;label++)if(walk->slot==c->node[c->root[label].node-1].slot)bits|=1u<<label;
   if(!walk->reserved){if(walk->slot!=d->root_slot)return 0;break;}
   {const GhcHeapRow*parent=ghc_common_heap_slot(heaps,h->count,walk->reserved);
    if(!parent||walk->heap[6]!=parent->heap[0]||walk->heap[7]!=parent->heap[1]||
     walk->scope[0]!=parent->heap[15])return 0;walk=parent;}
  }
  if(depth==GHD_DEPTH||(row->mask&7u)!=bits)return 0;
  if(row->slot==d->root_slot){if(row->reserved||!ghc_common_bytes_equal(row->heap,&c->node[selected-1].heap,64)||
   !ghc_common_bytes_equal(row->scope,&c->node[selected-1].scope,32))return 0;++root_rows;}
 }
 if(root_rows!=1)return 0;
 r=(const GhcAllocationRow*)(d+1);
 for(i=0;i<d->count;i++){
  const HlAllocation*a=&r[i].allocation;const GhcHeapRow*owner;
  if(!r[i].index||r[i].index>HL_ALLOCS||r[i].index<=last||!r[i].owner_slot||r[i].owner_slot>HL_HEAPS||
   !a->generation||(a->state!=HL_LIVE&&a->state!=HL_ENDING)||a->missing!=31||a->reserved)return 0;
  last=r[i].index;owner=ghc_common_heap_slot(heaps,h->count,r[i].owner_slot);
  if(!owner||a->heap!=owner->heap[0]||a->heap_generation!=owner->heap[1]||a->heap_epoch!=owner->heap[15]||
   a->kind!=owner->heap[3]||!ghc_policy_contains(owner->heap[4],owner->heap[5],a->pointer,a->capacity)||
   (a->kind==HL_EXP&&a->block&&(a->block>a->pointer||a->pointer-a->block<16||!ghc_policy_contains(owner->heap[4],owner->heap[5],a->block,16))))return 0;
  if(a->state==HL_LIVE)++live;else ++ending;
 }
 if(live!=d->live||ending!=d->ending||live+ending!=d->count||d->flags!=(ending?(u32)GHC_ALLOC_ENDING:0u)||
  d->row_checksum!=ghc_common_checksum(r,d->count*sizeof(*r)))return 0;
 for(i=at+d->bytes;i<(h->version==GHC_COMMON_VERSION?d->bank_offset:h->data);i++)if(p[i])return 0;
 return 1;
}
#endif
