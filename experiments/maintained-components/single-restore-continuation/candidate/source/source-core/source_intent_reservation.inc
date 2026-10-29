#include "source_intent_reservation.h"

/* Same actual SOURCE TU: the original OSA lease and adapter aliases must agree
 * before and after the genuine WCT-context callback. No saved body is read. */
static U soi_original_pin(U pin,U slot,U id,U gen,U used,U h){
 return pin&&!_gz_osa_validate(pin)&&so_lease_slot(pin)==slot&&
  so_identity(pin)==id&&so_generation(pin)==gen&&so_bytes(pin)==used&&
  so_raw_hash(pin)==h&&so_intent_match(slot,id,gen,used,h);
}
U _gz_osa_intent_reserve(U request,U frame,U slot,U id,U gen,U used,U h){
 Boundary held;OsaPin adapter;U pin,parent,again,cookie;
 if(!request||!frame||slot>9||!all_ok()||owner.entered||owner.writer||
  so_intent.cookie)return 0;
 pin=so_pin();if(!soi_original_pin(pin,slot,id,gen,used,h))return 0;
 copy(&adapter,&_gz_osa_state.pin,sizeof(adapter));boundary_take(&held);
 parent=_gz_wct_source_worker_context(request,frame,slot);
 if(!boundary_same(&held)||!same(&adapter,&_gz_osa_state.pin,sizeof(adapter))){
  (void)boundary_fault(0);return 0;
 }
 if(!parent||pin!=so_pin()||!soi_original_pin(pin,slot,id,gen,used,h))return 0;
 again=_gz_wct_source_worker_context(request,frame,slot);
 if(!boundary_same(&held)||!same(&adapter,&_gz_osa_state.pin,sizeof(adapter))){
  (void)boundary_fault(0);return 0;
 }
 if(again!=parent||pin!=so_pin()||!soi_original_pin(pin,slot,id,gen,used,h))return 0;
 cookie=soi_arm(&so_intent,request,slot,id,gen,used,h);
 __asm__ volatile("sync":::"memory");return cookie;
}
U _gz_osa_intent_reservation_match(U cookie,U request,U slot,U id,U gen,U used,U h){
 return all_ok()&&!owner.entered&&!owner.writer&&
  soi_matches(&so_intent,cookie,request,slot,id,gen,used,h);
}
U _gz_osa_intent_release(U cookie,U request){
 if(!all_ok()||owner.entered||owner.writer||!cookie||!request||
  so_intent.cookie!=cookie||so_intent.request!=request)return SO_BAD;
 /* No callback, source read, allocator or native call after validation. */
 soi_clear_leaf(&so_intent);__asm__ volatile("sync":::"memory");return 0;
}
