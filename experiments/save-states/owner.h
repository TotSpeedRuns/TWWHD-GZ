#ifndef GZ_PRIVATE_RESTORE_OPERATION_HOLD_H
#define GZ_PRIVATE_RESTORE_OPERATION_HOLD_H
typedef unsigned int u32;typedef unsigned short u16;typedef unsigned char u8;
enum { ROH_MAGIC=0x524f4831u,ROH_VERSION=2,ROH_MAX_FRAMES=120,ROH_MAX_SERVICES=8192 };
enum { ROH_IDLE=0,ROH_CLEAN=1,ROH_MUTATING=2,ROH_FAILED_CLEAN=3,ROH_FAILED_DIRTY=4 };
enum { ROH_ORDINARY=0,ROH_OWNED_SERVICE=1,ROH_RECOVERY_ONLY=2,ROH_DUPLICATE_FRAME=3 };
enum { ROH_OBSERVE=1,ROH_LIGHT=2,ROH_ENEMY=3,ROH_BRIDGE=4,ROH_CAMERA=5,ROH_FTRAVEL=6,ROH_FIELDTRAVEL=7,ROH_POT=8,ROH_LIFETIME=9,ROH_UI=10 };
enum { ROH_OK=0,ROH_BUSY=5,ROH_INVALID=28,ROH_OWNER=40,ROH_SCENE=41,ROH_ARCHIVE=42,
       ROH_REGISTRY=43,ROH_LINK=44,ROH_CAMERA_BAD=45,ROH_QUIET=46,ROH_TIMEOUT=47,
       ROH_REENTRY=48,ROH_SERVICE=49,ROH_PENDING=50,ROH_CORRUPT=51,ROH_EXHAUSTED=52 };
typedef struct {
 u32 slot,archive,archive_bytes,archive_version,components,directory_hash,archive_hash;
 u32 working_base,bank,registry,registry_revision,registry_header_hash,registry_hash;
 u32 link,pid,profile,morf,model,animation,resource444;
 u32 camera_index,camera,camera_self,camera_link,camera_bound_index;
 u32 stage0,stage1,room,layer,epoch,loading,event,frame;
 u32 actor_head,actor_count,actor_topology_hash,create_head,delete_head;
 u32 pot_source,pot_bytes,pot_work,pot_hash,light_header_hash,light_hash;
} RohSample;
typedef struct {
 u32 magic,bytes,version,phase,hold,token,mutated,error,limit,start_frame,last_frame;
 u32 frame_open,inflight,service_calls,committed_token,reserved;
 RohSample sample;
} RohOwner;
typedef char RohSampleSize[(sizeof(RohSample)==176)?1:-1];
typedef char RohOwnerSize[(sizeof(RohOwner)==240)?1:-1];
extern RohOwner _gz_roh_owner;
extern u32 _gz_roh_serial,_gz_roh_registry_revision;

/* Native caller bookkeeping only: no world-readiness or restore admission. */
u32 _gz_roh_begin(u32 slot,u32 max_frames);
u32 _gz_roh_work_replaced(void); /* idle only; MUST wire every install/rebind */
u32 _gz_roh_manager_gate(void); /* 0 normal Management; 1 suppress it */
u32 _gz_roh_pre_frame(void); /* phase-specific dispatch; never generic resume */
u32 _gz_roh_user_guard(void); /* 0 idle,5 ownership prevents conflicting work */
u32 _gz_roh_mark_mutating(u32 token); /* immediately BEFORE first world write */
u32 _gz_roh_service_begin(u32 token,u32 typed_id);
u32 _gz_roh_service_end(u32 token,u32 typed_id,u32 result);
u32 _gz_roh_replay_ready(void); /* strict boolean; only current owned service */
u32 _gz_roh_fail(u32 token,u32 reason);
u32 _gz_roh_abort_clean(u32 token);
/* Pure owner/acks check; does not mutate receipt, release hold or call APIs.
 * Root reads the pinned receipt after0 in this SAME non-yielding callback.
 * Completion publication must be private integer stores combined with PB,
 * Link count/tracker/RNG by root; no fallible post-copy release API exists. */
u32 _gz_roh_final_check(u32 token);
/* Pot access0=read-only prepare in OBSERVE;1=write in POT service;
 *2=same-frame live recovery (never reads the archive);3=final participant check.
 * This is operation/source authority, NOT full-game restore admission. */
u32 _gz_roh_pot_authority(u32 token,u32 source,u32 bytes,u32 frame,u32 access);
/* Deliberately literal28: successful runtime admission remains disabled. */
u32 _gz_roh_runtime_admit(u32 token);
/* Root bridge: scope0 uses the unchanged original path; scope1 requires a
 * live authenticated lifetime descriptor while MUTATING. Other values corrupt.
 * validate must independently certify fresh backend+sealed lane+queue+typed
 * closure between two complete common guard passes. Neither callback calls
 * owner APIs; scope is read-only authentication without any native calls. */
u32 _gz_roh_lifetime_scope(u32 token);
u32 _gz_roh_lifetime_validate(u32 token);
/* Read-only original common guards immediately before an SDK action.
 * Requires the real current held MUTATING/inflight9 token, pinned UI choice
 * and original deadline. deep0=original shallow checks; deep1=original deep
 * source/bank hashes and byte comparisons. Rejects deep>1.
 * Never calls lifetime callbacks, writes owner/game/source data, grants queue
 * ownership/closure, marks mutation or releases hold. Root MUST separately
 * validate fresh backend/key/receipt/lane/closure; real service_begin9 must
 * already have succeeded. Safe to call from a validation callback without
 * lifecycle recursion; success is common-context consistency only. */
u32 _gz_roh_lifetime_common(u32 token,u32 deep);
/* Completed-lifetime handoff only: original common checks under the exact
 * same-frame typed service2..8, with a sealed private handoff descriptor.
 * No native SDK/lifetime admission or world release follows from success. */
u32 _gz_roh_replay_common(u32 token,u32 frame,u32 deep);
/* Private FAILED_DIRTY recovery context. begin requires a strict authenticated
 * BOH scope1, retains the failed owner unchanged and opens one independently
 * bounded ticket per token. common/baseline are read-only and never call BOH,
 * source validators, native SDK functions or ordinary lifetime callbacks.
 * Success0 is context consistency, never issue/delete/release authority.
 * baseline additionally proves the original empty create/delete queues and
 * ordered actor topology. Original errors/inflight/hold remain untouched. */
u32 _gz_roh_recovery_begin(u32 token,u32 max_frames);
u32 _gz_roh_recovery_common(u32 token,u32 frame);
u32 _gz_roh_recovery_baseline(u32 token,u32 frame);
/* Finish a fully verified rollback, never a successful snapshot restore.
 * Requires the independent recovery ticket, whole native before-image closure
 * and original source-pin metadata. Retains the failed receipt before the
 * nonyielding private cleanup/pin-release tail publishes IDLE. */
u32 _gz_roh_recovery_finish(u32 token,u32 frame);
#endif
