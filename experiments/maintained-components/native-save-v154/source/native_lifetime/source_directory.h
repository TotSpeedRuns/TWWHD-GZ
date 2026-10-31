#ifndef GZ_SOURCE_RESOURCE_DIRECTORY_H
#define GZ_SOURCE_RESOURCE_DIRECTORY_H
#include "resource_dependencies.h"
/* Private inventory bounds, not claims about a full-game maximum. */
enum {SC_ALLOCS=4096,SC_OBJECTS=8192,SC_EDGES=49152,SC_REGISTRY=32768};
enum {SC_OK=0,SC_ARGUMENT=1,SC_OBSERVER=2,SC_TYPE=3,SC_FIELD=4,
 SC_UNTRACKED=5,SC_REGISTRY_ERROR=6,SC_CHANGED=7,SC_CAPACITY=8,
 SC_ANCESTRY=9,SC_IMAGE=10,SC_BUSY=11,SC_UNAVAILABLE=12};
typedef struct {
 HlHeap identity;HmScope scope;HlAllocation managed,header;
 HlCustomMeta custom;HlAllocation custom_anchor;
 u32 native_flags,first,last,count,stride,registry_begin,reserved;
} ScHeap;
typedef struct {HlAllocation identity;u32 object_begin,object_count,reserved[2];} ScAllocation;
typedef struct {
 u32 object,owner,allocation,offset,vtable,provider,edge_begin,edge_count;
 unsigned char raw[72];u32 table[2],reserved[4];
} ScObject;
typedef struct {u32 object,slot,role,field,value,allocation;} ScEdge;
typedef struct {
 u32 object,owner,storage_heap,storage_pointer,storage_generation,storage_capacity;
 u32 vtable,previous,next,reserved[3];
} ScRegistry;
typedef struct {
 u32 magic,bytes,version,valid,code,missing,serial,release_serial,frame,active_allocations;
 u32 heap_count,allocation_count,object_count,edge_count,unknown_count,registry_count;
 u32 seed_object,seed_owner,seed_allocation,heaps_offset,allocations_offset,objects_offset,edges_offset,registry_offset;
 u32 checksum,used_bytes,owned_complete,borrowed_complete,payloads_complete,writer_exclusion,session,reserved;
} ScHeader;
typedef struct {
 ScHeader header;ScHeap heaps[HL_HEAPS];ScAllocation allocations[SC_ALLOCS];
 ScObject objects[SC_OBJECTS];ScEdge edges[SC_EDGES];ScRegistry registry[SC_REGISTRY];
} ScDirectory;
/* Process-owned SDK storage, allocated once outside the metadata lock. The
 * directory layout and every capacity remain unchanged. Never serialized as
 * a pointer or freed/replaced during a game session. */
extern ScDirectory *_gz_sc_storage;
#define _gz_sc_directory (*_gz_sc_storage)
u32 _gz_sc_storage_ready(void);
void _gz_sc_storage_tick(void);
extern u32 _gz_scs_report[16];
/* Capture source-time heap ancestry, native registry membership, every object
 * in each selected allocation, and recursively owned edges of known providers.
 * One fixed output, metadata only, common observer lock, no SDK destruction.
 * Unknown providers are retained and keep owned_complete false. Borrowed
 * resources, payloads and actual engine/render exclusion remain absent. */
u32 _gz_sc_capture(u32 object,u32 owner_heap);
/* Validate an immutable copied directory without reading ANY captured native
 * object, allocation, heap header, or current lifetime record. Still no replay
 * authority: missing31, session0 and the three incomplete domains stay set. */
u32 _gz_sc_validate(const void *image,u32 bytes);
/* Internal overlap exclusion shared by other snapshot writers. */
u32 _gz_sc_private(u32 pointer,u32 bytes);
#endif
