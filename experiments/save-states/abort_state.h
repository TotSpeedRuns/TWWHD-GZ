/* Offline prototype. No hook, archive registration, or live integration. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
enum { PBA_MAGIC=0x50424132, PBA_IDLE=0, PBA_OWNED=1, PBA_ABORTING=2,
 PBA_CANCELLING=3, PBA_RETIRING=4, PBA_DONE=5 };
/* Original operation key is immutable. Current scene/slot can change while the
 * aborted operation is drained; that never transfers ownership to a new slot. */
typedef struct {
 u32 token,slot,archive,stage0,stage1,room,layer,generation,pid,callback;
 u8 home[20];
 u32 family[5]; /* modelData, BCK+448, BCK+45C, BRK+4E8, BRK+560 */
} PbaKey;
typedef struct {u32 create_head,actor_head,delete_head;} PbaQueues;
typedef struct {
 u32 magic,bytes,phase,ready;
 PbaKey key;
 u32 request_audit,request_layer,append_audit;
 u32 actor_audit,profile_audit,methods_audit,model_audit;
 u32 cancel_calls,delete_calls,last_reason;
 /* Allocation ownership is independent of the request after native Cancel. */
 u32 allocation_heap,allocation_heap_vtable,allocation_free_method;
 u32 append_free_calls,cancel_succeeded,cleanup_admitted;
} PbaTracker;
/* 0 success/finished, 5 waiting, 28 invalid identity/API precondition,
 * 30 fully created actor is now counted/modified and needs post-apply rollback.
 * No actor or Link bytes are written by this component. */
u32 _gz_pba_arm(PbaTracker*,const PbaKey*,const PbaQueues*);
u32 _gz_pba_abort(PbaTracker*,u32 token);
u32 _gz_pba_constructor_ready(PbaTracker*,void* actor,const PbaQueues*);
u32 _gz_pba_tick(PbaTracker*,u32 token,const PbaQueues*);
u32 _gz_pba_commit(PbaTracker*,u32 token);
u32 _gz_pba_should_pause(const PbaTracker*,void* current_actor);
/* Exact owned EUR v0 framework entries, verified by local byte disassembly.
 * They are explicit semantic doubles in offline tests. */
u32 _gz_pba_native_cancel_request(void* current_request);
u32 _gz_pba_native_delete_actor(void* current_actor);
extern const u8 _gz_pba_native_standard_methods[];

/* New prototype is disabled by default. This does not enable any constructor. */
extern u32 _gz_pba_append_cleanup_enabled;
/* 31 cleanup diagnostic disabled, 32 allocation heap contract changed.
 * Native Cancel success must be exactly1. No cached request dereference follows it.
 * Caller must arm immediately after native creation, on the same engine thread,
 * with the cMl heap unchanged across the fopAcM_create call. */
void _gz_pba_native_free_append(void* owned_allocation);
extern volatile u32 _gz_pba_native_heap;
extern volatile u32 _gz_pba_native_create_queue;
extern volatile u32 _gz_pba_native_actor_queue;
extern volatile u32 _gz_pba_native_delete_queue;
