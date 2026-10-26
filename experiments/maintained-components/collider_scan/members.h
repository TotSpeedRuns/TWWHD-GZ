#ifndef GZ_PROCESS_COLLIDER_MEMBERS_H
#define GZ_PROCESS_COLLIDER_MEMBERS_H
#include "process.h"
#include "observer.h"
enum {CM_MAGIC=0x434d4231u,CM_CREATED=1,CM_ENDING=2};
enum {CM_BAD_CO_HEADER=1,CM_CO_BUSY=2,CM_BAD_SPAN=4,CM_BAD_ROW=8,
 CM_COUNTER_EXHAUSTED=16,CM_REENTRY=32,CM_SOURCE_GAP=64,CM_FRAME_CHANGED=128};
enum {CM_ROW_CREATED=8,CM_ROW_ENDING=16};
enum {CM_OK=0,CM_UNAVAILABLE=1,CM_BAD_QUERY=2,CM_CHANGED=3,CM_NOT_LIVE=4};
enum {CM_MISSING_NATIVE_HOLD=1,CM_MISSING_ALLOCATION_GENERATION=2,
 CM_MISSING_REFERENCES=4,CM_MISSING_CAPTURE_BOUNDARY=8,CM_MISSING_SESSION=16,
 CM_MISSING_ALL=31};
typedef struct {
 u32 magic,bytes,version,busy,flags,attempts,commits,retired;
 u32 process,allocation_bytes,reason,frame,count,co_serial,session,reserved;
} CmControl;
typedef struct {
 u32 valid,result,missing,session,process,process_generation,profile,pid;
 u32 allocation_bytes,collider,collider_generation,member_offset;
 u32 status_audit,object_vtable_audit,co_flags,frame,po_serial,co_serial;
 u32 po_observation_flags,co_observation_flags,requested_bytes,cm_flags,reserved[2];
} CmMember;
typedef char CmControlSize[(sizeof(CmControl)==64)?1:-1];
typedef char CmMemberSize[(sizeof(CmMember)==96)?1:-1];
extern CmControl _gz_cm_control;
extern CmMember _gz_cm_member_observation;
extern u32 _gz_cm_query_busy;
/* Private observation-table writes only; NEVER dereference p, colliders,
 * statuses, actors, vtables, native heaps or allocation contents. This is
 * called at the observed fresh base-process return and exact pre-free call.
 * Retires observed live/unknown base-header overlaps and records the reason;
 * no native C++ destructor event is fabricated or counted. External heap
 * colliders remain explicit dependencies, not guessed members of the process.
 */
u32 _gz_cm_invalidate(u32 p,u32 bytes,u32 reason);
u32 _gz_cm_healthy(void);
/* Fixed private coherent observations, not replay permission. A member key
 * is the exact process-relative object offset, never registration order.
 * Status/vtable pointers are AUDIT ONLY. No payload pointer is followed.
 * Valid output still retains missing31 and unbound session0. Native held
 * boundary/allocation generation/complete reference/capture proofs remain
 * mandatory before these observations can authorize a restore.
 */
u32 _gz_cm_member(u32 object,u32 bytes);
#endif
