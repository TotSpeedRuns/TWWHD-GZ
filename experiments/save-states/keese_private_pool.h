#ifndef GZ_KEESE_PRIVATE_POOL_H
#define GZ_KEESE_PRIVATE_POOL_H
/* GZ owns this coreinit allocation for the lifetime of this module. It is
 * separate from game sead heap observations. Only the fixed allocator call
 * below can create this receipt; there is no pointer-registration API. */
typedef struct {
 u32 magic,bytes,valid,generation,pointer,heap,allocation_bytes,alignment;
 u32 thread,born_frame,return_site,checksum,reserved[28];
} KmpIdentity;
typedef char KmpIdentitySize[sizeof(KmpIdentity)==160?1:-1];
extern KmpIdentity _gz_kmp_identity;
u32 _gz_kmp_allocate(void),_gz_kmp_validate(void),_gz_kmp_private_apart(u32,u32);
#endif
