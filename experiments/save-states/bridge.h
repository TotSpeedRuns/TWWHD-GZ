#ifndef GZ_GAME_LAYER_OWNER9_BRIDGE_H
#define GZ_GAME_LAYER_OWNER9_BRIDGE_H
#include "integration.h"
#include "layer_owner.h"
enum { GLB_MAGIC=0x474c4231u,GLB_ARMING=1,GLB_READY=2 };
typedef struct {
 u32 magic,bytes,version,phase,token,frame,choice,ambient,request,reserved;
 RohSample sample;NbKey key;
} GlbReceipt;
typedef char GlbReceiptSize[sizeof(GlbReceipt)==408?1:-1];
extern GlbReceipt _gz_glb_receipt;
/* Explicit preparation only: actual CLEAN/OBSERVE, same frame immediately
 * BEFORE RI prepare. This copies metadata and invokes leaf PREPARE. Never
 * call it from a typed/layer/owner validation callback. */
u32 _gz_glb_prepare(const NbKey*);
/* Explicit cleanup only: after actual owner IDLE and RI forget. */
u32 _gz_glb_forget(u32 token);
/* Read-only queries compiled beside unchanged owner9/RI source copies.
 * Neither invokes lifetime callbacks, owner validate/service APIs or leaf APIs.
 * These authenticate context only and never grant typed lifetime closure. */
u32 _gz_roh_gl_context(u32 token,u32 frame,u32 action);
u32 _gz_ri_gl_context(u32 token,u32 frame,u32 action);
#endif
