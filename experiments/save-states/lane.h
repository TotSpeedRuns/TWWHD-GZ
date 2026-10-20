#ifndef GZ_SHARED_OWNER_LANE_H
#define GZ_SHARED_OWNER_LANE_H
#include "owner.h"
enum { RLL_MAGIC=0x524c4c31u,RLL_CAPACITY=512,RLL_KEY_BYTES=512 };
enum { RLL_CREATE=1,RLL_DELETE=2,RLL_FUTURE_OWNER_SERVICE=9 };
enum { RLL_EITHER=0,RLL_PRESENT=1,RLL_ABSENT=2 };
enum { RLL_OK=0,RLL_BUSY=5,RLL_INVALID=28,RLL_CONTEXT=60,RLL_FOREIGN=61,RLL_CHANGED=62,RLL_LIMIT=63 };
/* Generic process/actor identity words. word10/A8/F4 are read-only raw identity
 * observations, not claims that every profile uses the same resource layout. */
typedef struct {u32 node,actor,pid,profile,parameters,parent,init,word10,wordA8,wordF4,home[5];} RllRow;
typedef struct {
 u32 magic,bytes,version,valid,token,kind,profile,source_pid,source_actor;
 u32 choice,current_layer,captured_frame,head,count,key_bytes;
 RohSample sample;
 u8 key[RLL_KEY_BYTES];
 RllRow rows[RLL_CAPACITY];
} RllBaseline;
/* Comparison envelope from the separately owned backend's fresh receipt.
 * It is NOT native authority. Owner must first validate that backend's full
 * immutable key and fresh request/append/actor/tag ownership receipt. */
typedef struct {
 u32 token,kind,profile,source_pid,source_actor,live_pid,live_actor,actor_queue_node;
 u32 key,key_bytes,observation_frame,expected_presence;
} RllTarget;
typedef struct {u32 result,frame,target_seen,unowned_count,first_bad_node,head,count,reserved;} RllReport;
extern RllBaseline _gz_rll_baseline,_gz_rll_stage;
extern volatile RllReport _gz_rll_report;
extern u32 _gz_rll_fault;
/* PREPARE under the real current owner's CLEAN OBSERVE1, all create/delete/
 * priority queues empty. Captures an exact ordered identity baseline and an
 * opaque immutable backend key. No native issue/pump/actor/source stores. */
u32 _gz_rll_prepare(u32 token,u32 kind,u32 profile,u32 source_pid,u32 source_actor,u32 key,u32 key_bytes);
/* Scaffold for a NEW owner service9. Current owner508e rejects9, and this
 * component does not alter that. Checks only ordered baseline identity deltas;
 * backend request/append/typed closure and real common nonqueue-owner validation
 * remain separate mandatory gates. Any error is sticky until actual IDLE. */
u32 _gz_rll_check_actor_delta(const RllTarget*);
/* Read-only Manager/pre-frame context check between typed service calls. It
 * requires MUTATING/inflight0; native clock may precede opening the next owner
 * frame. It never pumps or updates the real owner frame. */
u32 _gz_rll_poll_actor_delta(const RllTarget*);
/* Fresh recovery comparison of the already sealed CREATE baseline plus only
 * its issued target. Requires the private FAILED_DIRTY recovery ticket; never
 * renews the old deadline/frame or rewrites the first sticky ordinary fault.
 * A successful fresh comparison does not clear that fault or grant SDK calls. */
u32 _gz_rll_recovery_actor_delta(const RllTarget*);
/* Before native issue there is no live PID. Compare the original ordered
 * baseline without inventing one or treating BAD as process identity. */
u32 _gz_rll_check_unissued(u32 token,u32 key,u32 key_bytes);
u32 _gz_rll_forget(void); /* real IDLE only; never releases the owner */
/* Fixed private metadata stores only, after the original rollback owner has
 * proved complete native closure and before its final IDLE publication. */
void _gz_rll_recovery_finish_leaf(void);
u32 _gz_rll_native_admit(void); /* deliberately literal28 */
#endif
