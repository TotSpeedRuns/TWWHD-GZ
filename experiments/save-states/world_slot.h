#ifndef GZ_WORLD_SLOT_H
#define GZ_WORLD_SLOT_H
#include "correspondence.h"
/* Immutable identity evidence. Actor addresses are audit keys, never replay
 * destinations. Every replay participant must resolve and validate ownership. */
extern VrWorld _gz_vws_source, _gz_vws_stage;
extern volatile u32 _gz_vws_ready;
u32 _gz_vws_validate(const VrWorld*,u32);
u32 _gz_vws_source_matches(const VrWorld*,const u32*,const u32*,const u32*);
u32 _gz_vws_stage_capture(void);
u32 _gz_vws_validate_ready(void);
u32 _gz_vws_validate_current(void);
void _gz_vws_commit(void);
void _gz_vws_discard(void);
#endif
