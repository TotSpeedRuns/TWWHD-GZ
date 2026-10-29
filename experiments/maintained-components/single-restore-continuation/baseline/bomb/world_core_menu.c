extern u32 _gz_osa_intent_match(u32,u32,u32,u32,u32),_gz_osa_raw_match(u32,u32,u32,u32);
#include "core_menu_features.h"
/* Ordinary chooser -> the exact existing no-bomb transaction. The queued
 * intent pins no snapshot while waiting. Original allocation identity and
 * bytes are requalified before the native owner acquires its real pin. */
enum {WCM_OFFSET=13216,WCM_BYTES=128};
static u32*wcm(void){return (u32*)ncb_core(WCM_OFFSET,33600-(WCM_OFFSET));}
static u32 wcm_hash(const u32*q){u32 i,h=2166136261u;const u8*b=(const u8*)q;for(i=0;i<WCM_BYTES;i++)h=(h^((i>=8&&i<12)||i>=124?0:b[i]))*16777619u;return h;}
static void wcm_seal(u32*q){q[31]=wcm_hash(q);__asm__ volatile("sync":::"memory");q[2]=1;}
static u32 wcm_valid(const u32*q){return q&&q[0]==0x57434d32u&&q[1]==WCM_BYTES&&q[2]==1&&!q[6]&&q[31]==wcm_hash(q);}
/* Authenticate the queued menu intent only. Current frame, selected
 * source, identity/hash and commit ticket remain separate worker checks.
 * This intentionally permits the next native frame and lets a changed
 * selection/source produce a terminal refusal through the normal ACK. */
u32 _gz_wcm_requested(u32 request){const u32*q=wcm();
 return GZ_CORE_MENU_RESTORE_ENABLED&&q&&wcm_valid(q)&&q[3]==1&&
  request&&request==q[4]&&q[5]<10&&_gz_wct_ack!=BAD&&request==_gz_wct_ack+1;
}
u32 _gz_wcm_choose(void){u32*q=wcm(),slot=_gz_pbac_selected,pin,p,n,id,gen,hash,z=28;
 if(!GZ_CORE_MENU_RESTORE_ENABLED||!q||slot>9||_gz_roh_user_guard()||_gz_boh_has_owner()||_gz_wct_request!=_gz_wct_ack||_gz_wct_ack==BAD||
  (q[2]&&(!wcm_valid(q)||q[3]==1))||_gz_osa_acquire(slot))return 28;
 pin=_gz_osa_pin_id();p=_gz_osa_source(pin);n=_gz_osa_bytes(pin);
 if(!p||p!=_gz_pbac_slots[slot]||!archive_ok(p)||R32(p,4)!=n||R32(p,12)<30||_gz_b294m_archive_validate(p,n,0))goto done;
 if(R32(p+R32(p,208),72)||bomb_count(_gz_pba_native_actor_queue)||gz_snapshot_activity_conflict(_gz_pbac_tools,0u)||_gz_pbac_pending||_gz_pbac_event||
  *(volatile u8*)&_gz_pbac_loading||_gz_pbac_world_resume_pending)goto done;
 id=_gz_osa_pin_identity(pin);gen=_gz_osa_pin_generation(pin);if(!id||!gen)goto done;hash=btr_hash((const void*)p,n);z=5;
done:_gz_osa_release_leaf();if(z!=5)return z;
 eks_zero(q,WCM_BYTES);q[0]=0x57434d32u;q[1]=WCM_BYTES;q[3]=1;q[4]=_gz_wct_ack+1;q[5]=slot;
 q[6]=0;q[7]=n;q[8]=id;q[9]=gen;q[10]=hash;q[11]=_gz_pbac_menu;q[13]=_gz_pbac_frame;wcm_seal(q);
 _gz_pbac_menu=1;_gz_wct_commit_ticket=q[4];__asm__ volatile("sync":::"memory");_gz_wct_request=q[4];return 5;}
u32 _gz_wcm_admission(u32 request,u32 frame,u32 slot){u32*q=wcm(),pin,p,n,z=42;
 if(!q||!q[2]||q[3]!=1)return 0;
 if(!wcm_valid(q)||q[4]!=request||q[5]!=slot||slot>9||frame!=_gz_pbac_frame||_gz_roh_user_guard()||
  !_gz_osa_intent_match(slot,q[8],q[9],q[7],q[10])||_gz_osa_acquire(slot))return 42;
 pin=_gz_osa_pin_id();p=_gz_osa_source(pin);n=_gz_osa_bytes(pin);
 if(_gz_osa_raw_match(pin,slot,p,n)&&n==q[7]&&p==_gz_pbac_slots[slot]&&_gz_osa_pin_identity(pin)==q[8]&&_gz_osa_pin_generation(pin)==q[9]&&
  btr_hash((const void*)p,n)==q[10])z=0;
 _gz_osa_release_leaf();return z;}
u32 _gz_wcm_admission_held(u32 request,u32 frame,u32 slot){u32*q=wcm(),pin,p,n,z=42;
 pin=_gz_swc_admission_pin(request,frame,slot);if(!pin)return 42;
 if(!q||!q[2]||q[3]!=1)return 0;
 if(!wcm_valid(q)||q[4]!=request||q[5]!=slot||slot>9||frame!=_gz_pbac_frame||_gz_roh_user_guard()||
  !_gz_osa_intent_match(slot,q[8],q[9],q[7],q[10]))return 42;
 p=_gz_osa_source(pin);n=_gz_osa_bytes(pin);
 if(_gz_osa_raw_match(pin,slot,p,n)&&n==q[7]&&p==_gz_pbac_slots[slot]&&_gz_osa_pin_identity(pin)==q[8]&&_gz_osa_pin_generation(pin)==q[9]&&
  btr_hash((const void*)p,n)==q[10])z=0;
 if(_gz_swc_admission_pin(request,frame,slot)!=pin)return 42;return z;}
void _gz_wcm_tick(void){u32*q=wcm(),z;if(!q||!wcm_valid(q)||q[3]!=1||_gz_wct_ack!=q[4])return;
 /* An unverified dirty failure keeps the original held owner. No source
  * body read or fabricated success occurs in this result dispatcher. */
 if(_gz_roh_user_guard()||_gz_osa_pin_id()||_gz_wgp_unreleased())return;
 z=!_gz_wct_report[5]&&_gz_wct_report[96]==1?15:28;
 q[3]=2;q[12]=z;q[14]=_gz_pbac_frame;q[15]=_gz_wct_report[8];wcm_seal(q);
 _gz_pbac_menu=q[11];_gz_status=z;_gz_start_toast(z);}
