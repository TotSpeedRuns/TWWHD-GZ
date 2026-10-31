#ifndef GZ_OPAQUE_HEAP_CHUNKS_H
#define GZ_OPAQUE_HEAP_CHUNKS_H
#include <stdint.h>
#define oc_measure _gz_och_measure
#define oc_encode _gz_och_encode
#define oc_validate _gz_och_validate
#define oc_decode_page _gz_och_decode_page
#define oc_read _gz_och_read
#define oc_matches _gz_och_matches

#ifdef _WIN32
#define OC_API __declspec(dllexport)
#else
#define OC_API
#endif
 enum {OC_OK=1,OC_ARGUMENT=3,OC_SPACE=4,OC_CORRUPT=5,OC_DIFFERENT=6,
      OC_PAGE=0x200000,OC_ENCODE_PAGE=0x10000,OC_MAX=0x40000000};
/* Pure caller-owned immutable-byte codec. No allocation, native pointer
 * following, actor policy, lifetime/coherence grant or world-state writer.
 * New packets use 64KiB pages. Readers also accept legacy 2MiB pages.
 * Every logical byte is represented, including raw incompressible pages.
 * Failure never publishes a nonzero written/valid result. On malformed input,
 * page decode leaves the destination unchanged. All bounds are explicit.
 * Capturing a native domain requires the actual producer's hold/exclusions;
 * these functions alone supply none of those permissions. */
OC_API uint32_t oc_measure(const void*,uint32_t,uint32_t*);
OC_API uint32_t oc_encode(const void*,uint32_t,void*,uint32_t,uint32_t*);
OC_API uint32_t oc_validate(const void*,uint32_t,uint32_t);
/* Decode at most2MiB to unpublished caller-owned scratch, for eventual
 * bounded native apply/inverse. This does not require a full raw-domain copy. */
OC_API uint32_t oc_decode_page(const void*,uint32_t,uint32_t,uint32_t,
                             void*,uint32_t,uint32_t*);
/* Extract private logical bytes without allocating a whole raw-domain copy.
 * All affected pages are checked before the first output store. */
OC_API uint32_t oc_read(const void*,uint32_t,uint32_t,uint32_t,void*,uint32_t);
OC_API uint32_t oc_matches(const void*,uint32_t,uint32_t,const void*);
#endif
