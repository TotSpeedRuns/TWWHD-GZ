/* Unwired capture-BL component. It does not replace the public capture body.
 * The original five arguments are forwarded to the unchanged current native
 * capture chain only after normalization and lifetime checks succeed.
 * GPU/framework serialization and the fixed registry are separate providers;
 * an unresolved provider is an integration obligation, never a boolean option.
 */
#include "capture_normalize.h"
#include "../render-current-graph-preflight-r2/lifetime_witness.h"
#include "../render-normalized-slots-r1/normalized_slot_registry.h"
#include "../render-normalized-slots-r1/render_trace.h"
#include <stddef.h>
enum {RCW_CONTEXT=2881,RCW_SPACE=2883,RCW_SAVE=1,RCW_RESTORE=2,
 RCW_CLAIMED=1,RCW_GUARDED=2,RCW_RESET_RETURNED=3,RCW_EXPORTED=4};
extern RcwU _gz_ghs_scope(RcwU,RcwU,RcwU),_gz_ghs_hg(RcwU,RcwU);
extern RcwU _gz_ois_heap_role(RcwU,RcwU),_gz_ois_heap_stage_match(RcwU,RcwU,RcwU,RcwU);
extern RcwU _gz_image_write_allowed(RcwU,RcwU);
/* Must be a fixed genuine graphics/framework ownership provider, integrated
 * with the qualified before-HG GPU retirement coordinator. Not supplied by
 * this source and not derived from empty queues or a caller-selected token. */
extern RcwU _gz_rcw_graphics_held(RcwU,RcwU);
extern void _gz_rcw_native_owner_reset(void);
extern RcwU _gz_rcw_native_capture(RcwU,RcwU,RcwU,RcwU,RcwU*);

static RcwU apart(RcwU p,RcwU n,RcwU q,RcwU m){return p>=q?p-q>=m:q-p>=n;}
static RcwU owner(RcwU req,RcwU frame,RcwU p,RcwU cap,RcwU hg,RcwU role)
{
 if(!req||!frame||!hg||(role!=RCW_SAVE&&role!=RCW_RESTORE)){
  _gz_rns_trace(req,frame,RT_OWNER_FAIL,RCW_CONTEXT,1);return 0;}
 if(!_gz_ghs_scope(req,frame,2)){
  _gz_rns_trace(req,frame,RT_OWNER_FAIL,RCW_CONTEXT,2);return 0;}
 if(_gz_ghs_hg(req,frame)!=hg){
  _gz_rns_trace(req,frame,RT_OWNER_FAIL,RCW_CONTEXT,3);return 0;}
 if(_gz_ois_heap_role(req,frame)!=role){
  _gz_rns_trace(req,frame,RT_OWNER_FAIL,RCW_CONTEXT,4);return 0;}
 if(!_gz_ois_heap_stage_match(req,frame,p,cap)){
  _gz_rns_trace(req,frame,RT_OWNER_FAIL,RCW_CONTEXT,5);return 0;}
 if(!_gz_rcw_graphics_held(req,frame)){
  _gz_rns_trace(req,frame,RT_OWNER_FAIL,RCW_CONTEXT,6);return 0;}
 return 1;
}
/* This fixed entry is called by the exporter before/after its metadata lock.
 * Only this coordinator writes RESET_RETURNED, after the actual native reset
 * has returned. The witness exporter cannot manufacture this receipt. */
RcwU _gz_rcl_normalized_scope(RcwU req,RcwU frame)
{
 if(!(_gz_rns_order.active==1 && _gz_rns_order.stage==RCW_RESET_RETURNED &&
  _gz_rns_order.request==req && _gz_rns_order.frame==frame)){
  _gz_rns_trace(req,frame,RT_ORDER_FAIL,RCW_CONTEXT,0);return 0;}
 return owner(req,frame,_gz_rns_order.staging,_gz_rns_order.capacity,
   _gz_rns_order.hg,_gz_rns_order.role);
}
RcwU _gz_rcw_capture(RcwU req,RcwU frame,RcwU p,RcwU cap,RcwU *written)
{
 RcwU hg,role,z=RCW_CONTEXT,done;
 /* Decline reentry without clearing the outer invocation's real receipt. */
 if(_gz_rns_order.active||_gz_rns_order.stage==RCW_EXPORTED)return RCW_CONTEXT;
 hg=_gz_ghs_hg(req,frame);role=_gz_ois_heap_role(req,frame);
 if(!owner(req,frame,p,cap,hg,role))return RCW_CONTEXT;
 if(cap<RCG_SCRATCH_BYTES)return RCW_SPACE;
 if(!written || !_gz_image_write_allowed((RcwU)(uintptr_t)written,4) ||
  !apart((RcwU)(uintptr_t)written,4,p,cap))return RCW_CONTEXT;
 z=_gz_rns_current_begin(req,frame);if(z)return z;
 /* Claim the unused staging PREFIX, never an assumed unused tail. Original
  * OISread is at its one capture BL with phase2/writer4 and no capture bytes
  * yet published. Guard/export use this slice sequentially; native capture
  * later overwrites it using the ORIGINAL p/cap, not p+scratch/cap-scratch. */
 z=_gz_rns_prefix_claim(req,frame,p,cap);if(z)return z;
 _gz_rns_trace(req,frame,RT_CLAIMED,0,cap);
 if(!owner(req,frame,p,cap,hg,role)){z=RCW_CONTEXT;goto abort;}
 if(role==RCW_RESTORE){z=_gz_rns_saved_ready(req,frame);if(z)goto abort;}
 _gz_rns_trace(req,frame,RT_GUARD_ENTER,0,0);
 z=_gz_rcg_preflight(req,frame,p,RCG_SCRATCH_BYTES);if(z){z=RCW_CONTEXT;goto abort;}
 _gz_rns_trace(req,frame,RT_GUARD_OK,0,0);
 _gz_rns_order.stage=RCW_GUARDED;
 if(!owner(req,frame,p,cap,hg,role)){z=RCW_CONTEXT;goto abort;}
 _gz_rns_trace(req,frame,RT_OWNER_BEFORE_RESET_OK,0,0);
 /* Typed binding must target authentic025f0384 and must not redirect that
  * public entry back to this component. Preflight checked initflag==1. */
 _gz_rns_trace(req,frame,RT_RESET_ENTER,0,0);
 _gz_rcw_native_owner_reset();
 _gz_rns_order.stage=RCW_RESET_RETURNED;
 _gz_rns_trace(req,frame,RT_RESET_RETURNED,0,0);
 if(!_gz_rcl_normalized_scope(req,frame)){z=RCW_CONTEXT;goto abort;}
 _gz_rns_trace(req,frame,RT_EXPORT_ENTER,0,0);
 z=_gz_rcl_export(req,frame,p,RCG_SCRATCH_BYTES);if(z){z=RCW_CONTEXT;goto abort;}
 _gz_rns_order.stage=RCW_EXPORTED;
 _gz_rns_trace(req,frame,RT_EXPORT_OK,0,0);
 if(!owner(req,frame,p,cap,hg,role)){z=RCW_CONTEXT;goto abort;}
 if(role==RCW_RESTORE){z=_gz_rns_saved_match(req,frame);if(z)goto abort;}
 /* Receipt is no longer usable as permission to export or repeat a reset.
  * No world/codec call occurs while its normalized-scope gate is true. */
 _gz_rns_order.active=0;
 _gz_rns_trace(req,frame,RT_CAPTURE_ENTER,0,0);
 z=_gz_rcw_native_capture(req,frame,p,cap,written);
 _gz_rns_trace(req,frame,RT_CAPTURE_RESULT,z,0);
 done=_gz_rns_capture_done(req,frame,z?0:*written,z);
 _gz_rns_trace(req,frame,RT_CAPTURE_DONE,done,0);
 _gz_rns_order.stage=0;
 return z?z:done;
abort:
 _gz_rns_order.active=0;
 _gz_rns_order.stage=0;
 return z;
}
