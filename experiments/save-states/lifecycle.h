#ifndef GZ_LIFECYCLE_PLAN_H
#define GZ_LIFECYCLE_PLAN_H
#include "birth_bank.h"
enum {LP_MAGIC=0x4c535031u,LP_CAP=1024,LP_NO=0xffffffffu};
enum {LP_SURVIVE=1,LP_REBIND=2,LP_CREATE=3,LP_REMOVE=4,LP_AMBIGUOUS=5,LP_PLAYER=6,LP_HOLD_AMBIGUOUS=7};
enum {LP_AREA=1,LP_AMBIG=2,LP_PARENT=4,LP_CYCLE=8,LP_EXTERNAL=16,LP_RETAINED_CHILD=32};
enum {LP_PARENT_NONE=0,LP_PARENT_ZERO=1,LP_PARENT_INTERNAL=2,LP_PARENT_EXTERNAL=3};
typedef struct {
 u32 kind,source_index,live_index,source_pid,live_pid,live_actor,source_generation,live_generation;
 u32 candidate_count,parent_kind,parent_index,flags;
} LpRow;
typedef struct {
 u32 magic,bytes,version,valid,result,source_count,live_count,row_count;
 u32 frame,source_epoch,live_epoch,source_hash,live_hash,source_bank_hash,flags,checksum;
 u32 surviving_count,rebind_count,create_count,remove_count,ambiguous_count,held_count,owned_player_count;
 u32 external_parent_count,parent_rebind_count,create_order_count,source_session,live_session,source_sequence,live_sequence;
 u32 runtime_validation_required,mutation_permission;
 u32 remove_order_count,retained_child_count,reserved0,reserved1;
 LpRow rows[LP_CAP];u32 create_order[VR_CAPACITY],remove_order[VR_CAPACITY];
} LpPlan;
typedef char LpRowSize[(sizeof(LpRow)==48)?1:-1];
typedef char LpPlanSize[(sizeof(LpPlan)==53392)?1:-1];
extern LpPlan _gz_lsp_work;
/* Pure typed inventory difference. Native addresses are values only. Output is
 * fixed private diagnostic storage, never an actor or caller destination.
 * valid means a complete diagnostic plan, NOT execution/lifetime permission. */
u32 _gz_lsp_plan(const VrWorld*,const BbBank*,const VrWorld*,const PoEntry*);
u32 _gz_lsp_hash(const LpPlan*);
u32 _gz_lsp_build(u32 token,u32 frame);
extern VrWorld _gz_lsp_live;
extern PoEntry _gz_lsp_processes[VR_CAPACITY];
#endif
