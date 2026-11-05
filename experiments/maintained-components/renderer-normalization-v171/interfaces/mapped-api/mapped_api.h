#ifndef GZ_MAPPED_API_COMPONENT_H
#define GZ_MAPPED_API_COMPONENT_H
#include <stdint.h>

/* Component boundary only: the existing OIS owner must protect this cache and
 * its API snapshots, establish the application epoch, and retire every mapped
 * Region before closing. This cache is not allocation provenance or authority.
 */
typedef void *(*GzMmAllocFn)(uint32_t bytes, int32_t alignment);
typedef void (*GzMmFreeFn)(void *pointer);
typedef void *(*GzMmFindHeapFn)(void *pointer);

typedef struct {
    void *mapped_module;
    void *core_module;
    GzMmAllocFn generic_alloc;
    GzMmAllocFn video_alloc;
    GzMmFreeFn mapped_free;
    GzMmFindHeapFn find_heap;
} GzMmApi;

enum {
    GZ_MM_EMPTY = 0, GZ_MM_RESOLVING = 1, GZ_MM_READY = 2,
    GZ_MM_UNAVAILABLE = 3, GZ_MM_CLOSING = 4, GZ_MM_CLOSED = 5
};
enum {
    GZ_MM_OK = 0, GZ_MM_ARGUMENT = 1, GZ_MM_BUSY = 2,
    GZ_MM_MISSING = 3, GZ_MM_CORRUPT = 4
};
typedef struct {
    uint32_t epoch;
    volatile uint32_t phase;
    GzMmApi api;
} GzMmCache;

/* begin() accepts only a genuinely fresh, all-zero caller-owned object. It
 * cannot reset any prior epoch, including an unavailable or closed cache.
 */
uint32_t gz_mm_begin(GzMmCache *cache, uint32_t epoch);
uint32_t gz_mm_resolve(GzMmCache *cache, uint32_t epoch);
/* snapshot() requires independent caller-owned output outside the cache. The
 * caller must hold its real owner/operation exclusion until done with it.
 */
uint32_t gz_mm_snapshot(const GzMmCache *cache, uint32_t epoch, GzMmApi *output);
/* live_regions and active_users are authoritative owner inputs, not evidence
 * created by this resolver. Passing zero does not itself prove quiescence.
 * After close no previously borrowed API snapshot may be invoked.
 */
uint32_t gz_mm_close(GzMmCache *cache, uint32_t epoch,
                     uint32_t live_regions, uint32_t active_users);

#endif
