#ifndef GZ_CLOCK_STATE_H
#define GZ_CLOCK_STATE_H
#include "world_slot.h"
enum {CKB_MAGIC=0x434b4231u,CKB_BYTES=128};
typedef struct {
 u32 magic,bytes,version,valid,frame,epoch,session,stage0,stage1,room,layer,world_hash,world_sequence;
 u32 values[5]; /* canonical time/date, live time/date, time advance */
 u32 checksum,reserved[13]; /* v2: reserved[0..2] = air current/max/status; v1 all zero */
} CkbBank;
typedef char CkbSize[sizeof(CkbBank)==CKB_BYTES?1:-1];
extern CkbBank _gz_ckb_stage,_gz_ckb_source;
u32 _gz_ckb_stage_capture(void),_gz_ckb_validate_ready(void),_gz_ckb_validate_current(void);
u32 _gz_ckb_validate(const CkbBank*,u32,const VrWorld*);
u32 _gz_ckb_capture_scope(u32),_gz_ckb_archive_validate(u32,u32,u32),_gz_ckb_prepare_install(u32);
void _gz_ckb_commit(void),_gz_ckb_discard(void);
#endif
