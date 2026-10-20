#ifndef GZ_GRAPPLE_STATE_H
#define GZ_GRAPPLE_STATE_H
#include "birth_bank.h"
enum {HRP_CAP=8,HRP_HEADER=128,HRP_ROW=8192,HRP_BYTES=HRP_HEADER+HRP_CAP*HRP_ROW,HRP_MAGIC=0x48525031u};
typedef struct {u32 audit,pid,profile,generation,world_index,kind;} HrpRef;
typedef struct {
 u32 world_index,flags,scalar_bytes,ref_count;
 PoEntry process;BcRow request;
 u32 layout,checksum,mode,point_count,active_targets,model_audit[4],reserved[3];
 u8 scalars[512],points[4000];HrpRef refs[103];u8 tail[968];
} HrpRow;
typedef struct {
 u32 magic,bytes,version,valid,count,frame,epoch,session,stage0,stage1,room,layer;
 u32 world_hash,world_sequence,checksum,layout,reserved[16];HrpRow rows[HRP_CAP];
} HrpBank;
typedef char HrpRowSize[sizeof(HrpRow)==HRP_ROW?1:-1];
typedef char HrpBankSize[sizeof(HrpBank)==HRP_BYTES?1:-1];
extern HrpBank _gz_hrp_source,_gz_hrp_stage;
u32 _gz_hrp_stage_capture(void),_gz_hrp_validate_ready(void),_gz_hrp_validate_current(void);
u32 _gz_hrp_validate(const HrpBank*,u32,const VrWorld*,const BbBank*);
u32 _gz_hrp_archive_validate(u32,u32,u32);
void _gz_hrp_commit(void),_gz_hrp_forget_legacy(void);
#endif
