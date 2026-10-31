#ifndef GZ_NATIVE_RESOURCE_DESTRUCTION_DEPENDENCIES_H
#define GZ_NATIVE_RESOURCE_DESTRUCTION_DEPENDENCIES_H
#include "lifetime.h"
enum {RD_OK=0,RD_ARGUMENT=1,RD_OBSERVER=2,RD_TYPE=3,RD_FIELD=4,RD_UNTRACKED=5,RD_REGISTRY=6,RD_CHANGED=7,RD_BUSY=8};
enum {RD_OWNED_ARRAY=1,RD_OWNED_TABLE=2,RD_OWNED_TABLE_DATA=3};
typedef struct {u32 slot,role,field,value;HlObservation identity;} RdEdge;
typedef struct {
 u32 magic,bytes,version,valid,code,object,owner,vtable,edge_count,serial,release_serial,missing,table_value[2],reserved[2];
 HlObservation root;
 unsigned char object_bytes[72];
 RdEdge edges[6];
} RdReport;
extern RdReport _gz_rd_report;
/* Diagnostic dependency inventory for FOUR evidenced EUR destructor families.
 * Resolves every owned pointer touched by their common two-buffer cleanup.
 * Metadata is copied while holding the common observer lock; object/table
 * bytes are rechecked. This is NOT complete object/borrowed-resource closure,
 * actual world exclusion, a source snapshot, destructor permission or restore
 * admission. Every identity retains missing31/session0. Output is fixed private
 * storage; callers must inspect the returned code and consume it without yield.
 * No source-selected function pointers or caller-chosen output address. */
u32 _gz_rd_inspect(u32 object,u32 owner_heap);
#endif
