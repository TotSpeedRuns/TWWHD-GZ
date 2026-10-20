#ifndef GZ_SAND_STATE_H
#define GZ_SAND_STATE_H
#include "world_slot.h"
enum { SD_MAGIC=0x534e4431,SD_N=0x1081,SD_NRM=0x1040,
 SD_WORDS=SD_N*10,SD_BYTES=96+SD_WORDS*4 };
typedef struct { u32 h[24],data[SD_WORDS]; } SdRecord;
typedef char SdSize[sizeof(SdRecord)==169096?1:-1];
extern SdRecord _gz_sand_saved,_gz_sand_work;
extern volatile u32 _gz_sand_diag[8];
u32 _gz_sand_record_validate(const SdRecord*,u32,const VrWorld*);
u32 _gz_sand_stage(void);
u32 _gz_sand_validate_ready(void);
void _gz_sand_commit(void);
void _gz_sand_discard(void);
u32 _gz_sand_preflight(void);
u32 _gz_sand_saved_valid(void);
void _gz_sand_apply_core(void);
#endif
