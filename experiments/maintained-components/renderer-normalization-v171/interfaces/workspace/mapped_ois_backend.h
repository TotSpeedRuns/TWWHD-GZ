#ifndef GZ_MAPPED_OIS_BACKEND_H
#define GZ_MAPPED_OIS_BACKEND_H
#include "workspace_overlay_private.h"
#include "mapped_api.h"
typedef struct {
    GzMmCache cache;
    uint32_t live, gate;
    WsRegion rejected;
    volatile uint32_t frame_state;
    uint32_t sdk_heap, sdk_live;
} GzMmOisOwner;
extern GzMmOisOwner _gz_ois_mm_backend;
uint32_t _gz_ois_mm_allocate_part0(WsRegion *,uint32_t);
uint32_t _gz_ois_mm_stage_allocate(WsRegion *,uint32_t);
uint32_t _gz_ois_mm_video_allocate(WsRegion *,uint32_t);
void _gz_ois_mm_release(WsRegion *);
uint32_t _gz_ois_mm_budget(uint32_t);
uint32_t _gz_ois_mm_private_apart(uint32_t,uint32_t);
uint32_t _gz_ois_mm_heap_private_apart(uint32_t,uint32_t);
/* Caller must establish the normal main-thread owner cleanup seam. This only
 * checks actual OIS state/counts; it does not create application quiescence. */
uint32_t _gz_ois_mm_close_empty(void);
enum {GZ_OIS_SDK_WORK_MAX=16u*1024u*1024u};
/* Exact CURRENT/RESERVED Cut.work only. Allocation is pre-hold scope1;
 * release is pre-hold scope1 or clean released scope3. The returned Region
 * stays in Cut.work and is never counted/freed as a mapped allocation. */
uint32_t _gz_ois_sdk_work_allocate(uint32_t capacity);
uint32_t _gz_ois_sdk_work_release(void);
#endif
