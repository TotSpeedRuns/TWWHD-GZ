#ifndef GZ_RESOURCE_PAYLOAD_BUNDLE_H
#define GZ_RESOURCE_PAYLOAD_BUNDLE_H
#include "source_directory.h"
typedef struct {
 u32 magic,bytes,version,valid,checksum,metadata_offset,metadata_bytes,index_offset;
 u32 count,payload_offset,payload_bytes,missing,session,writer_exclusion,reserved[2];
} SpHeader;
typedef struct {u32 allocation,offset,bytes,checksum;} SpEntry;
/* SPB1 packs only the used SCD1 sections, followed by a one-to-one payload
 * index and every selected allocation's complete observed capacity. Directory
 * offsets retain their SCD1 virtual layout; SPB1 explicitly maps the packed
 * sections. No heap recipes, borrowed leases, GPU/audio/thread or world-hold
 * certificates are manufactured. Archive integrity is NOT restore authority.
 * Caller supplies separate, unoccupied, leased staging storage; publication
 * and durable slot ownership belong to the actual held operation. A valid
 * SPB1 already at the destination is never overwritten by capture. */
u32 _gz_sp_capture(u32 object,u32 owner,void*image,u32 capacity,u32*written);
/* Validates metadata, payload hashes, object slices and table values without
 * reading source-selected native pointers or current lifetime records. */
u32 _gz_sp_validate(const void*image,u32 bytes);
#endif
