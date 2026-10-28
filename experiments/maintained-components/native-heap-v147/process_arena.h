#ifndef GZ_PROCESS_ARENA_BIRTH_H
#define GZ_PROCESS_ARENA_BIRTH_H
#include "gate.h"
#include "lifetime.h"
enum { GPA_MATCH=0, GPA_MISSING=1, GPA_BUSY=2, GPA_CHANGED=3, GPA_FAULT=4 };
enum { GPA_COLD=0, GPA_PENDING=1, GPA_ALLOCATED=2, GPA_CONSTRUCTING=3,
       GPA_BORN=4, GPA_LIVE=5, GPA_RETIRED=6 };
enum { GPA_DROP=1, GPA_PROTOCOL=2, GPA_REPEATED=4, GPA_RANGE=8,
       GPA_SDK_OWNER=16, GPA_NATIVE=32 };
/* These callbacks accept ONLY the fixed, currently owned HG wrapper stack
 * ticket. Their callers/arguments must be proved from the emitted seams.
 * They never skip a native operation and never manufacture an API result. */
void _gz_gpa_resolve(u32 native_lr);
void _gz_gpa_sdk_prepare(const HgTicket*,u32 serial,u32 bytes,u32 alignment,
                         u32 actual_ctr,u32 native_lr);
void _gz_gpa_sdk_return(const HgTicket*,u32 serial,u32 returned,u32 bytes,
                        u32 alignment,u32 actual_ctr,u32 native_lr);
void _gz_gpa_factory_begin(const HgTicket*,u32 serial,u32 pointer,u32 bytes,
                          u32 native_lr);
void _gz_gpa_factory_return(const HgTicket*,u32 serial,u32 returned);
void _gz_gpa_sdk_release(const HgTicket*,u32 serial,u32 a0,u32 a1);
void _gz_gpa_sdk_release_return(const HgTicket*,u32 serial);
/* Called synchronously by the actual HL producer while its lock is held.
 * Birth pointers are fixed private HL storage, never native heap bodies.
 * Ending receives scalars BEFORE any fallible HL lock/header/row check. */
void _gz_gpa_hl_born(const HlHeap*,const HlControl*);
void _gz_gpa_hl_ending(u32 pointer,u32 kind,u32 native_site);
/* Provenance-only match: actual HG seal and fresh healthy HL/current-root
 * metadata are checked by this function, in HL -> GPA lock order.
 * It is NOT job/resource/device exclusion, a retained slot, or READ_READY. */
u32 _gz_gpa_try_match(u32 hg_token,u32 root,u32 generation,u32 start,u32 bytes);
extern u32 _gz_gpa_report[32];
#endif
