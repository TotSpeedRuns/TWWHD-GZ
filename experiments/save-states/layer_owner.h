#ifndef GZ_GAME_PROCESS_LAYER_OWNER_H
#define GZ_GAME_PROCESS_LAYER_OWNER_H
typedef unsigned int u32;typedef unsigned short u16;typedef unsigned char u8;
enum { GL_MAGIC=0x474c5231u,GL_DEPTH=8,GL_MAX_LAYERS=128,GL_MAX_NODES=4096 };
enum { GL_PREPARE=1,GL_CHECK=2,GL_FORGET=3 };
enum { GL_OK=0,GL_ARGUMENT=28,GL_DENIED=60,GL_BUSY=61,GL_CHANGED=65,GL_REGISTRY=66,GL_OWNER=67,GL_DEPTH_ERROR=68,GL_ABSENT=69 };
typedef struct {
 u32 layer,id,owner,parent,lists,list_count;
 u32 pid,profile,profile_address,node_methods,base_methods,parent_list;
} GlLink;
typedef struct {
 u32 magic,bytes,version,valid,token,prepared_frame,next_id;
 u32 registry_head,registry_tail,registry_count,node_type,base_type,depth[2];
 GlLink links[2][GL_DEPTH];
} GlReceipt;
typedef char GlLinkSize[sizeof(GlLink)==48?1:-1];
typedef char GlReceiptSize[sizeof(GlReceipt)==824?1:-1];
extern GlReceipt _gz_gl_receipt;
/* PREPARE is real held CLEAN/OBSERVE, empty lifecycle work and stable scene.
 * CHECK is the same held operation/current real frame: CLEAN/OBSERVE while RI
 * captures initial bytes, then MUTATING/service9 or between-service validation.
 * FORGET is real IDLE only. The bridge must not call owner validate or this leaf.
 * Defaults deny. These are read-only callbacks, including all private inputs. */
u32 _gz_gl_authority(u32 token,u32 real_frame,u32 action);
/* Captures at most ambient and request layers; no certificate-time rebinding.
 *0 is success; failure leaves the prior receipt unchanged. */
u32 _gz_gl_prepare(u32 token,u32 real_frame,u32 ambient,u32 request);
/* Strict boolean. No native call, game/source store or receipt rebinding.
 *A callback's illegal receipt edits are restored before rejection. Membership is
 *proved from current native lists before a saved layer body or owner is read.
 *The bounded depth/layer/process limits are explicit supported limits. */
u32 _gz_gl_layer_live(u32 token,u32 real_frame,u32 layer);
u32 _gz_gl_forget(u32 token);
/* Exact original EUR aliases; native evidence is retained beside this source. */
extern const volatile u32 _gz_gl_layer_list[3],_gz_gl_next_id,_gz_gl_initialized;
extern const volatile u32 _gz_gl_node_type,_gz_gl_base_type,_gz_gl_profile_table[];
extern const u8 _gz_gl_root[],_gz_gl_root_lists[];
#endif
