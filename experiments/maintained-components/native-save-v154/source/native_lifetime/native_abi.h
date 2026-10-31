#ifndef GZ_NATIVE_ABI_METADATA_H
#define GZ_NATIVE_ABI_METADATA_H
/* Native ABI address classification only. This does not acquire a world,
 * allocator/resource lease or authorize a heap root/lifetime incarnation. */
unsigned int _gz_abi_metadata_span(unsigned int address,unsigned int bytes);
/* Optional fixed image-owner alias guard only. Default1 means no such owner
 * is linked, not that a buffer is mapped/owned or a world restore is allowed. */
unsigned int _gz_image_write_allowed(unsigned int address,unsigned int bytes);
#endif
