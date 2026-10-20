#ifndef GZ_HELD_ITEM_RECIPE_H
#define GZ_HELD_ITEM_RECIPE_H
#include "birth_bank.h"
enum {HIR_MAGIC=0x48495231u,HIR_NONE=0,HIR_MODEL=1,HIR_FAST=2,
 HIR_SOURCE=301,HIR_ITEM=302,HIR_SCENE=303,HIR_PROCESS=304,HIR_KEEP=305,HIR_RECIPE=306,HIR_CHANGED=307};
enum {HIR_CREATE=1,HIR_RETIRE=2,HIR_REBIND=4,HIR_MODEL_CHANGE=8,HIR_CHILD_REPLAY=16,HIR_OTHER_KEEPS=32};
/* Immutable constructor/correspondence evidence only. Saved process, model,
 * callback and APP addresses are opaque audit values, never dereferenced.
 * No valid bit in this record grants lifetime, SDK or replay authority. */
typedef struct {
 u32 magic,bytes,version,valid,token,frame,source_kind,serial;
 u32 source_player,source_world,source_bank,source_hash,world_hash;
 u32 source_item,source_model,source_kind_item,source_link,source_link_pid,source_link_generation;
 u32 source_keeps[4],source_index,source_profile;
 PoEntry source_child;BcRow source_request;
 u32 current_valid,current_item,current_model,current_kind_item,current_keeps[4];
 u32 current_keep_actors[4],current_profile,current_index,disposition,process_serial;
 PoEntry current_link,current_child;BcRow current_request;
} HirPlan;
extern HirPlan _gz_hir_result;
u32 _gz_hir_prepare_source(u32 token,u32 frame);
u32 _gz_hir_resolve_current(u32 token,u32 frame);
u32 _gz_hir_read(u32 token,u32 frame);
#endif
