#ifndef GZ_BIRTH_BANK_H
#define GZ_BIRTH_BANK_H
#include "catalog.h"
#include "../native-scene-inventory-20261017/process.h"
#include "../void-rebuild-correspondence-20261015/correspondence.h"
enum {BB_MAGIC=0x42434231u,BB_HEADER=96,BB_ROW=192,BB_CAP=VR_CAPACITY,BB_BYTES=BB_HEADER+BB_CAP*BB_ROW};
typedef struct {u32 index,valid,requested_profile,generation;PoEntry process;BcRow request;} BbEntry;
typedef struct {
 u32 magic,bytes,version,valid,count,frame,epoch,session,stage0,stage1,room,layer;
 u32 world_hash,world_sequence,checksum,reserved,catalog_version,catalog_count;
 u32 catalog_attempts,catalog_completed,catalog_seal,process_serial,process_session,reserved2;
 BbEntry rows[BB_CAP];
} BbBank;
typedef char BbEntrySize[(sizeof(BbEntry)==BB_ROW)?1:-1];
typedef char BbBankSize[(sizeof(BbBank)==BB_BYTES)?1:-1];
extern BbBank _gz_bcb_work,_gz_bcb_staging;
extern volatile u32 _gz_bcb_ready;
u32 _gz_bcb_stage_capture(void),_gz_bcb_validate_ready(void),_gz_bcb_validate_current(void);
void _gz_bcb_commit(void),_gz_bcb_discard(void);
u32 _gz_bcb_validate(const BbBank*,u32,const VrWorld*);
u32 _gz_bcb_archive_validate(u32,u32,u32);
#endif
