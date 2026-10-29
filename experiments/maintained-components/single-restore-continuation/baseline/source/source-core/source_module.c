#include "source_pin_digest.h"
#include "source_pin_format.h"
/* Logical compressed backings with original typed source/owner authority.
 * Native fixed-admission composition is separate from game restore coverage. */
extern unsigned int _gz_ncb_archive_validate(unsigned int,unsigned int,unsigned int);
extern unsigned int _gz_ckb_archive_validate(unsigned int,unsigned int,unsigned int);
extern unsigned int _gz_csb_archive_validate(unsigned int,unsigned int,unsigned int);
extern unsigned int _gz_cga_archive_validate(unsigned int,unsigned int,unsigned int);
extern unsigned int _gz_hrp_archive_validate(unsigned int,unsigned int,unsigned int);
#include "source_worker_state.inc"
#include "../owner/owner.c"
#include "compressed_slot_source.c"
#include "source_bridge.c"
#include "source_worker_custodian.inc"
#include "source_direct_recovery.c"
#include "source_read.c"
#include "source_access.c"
#include "held_source.c"
#include "held_item_recipe.c"
#include "source_native_adapter.c"

#include "source_pin_digest.inc"
#include "source_pin_format.inc"
#include "source_bomb_final.inc"
