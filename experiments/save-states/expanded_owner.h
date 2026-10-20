#ifndef GZ_EXPANDED_HEAP_CURRENT_OWNER_H
#define GZ_EXPANDED_HEAP_CURRENT_OWNER_H
#include "heap_owner.h"
enum { GE_OK=0,GE_CLOSURE=69,GE_MAX_BLOCKS=4096 };
typedef struct {
 u32 valid,heap,parent,vtable,start,bytes,direction;
 u32 block,block_bytes,padding,allocation,requested,capacity;
 u32 used_count,used_head,used_tail,free_count,free_head,free_tail,reserved;
} GeIdentity;
typedef char GeIdentitySize[sizeof(GeIdentity)==80?1:-1];
extern volatile GeIdentity _gz_ge_identity;
/* Exact current forward ExpHeap used-block membership only. The allocation
 * must begin at its native return pointer and the whole requested extent must
 * fit. Registry membership precedes heap-header reads; no allocation contents
 * are read. Only fixed private result/stack are written. Caller must own the
 * stable heap/lifetime boundary. There is no generation or reclamation proof. */
u32 _gz_ge_current(u32 heap,u32 allocation,u32 requested_bytes);
#endif
