#ifndef GZ_LAYER_BANK_H
#define GZ_LAYER_BANK_H
#include "birth_bank.h"
enum {LG_MAGIC=0x4c474231u,LG_CAP=128,LG_HEAD=96,LG_ROW=80,LG_BYTES=12384,LG_NO=0xffffffffu};
typedef struct {
 u32 index,layer,id,owner,parent,lists,list_count,pid;
 u32 profile,process_name,profile_entry,parameters,base_methods,node_methods,generation;
 u32 reserved[5];
} LgRow;
typedef struct {
 u32 magic,bytes,version,valid,count,actor_count,frame,epoch;
 u32 session,world_hash,world_sequence,stage0,stage1,room,layer,checksum;
 u32 registry_head,registry_tail,next_id,ambient,root,node_type,base_type,flags;
 LgRow rows[LG_CAP];u32 actor_layer[VR_CAPACITY];
} LgBank;
typedef char LgRowSize[sizeof(LgRow)==LG_ROW?1:-1];
typedef char LgBankSize[sizeof(LgBank)==LG_BYTES?1:-1];
extern LgBank _gz_lgb_work,_gz_lgb_staging;
extern volatile u32 _gz_lgb_ready;
u32 _gz_lgb_stage_capture(void),_gz_lgb_validate_ready(void),_gz_lgb_validate_current(void);
void _gz_lgb_commit(void),_gz_lgb_discard(void);
u32 _gz_lgb_validate(const LgBank*,u32,const VrWorld*,const BbBank*);
u32 _gz_lgb_archive_validate(u32,u32,u32);
/* Observed process generations and native registered layer membership are
 * diagnostic facts, never native allocator or replay/creation permission. */
#endif
