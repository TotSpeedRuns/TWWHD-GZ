#ifndef GZ_ARENA_HEAP_GRAPH_H
#define GZ_ARENA_HEAP_GRAPH_H
typedef unsigned int AgU32;
enum { AG_MAX_HEAPS=8,AG_MAX_OBJECTS=64,AG_REQUIRED=31,
 AG_OK=0,AG_ARGUMENT=1,AG_SCOPE=2,AG_LAYOUT=3,AG_LIMIT=4,AG_SPACE=5,
 AG_CORRUPT=6,AG_CHANGED=7,AG_BUSY=8 };
/* Algorithm-only ownership inputs. A token/flag value does not itself acquire
 * an engine/allocator/JPA/audio lease. There is no native admission binding. */
typedef struct {AgU32 root,parent,session,token,obligations,reserved[3];} AgDomain;
typedef struct {
 AgU32 heap,parent,start,span,vtable,kind,direction,flags;
 AgU32 child_first,child_count,object_first,object_count;
} AgHeap;
typedef struct {AgU32 object,owner,vtable,in_own_heap;} AgObject;
/* Compact ordered ownership graph. No mutex/thread bytes or allocation
 * payloads are copied by these APIs. Unknown heap types are recorded as kind0
 * so coverage cannot silently omit their objects; they need separate recipes. */
AgU32 _gz_ag_capture(const AgDomain*,void*staging,AgU32 capacity,AgU32*written);
AgU32 _gz_ag_validate(const AgDomain*,const void*,AgU32 bytes);
AgU32 _gz_ag_matches_live(const AgDomain*,const void*,AgU32 bytes);
/* Find the smallest containing registered arena for a complete object range.
 * It is an arena association, not proof of an allocated block's generation. */
AgU32 _gz_ag_owner(const AgDomain*,const void*,AgU32 bytes,
                  AgU32 object,AgU32 size,AgU32*heap_index);
/* Buffer classification only; not a native ownership certificate. */
AgU32 _gz_ag_buffer_private(AgU32,AgU32);
#endif
