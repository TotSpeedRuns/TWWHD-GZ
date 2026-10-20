#ifndef GZ_TYPE1_POT_RESOURCE_CHECKS_H
#define GZ_TYPE1_POT_RESOURCE_CHECKS_H
typedef unsigned int u32;
enum {
 PR_OK=0, PR_BASE_PREFLIGHT=0x100, PR_BAD_PHASE=0x120,
 PR_BAD_CALLBACK=0x121, PR_BAD_MODEL=0x122, PR_BAD_CONTROL=0x123,
 PR_BAD_POOL=0x124, PR_BAD_NODE=0x125, PR_NO_ALWAYS=0x126,
 PR_BAD_ALWAYS=0x127, PR_BAD_RESOURCE=0x128, PR_BAD_HEAP=0x129,
 PR_CHANGED=0x12a
};
/* Actual owned-image addresses are linker aliases; source reconstruction
 * function/global addresses are not substituted for the original image. */
extern volatile u32 _gz_pr_resource_control;
extern volatile u32 _gz_pt_epoch;
u32 _gz_pt_preflight(u32 actor,u32 packet,u32 live_pid,u32 epoch);
u32 _gz_gh_solid(u32 heap,u32 model,u32 model_bytes);
/* Read-only leaf, only for a genuine queued completed Type1 WAIT actor.
 * Reuses scalar/self/child preflight. Heap helper writes its fixed private
 * identity only; there are no actor/resource/native writes or native calls.
 * Zero certifies these current fields, NOT birth/layer/ABA/destructor facts. */
u32 _gz_pr_ready(u32 actor,u32 packet,u32 live_pid,u32 epoch);
enum {
 PR_MISSING_ALLOCATION_GENERATION=1,
 PR_MISSING_SHARED_RESOURCE_LIFETIME=2,
 PR_MISSING_PARTICLE_GLOBAL_BACKREFS=4,
 PR_MISSING_DESTRUCTOR_RECEIPT=8
};
u32 _gz_pr_missing(void);
#endif
