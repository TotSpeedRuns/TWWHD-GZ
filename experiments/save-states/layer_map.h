#ifndef GZ_SAVED_LAYER_MAPPING_H
#define GZ_SAVED_LAYER_MAPPING_H
#include "layer_bank.h"
enum {LM_MAGIC=0x4c4d5031u,LM_NO=0xffffffffu,LM_BYTES=6240};
enum {LM_ROOT=1,LM_SURVIVE=2,LM_SEMANTIC=3,LM_MISSING=4,LM_AMBIGUOUS=5,LM_PARENT_BLOCKED=6,LM_SCENE_BLOCKED=7};
enum {LM_SCENE=1,LM_AMBIG=2,LM_ABSENT=4,LM_PARENT=8};
typedef struct {
 u32 source_index,current_index,source_layer,current_layer;
 u32 source_owner,current_owner,source_generation,current_generation;
 u32 kind,required,parent_source,candidates;
} LmRow;
typedef struct {
 u32 magic,bytes,version,valid,result,source_count,current_count,flags;
 u32 source_hash,current_hash,source_frame,current_frame,source_epoch,current_epoch,session,checksum;
 u32 bound_count,missing_count,ambiguous_count,blocked_count,required_count,required_unbound,runtime_validation_required,mutation_permission;
 LmRow rows[LG_CAP];
} LmPlan;
typedef char LmRowSize[sizeof(LmRow)==48?1:-1];
typedef char LmPlanSize[sizeof(LmPlan)==LM_BYTES?1:-1];
extern LmPlan _gz_lgm_work;
/* Structural candidates only. Both inputs must have complete World and
 * original-constructor banks. No saved native pointer is dereferenced, no
 * ambient/currentLayer fallback exists, and no actor API is called.
 * A current registry/lifetime lease is still required at any future use. */
u32 _gz_lgm_plan(const LgBank*,const VrWorld*,const BbBank*,const LgBank*,const VrWorld*,const BbBank*);
u32 _gz_lgm_hash(const LmPlan*);
#endif
