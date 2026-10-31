#ifndef GZ_SAVED_RESOURCE_REFERENCES_H
#define GZ_SAVED_RESOURCE_REFERENCES_H
#include "resource_payload.h"
typedef struct {
 u32 magic,bytes,version,valid,code,source_checksum,objects,known_objects;
 u32 members,owned_views,borrowed_views,unresolved_references,table_headers,pending_derived,missing,reserved;
} SpReferenceReport;
/* Validate the two native buffer members of four evidenced resource types
 * from saved bytes only: active views must name a complete retained buffer,
 * descriptor-table counts must fit their retained six-byte records. No saved
 * native pointer is dereferenced, even after its original allocation expires.
 * valid acknowledges this inventory only. Derived-object, GPU/cache and
 * external borrowed-resource closure remain pending, with missing31. Source
 * and output must be caller-owned mapped private buffers; this is not a lease
 * provider or permission to reconstruct a resource or publish a game state. */
u32 _gz_sp_references(const void *image,u32 bytes,SpReferenceReport *out);
#endif
