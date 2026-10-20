#ifndef GZ_CURRENT_LAYER_READ_BINDING_H
#define GZ_CURRENT_LAYER_READ_BINDING_H
#include "layer_map.h"
#include "lifecycle.h"
typedef struct {
 u32 magic,bytes,version,valid,result,token,frame,source_world,source_birth,source_layers;
 u32 source_world_hash,source_birth_hash,source_layers_hash,current_world_hash,current_birth_hash,current_layers_hash,plan_hash;
 u32 source_epoch,current_epoch,required_unbound,required_count,bound_count,process_serial,catalog_attempts,catalog_completed,registry_revision;
 u32 runtime_validation_required,mutation_permission,context_kind,context_serial,reserved,checksum;
} LcView;
typedef char LcViewSize[sizeof(LcView)==128?1:-1];
extern BbBank _gz_lgc_birth;
extern LgBank _gz_lgc_layers;
extern LcView _gz_lgc_view;
u32 _gz_bcb_read_current(u32,u32),_gz_lgb_read_current(u32,u32),_gz_lgb_current_read_validate(u32,u32);
/* One serialized native callback, under a selected immutable READ or held CLEAN/OBSERVE service.
 * No saved actor/layer address is executed or used as a current registry root.
 * This is current structural context, not physical lifetime/mutation authority. */
u32 _gz_lgc_build(u32 token,u32 frame),_gz_lgc_read(u32 token,u32 frame);
#endif
