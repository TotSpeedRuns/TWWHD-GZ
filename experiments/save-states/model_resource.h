#ifndef GZ_POT_PRIMARY_MODEL_RESOURCE_H
#define GZ_POT_PRIMARY_MODEL_RESOURCE_H
typedef unsigned int u32;
typedef struct {char archive[12];u32 crc,index,flags,heap_estimate;} PmrRecipe;
typedef char PmrRecipeSize[sizeof(PmrRecipe)==28?1:-1];
extern const PmrRecipe _gz_pmr_recipes[16];
extern volatile u32 _gz_pr_resource_control,_gz_pt_epoch;
u32 _gz_gh_solid(u32 heap,u32 model,u32 bytes);
enum {PMR_OK=0,PMR_ACTOR=200,PMR_TYPE=201,PMR_HEAP=202,PMR_MODEL=203,
      PMR_CONTROL=204,PMR_POOL=205,PMR_NODE=206,PMR_MISSING=207,
      PMR_ARCHIVE=208,PMR_RESOURCE=209,PMR_CHANGED=210};
/* Current primary-model lookup only. Caller already owns a stable actor,
 * resource and heap Management boundary and proves genuine queued identity.
 * A zero result grants NO restore, lifetime, phase, BRK, effect or child
 * coverage. No actor, model, source, allocator or resource writes/calls.
 * The real heap helper may write only its fixed private identity result. */
u32 _gz_pmr_model(u32 actor,u32 live_pid,u32 epoch);
#endif
