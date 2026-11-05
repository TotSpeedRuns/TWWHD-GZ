#ifndef GZ_NORMALIZED_SLOT_REGISTRY_H
#define GZ_NORMALIZED_SLOT_REGISTRY_H
#include <stdint.h>
#include "../render-current-graph-preflight-r2/lifetime_witness.h"

/* Fixed private order receipt, written only along the compiled capture
 * wrapper's guard -> genuine reset -> exporter path. No caller object is
 * adopted. The wrapper provides _gz_rcl_normalized_scope(req,frame). */
typedef struct {
 uint32_t request,frame,stage,staging,capacity,hg,role,active;
} RnsOrder;
extern RnsOrder _gz_rns_order;
enum {RNS_CLAIMED=1,RNS_GUARDED=2,RNS_RESET_RETURNED=3,RNS_EXPORTED=4};

/* These functions authenticate the existing OIS owner before reading its
 * slot/pin metadata. They grant no GPU, allocator, graph or write authority.
 * All return the existing GHC result namespace (zero is success). */
uint32_t _gz_rns_current_begin(uint32_t request,uint32_t frame);
uint32_t _gz_rns_prefix_claim(uint32_t request,uint32_t frame,
 uint32_t pointer,uint32_t capacity);
uint32_t _gz_rns_saved_ready(uint32_t request,uint32_t frame);
uint32_t _gz_rns_saved_match(uint32_t request,uint32_t frame);
uint32_t _gz_rns_capture_done(uint32_t request,uint32_t frame,
 uint32_t written,uint32_t capture_result);
/* Called inside the genuine finish owner, after final slot publication and
 * compaction, before heap/state ownership is released. */
uint32_t _gz_rns_publish(uint32_t request,uint32_t frame);
/* Final clean GHW boundary, before dirty/APPLY. Zero means exact config. */
uint32_t _gz_rns_held_config(uint32_t request,uint32_t frame);
uint32_t _gz_rns_private_apart(uint32_t pointer,uint32_t bytes);
#endif
