#ifndef GZ_SHARED_OWNER_LIFETIME_INTEGRATION_H
#define GZ_SHARED_OWNER_LIFETIME_INTEGRATION_H
#include "owner.h"
#include "backend.h"
#include "lane.h"
#include "catalog.h"
enum { RI_MAGIC=0x52494a31u,RI_PLAN=0x52495031u,RI_NONE=0,RI_PREPARED=1,RI_ALLOCATING=2,RI_ISSUING=3,RI_ARMING=4,RI_ACTIVE=5,RI_FINISHED=6,RI_FAILED=7 };
enum { RI_TYPE_PREPARE=1,RI_TYPE_ISSUE=2,RI_TYPE_POLL=3,RI_TYPE_READY=4,RI_TYPE_RETIRE=5,RI_TYPE_LAYER=6 };
typedef struct {
 u32 magic,bytes,version,valid,token,choice,frame;
 RohSample sample;NbKey key;BcRow birth;
} RiPlan;
typedef struct {
 u32 magic,bytes,version,phase,error,token,append,request,pid,issue_frame;
 u32 heap,heap_table,heap_alloc,heap_free,layer,ambient_layer,layer_count,cancel_head,cancel_count;
 u32 native_issued,backend_armed,append_freed,completed;
} RiJournal;
extern RiPlan _gz_ri_plan,_gz_ri_stage;
extern RiJournal _gz_ri_journal;
/* Actual original catalog lookup is mandatory. PREPARE remains read-only
 * under the real CLEAN/OBSERVE owner and seals immutable source/key inputs. */
u32 _gz_ri_prepare(const NbKey*);
/* The caller already owns current service9. Issue is journaled before every
 * native allocation/request. Failed ARM never discards the original APP/PID. */
u32 _gz_ri_issue(void);
u32 _gz_ri_step(void);
u32 _gz_ri_finish(u32 commit_create);
u32 _gz_ri_forget(void); /* real IDLE only, never releases owner */
u32 _gz_ri_constructor(u32 actor,u32 result,u32 method);
/* Before: r3 actor/r4 method -> cookie r3. After: r3 actor/r4 native result/
 * r5 method/r6 cookie -> observer status. Native wrapper restores native
 * result and complete context regardless of observer return. */
u32 _gz_ri_delete_before(u32 actor,u32 method);
u32 _gz_ri_delete_after(u32 actor,u32 result,u32 method,u32 cookie);
/* NEW owner callbacks; no calls back into owner validate/service APIs. */
u32 _gz_roh_lifetime_scope(u32 token);
u32 _gz_roh_lifetime_validate(u32 token);
/* Typed source/birth/resource/child/layer closure certificate. Default deny.
 * A catalog hash or successful PID is never sufficient authority. */
u32 _gz_ri_typed(const RiPlan*,const RiJournal*,const NbClock*,u32 actor,u32 action);
u32 _gz_ri_layer_live(const RiPlan*,u32 layer);
u32 _gz_ri_native_allocate_append(void);
u32 _gz_ri_native_standard_request(u32 layer,u32 profile,u32 callback,u32 data,u32 append);
void _gz_ri_native_free_append(u32 append);
extern volatile u32 _gz_ri_heap,_gz_ri_current_layer;
extern volatile u32 _gz_ri_next_pid;
extern const u8 _gz_ri_private_begin[],_gz_ri_private_end[];
/* Exact allocated static DATA/BSS envelope from the pinned native RPX. An
 * append returned by the heap must never point into native globals/tables. */
extern const u8 _gz_ri_native_data_begin[],_gz_ri_native_data_end[];
#endif
