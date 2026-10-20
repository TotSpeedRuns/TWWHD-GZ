#ifndef GZ_ACTOR_BIRTH_RECIPE_CATALOG_H
#define GZ_ACTOR_BIRTH_RECIPE_CATALOG_H
typedef unsigned int u32;typedef unsigned char u8;
enum { BC_VERSION=4,BC_CAP=2048,BC_DEPTH=8,BC_ROW_MAGIC=0x42435231u,BC_MAGIC=0x42434331u };
/* Original standardRequest inputs at the genuine actor-factory callsite.
 * A successful request PID is NOT constructor/readiness authority.
 */
typedef struct {
 u32 magic,bytes,version,valid,sequence,origin,pid,profile,profile_entry;
 u32 layer,callback,callback_arg,append_audit,frame_before,frame_after,checksum;
 u8 append[36];u32 reserved[3];
} BcRow;
typedef struct {
 u32 magic,bytes,version,row_bytes,capacity,count,valid_count,depth;
 u32 attempts,completed,failed_requests,invalid_inputs,overflow,depth_overflow,blocked,reserved;
 BcRow rows[BC_CAP];
} BcCatalog;
typedef char BcRowSize[(sizeof(BcRow)==112)?1:-1];
extern BcCatalog _gz_bc_catalog;
extern BcRow _gz_bc_result;
/* Coherent private traversal, not a lock or a native lifetime grant. The
 * consumer is an original nonyielding capture callback. No caller destination
 * or actor/APP pointer is accepted. Any request/frame change expires it. */
typedef struct {u32 magic,bytes,valid,attempts,completed,count,header_seal,frame;} BcView;
extern BcView _gz_bc_view_state;
u32 _gz_bc_view_open(void),_gz_bc_view_valid(void);
const BcRow* _gz_bc_view_row(u32 index);
void _gz_bc_view_close(void);

/* Observation only; wrapper must preserve original pre/post API CPU context. */
void _gz_bc_before(u32 layer,u32 profile,u32 callback,u32 callback_arg,u32 append);
void _gz_bc_after(u32 native_result);
void _gz_bc_before_extended(u32,u32,u32,u32,u32,u32);
void _gz_bc_after_extended(u32,u32);
void _gz_bc_before_null(u32 layer,u32 profile,u32 callback,u32 data,u32 append,u32 origin);
void _gz_bc_before_stage(u32 layer,u32 profile,u32 callback,u32 callback_arg,u32 append);
void _gz_bc_before_fast(u32 layer,u32 profile,u32 callback,u32 callback_arg,u32 append,u32 actor_caller);
void _gz_bc_after_fast(u32 process,u32 actor_caller);
/* Returns0 only for one exact immutable row, 1invalid/unmatched,2duplicates.
 * The only copy-out is private _gz_bc_result, invalidated at entry and
 * published valid-last. Never accepts a caller destination/game pointer.
 * Consumers require return0 and validate that exact fixed result row.
 * No actor, request or append pointer is dereferenced by lookup.
 */
u32 _gz_bc_lookup(u32 pid,u32 profile);
u32 _gz_bc_row_valid(const BcRow*);
extern volatile u32 _gz_bc_native_frame;
extern const u32 _gz_bc_native_profiles[];
#endif
