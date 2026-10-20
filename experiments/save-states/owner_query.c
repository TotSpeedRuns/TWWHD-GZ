/* Original owner9 remains byte-identical as an include. This separate private
 * unit adds a read-only no-callback context query; no final-main file is edited. */
#include "owner.c"
#include "bridge.h"
/* Actor Draw can mutate animation buffers and shared resource attachments.
 * Match the existing global Management hold without following actor/source
 * pointers or granting any restore authority. Corrupt owner metadata stays
 * frozen for the original owner's normal fault/recovery handling. */
u32 _gz_roh_render_hold(void){return idle()?0:1;}
u32 _gz_roh_gl_context(u32 token,u32 frame,u32 action){
 const RohOwner*o=&_gz_roh_owner;const RohSample*s=&o->sample;u32 z,f,life;
 if(action==GL_FORGET)return token&&idle()?1:0;
 if(action!=GL_PREPARE&&action!=GL_CHECK)return 0;
 if(!owned(token)||failed()||!o->frame_open||!o->limit||o->limit>ROH_MAX_FRAMES)return 0;
 /* Reject rebound source/player/camera globals as integers before the
  * original sampler follows any current allocation. The pinned original
  * owner sample is never replaced by this query. */
 if(s->slot>9||_gz_roh_live_slots[s->slot]!=s->archive||_gz_roh_live_bflower!=s->working_base||_gz_roh_live_link!=s->link||_gz_roh_live_camera_index!=s->camera_index||s->camera_index>1||_gz_roh_live_camera_table[s->camera_index*13]!=s->camera||_gz_roh_registry_revision!=s->registry_revision||!choice_current(_gz_roh_pinned_choice)||_gz_roh_live_epoch!=s->epoch||_gz_roh_live_stage[0]!=s->stage0||_gz_roh_live_stage[1]!=s->stage1||_gz_roh_live_room!=s->room||_gz_roh_live_layer!=s->layer)return 0;
 if(!span(s->link,0x6b00)||U16(s->link,8)!=168||U8(s->link,12)!=2||U32(s->link,4)!=s->pid||U32(s->link,0x65cc)!=s->morf)return 0;
 f=_gz_roh_live_frame;if(frame!=f||f-o->last_frame>=0x80000000u)return 0;
 if(o->phase==ROH_CLEAN){if(o->mutated||o->inflight!=ROH_OBSERVE||o->last_frame!=f)return 0;life=0;}
 else if(o->phase==ROH_MUTATING){if(action==GL_PREPARE||o->mutated!=1||(o->inflight&&o->inflight!=ROH_LIFETIME)||(o->inflight&&o->last_frame!=f))return 0;life=1;}
 else return 0;
 /* PREPARE validates original full immutable source once. CHECK uses the
  * original shallow common policy, independently of RI topology exceptions. */
 z=common_validate(action==GL_PREPARE,1,life);if(z)return 0;
 return owned(token)&&!failed()&&o->frame_open&&frame==_gz_roh_live_frame&&
  (life?(o->phase==ROH_MUTATING&&o->mutated==1&&(!o->inflight||o->inflight==ROH_LIFETIME)):
        (o->phase==ROH_CLEAN&&!o->mutated&&o->inflight==ROH_OBSERVE&&o->last_frame==frame));
}

#include "owner_direct_recovery.c"
