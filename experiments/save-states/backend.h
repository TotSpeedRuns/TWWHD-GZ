#ifndef GZ_SHARED_NATIVE_LIFETIME_BACKEND_H
#define GZ_SHARED_NATIVE_LIFETIME_BACKEND_H
typedef unsigned int u32;typedef unsigned short u16;typedef unsigned char u8;
enum { NB_KEY=0x4e424b31u,NB_MAGIC=0x4e424a31u,NB_RECEIPT=0x4e425231u };
enum { NB_CREATE=1,NB_DELETE=2,NB_IDLE=0,NB_PENDING=1,NB_READY=2,NB_RETIRING=3,NB_DONE=4,NB_FAILED=5 };
enum { NB_ARM=1,NB_QUERY=2,NB_STEP_PRE=3,NB_STEP_POST=4,NB_CTOR=5,NB_DELETE_ISSUE=6,NB_CANCEL=7,NB_FREE=8,NB_FINISH=9,NB_COMMIT=10,NB_RECOVERY=11,NB_FAMILY_BEFORE=12 };
enum { NB_TYPED_PREPARE=1,NB_TYPED_READY=2,NB_TYPED_RETIRE=3,NB_TYPED_LAYER=4 };
enum { NB_APPEND_REQUEST=1,NB_APPEND_ACTOR=2,NB_APPEND_NATIVE_FREED=3,NB_APPEND_CANCEL_FREED=4,NB_APPEND_UNKNOWN=5 };
enum { NB_OK=0,NB_WAIT=5,NB_ARGUMENT=28,NB_DENIED=60,NB_BUSY=61,NB_CONTEXT=62,NB_IDENTITY=64,NB_CHANGED=65,NB_QUEUE=66,NB_LAYER=67,NB_PROFILE=68,NB_CLOSURE=69 };
typedef struct {u32 frame,epoch,stage0,stage1,room,layer,token;} NbClock;
typedef struct {
 u32 magic,bytes,version,token,kind,profile,source_pid,source_actor,evidence_hash,recipe_hash;
 u32 epoch,stage0,stage1,room,layer,profile_address,actor_type,actor_bytes,callback,callback_data,request_layer,live_parent;
 u32 profile_raw[12],methods[5];u8 birth[36];
} NbKey;
typedef struct {
 u32 magic,bytes,version,valid,result,token,kind,profile,source_pid,source_actor,live_pid,live_actor,actor_queue_node;
 u32 request,append,request_layer,callback,callback_data,key_hash,step_frame,step_serial,cancel_calls,append_disposition,delete_tag,observation_frame;
} NbReceipt;
typedef struct {
 u32 magic,bytes,version,phase,error,key_hash,pid,request,append,actor,actor_node,delete_tag;
 u32 heap,heap_table,free_method,started,last_frame,last_step,steps,constructor_done,constructor_actor,constructor_frame;
 u32 create_steps,delete_steps,cancel_calls,free_calls,delete_calls,disposition,cancel_succeeded,completed,delete_done,reserved;
 NbKey key;NbReceipt receipt;
} NbJournal;
typedef char NbKeySize[sizeof(NbKey)==192?1:-1];
typedef char NbReceiptSize[sizeof(NbReceipt)==100?1:-1];
typedef char NbJournalSize[sizeof(NbJournal)==420?1:-1];
/* Separate evidence: never extends NbJournal420 or NbReceipt100. */
enum { NB_FAMILY_MAGIC=0x4e424631u,NB_FAMILY_PROFILE=453,
 NB_FAMILY_METHOD=0x024d09c0u,NB_FAMILY_BODY=0x024cbc9cu,
 NB_FAMILY_MISSING_FRAMEWORK=1,NB_FAMILY_MISSING_HEAP=2,
 NB_FAMILY_MISSING_REVERSE_REFS=4,NB_FAMILY_MISSING_ALL=7 };
typedef struct {
 u32 magic,bytes,version,valid,cookie,token,profile,pid,actor,delete_tag;
 u32 method,body,result,family_return_one,frame,step_serial,key_hash;
 u32 journal_hash,clock_hash,missing_coverage;
} NbFamilyReturn;
typedef char NbFamilyReturnSize[sizeof(NbFamilyReturn)==80?1:-1];
extern NbJournal _gz_nb_journal;
extern NbFamilyReturn _gz_nb_family_return;
/* Before returns a nonzero monotonic opaque cookie, or zero on denial.
 * Nested/failed before leaves an existing pending call untouched.
 * After follows no actor/profile/heap pointers and calls no owner validator. */
u32 _gz_nb_delete_before(u32 actor,u32 method,const NbClock*);
u32 _gz_nb_delete_after(u32 actor,u32 result,u32 method,u32 cookie);
u32 _gz_nb_family_return_valid(void);
/* ARM must run in the same owned engine frame immediately after an audited
 * standard-request issue; it does not invent/allocate a birth recipe. */
u32 _gz_nb_arm_create(const NbKey*,u32 issued_pid,const NbClock*);
u32 _gz_nb_arm_delete(const NbKey*,u32 current_actor,const NbClock*);
u32 _gz_nb_query(const NbClock*);
u32 _gz_nb_step(const NbClock*);
u32 _gz_nb_retire(const NbClock*);
u32 _gz_nb_commit(const NbClock*);
/* Narrow native Create-hook receipt. Allowed only inside our owned pump,
 * exact current request.mpRes/method, and separate native-hook authority. */
u32 _gz_nb_constructor(u32 actor,u32 result,u32 typed_method,const NbClock*);
/* Owner/lane and typed dependency closure are default-denied integration APIs.
 * All inputs read-only. Receipt is comparison evidence, never authority alone. */
u32 _gz_nb_authority(const NbKey*,const NbClock*,const NbReceipt*,u32 action);
u32 _gz_nb_typed(const NbKey*,const NbClock*,u32 actor,u32 access);
u32 _gz_nb_layer_live(const NbKey*,u32 layer);
u32 _gz_nb_native_create_do(u32 request);
u32 _gz_nb_native_delete_tag_do(u32 tag,u32 method);
u32 _gz_nb_native_cancel(u32 request);
u32 _gz_nb_native_delete(u32 actor);
void _gz_nb_native_free(u32 append);
void _gz_nb_native_set_layer(u32 layer);
extern volatile u32 _gz_nb_live_frame; /* exact native frame scalar101FF558 */
extern volatile u32 _gz_nb_actor_head,_gz_nb_create_head,_gz_nb_delete_head,_gz_nb_priority_head,_gz_nb_heap,_gz_nb_current_layer,_gz_nb_actor_type,_gz_nb_leaf_type;
extern const u32 _gz_nb_profile_table[],_gz_nb_standard_methods[],_gz_nb_standard_phases[];
#endif
