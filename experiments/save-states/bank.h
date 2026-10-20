#ifndef GZ_B294_BANK_H
#define GZ_B294_BANK_H
#include "typed_core.h"
enum { B294_HEADER=64, B294_BYTES=64+PB_BANK_BYTES, B294_VERSION=1 };
/* Serialized source addresses are observations, never replayed as pointers. */
extern u8 _gz_b294_source[B294_BYTES],_gz_b294_stage[B294_BYTES];
extern volatile u32 _gz_b294_ready;
u32 _gz_b294_archive_bytes(void);
u32 _gz_b294_validate(u32 source,u32 bytes);
u32 _gz_b294_stage_capture(u32 queue,const PbContext* coherent_context);
u32 _gz_b294_validate_ready(const PbContext* unchanged_context);
void _gz_b294_discard(void);
/* Internal final tail only, after validate_ready under the same world hold. */
void _gz_b294_commit(void);
/* Convenience synchronous capture for offline/diagnostics, not world restore. */
u32 _gz_b294_capture(u32 queue,const PbContext* coherent_context);
/* Pure query; nonempty banks are explicitly NOT admitted for restore yet. */
u32 _gz_b294_restore_admission(u32 source,u32 bytes);
#endif
