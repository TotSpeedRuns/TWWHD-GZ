/* Delay a save request while native UI is not representable. This runs before
 * ANY capture/slot write; it never synthesizes an unknown UI as gameplay.
 * Exact request-time world capture during transitions is still future work. */
extern volatile u32 _gz_pending;
extern u8 _gz_camera_saved[];
extern volatile u32 _gz_slots_selected;
static u32 uic_kind=8,uic_slot;
static u32 uic_probe[387],uic_waiting,uic_first_frame,uic_epoch,uic_stage0,uic_stage1,uic_room,uic_layer,uic_player;
u32 _gz_uic_gate_report[24]={0x55494731u,96,1};
static void uic_wait_clear(void){uic_waiting=uic_first_frame=uic_epoch=uic_stage0=uic_stage1=uic_room=uic_layer=uic_player=0;}
static void uic_wait_begin(u32 f){uic_waiting=1;uic_first_frame=f;uic_epoch=_gz_pbac_epoch;uic_stage0=_gz_pbac_stage[0];uic_stage1=_gz_pbac_stage[1];uic_room=_gz_pbac_room;uic_layer=_gz_pbac_layer;uic_player=_gz_pbac_player;uic_slot=_gz_slots_selected;}
static u32 uic_same_world(void){return (uic_kind!=10||uic_slot==_gz_slots_selected)&&uic_epoch==_gz_pbac_epoch&&uic_stage0==_gz_pbac_stage[0]&&uic_stage1==_gz_pbac_stage[1]&&uic_room==_gz_pbac_room&&uic_layer==_gz_pbac_layer&&uic_player==_gz_pbac_player;}
void _gz_uic_capture_tick(void){
 /* A restore, warp, cancellation or new menu command replaces pending8
  * before dispatch. Retire only this private wait ticket in that case. */
 if(uic_waiting&&_gz_pending!=uic_kind){_gz_uic_gate_report[17]++;uic_wait_clear();}
}
static u32 uic_capture_gate(void){u32 f=_gz_pbac_frame,delta;
 _gz_uic_gate_report[3]++;_gz_uic_gate_report[4]=f;
 if(_gz_roh_user_guard()){if(!_gz_pending)_gz_pending=uic_kind;_gz_uic_gate_report[5]=5;return 5;}
 if(uic_waiting&&!uic_same_world()){
  _gz_uic_gate_report[11]++;uic_wait_clear();_gz_uic_gate_report[5]=27;return 27;
 }
 if(uic_waiting&&((f-uic_first_frame)>120||(f-uic_first_frame)>=0x80000000u)){
  _gz_uic_gate_report[10]++;uic_wait_clear();_gz_uic_gate_report[5]=27;return 27;
 }
 _gz_ui_current_capture((u32)uic_probe);
 _gz_uic_gate_report[6]=uic_probe[384];_gz_uic_gate_report[7]=_gz_ui_capture_report[7];
 if(uic_probe[384]==0x55495331u&&uic_probe[385]<=1&&uic_probe[386]<=2){
  _gz_uic_gate_report[8]=uic_waiting?f-uic_first_frame:0;
  _gz_uic_gate_report[9]++;if(!uic_waiting)uic_wait_begin(f);_gz_uic_gate_report[5]=0;return 0;
 }
 if(!uic_waiting)uic_wait_begin(f);
 delta=f-uic_first_frame;
 if(delta>120||delta>=0x80000000u){_gz_uic_gate_report[10]++;uic_wait_clear();_gz_uic_gate_report[5]=27;return 27;}
 if(_gz_pending){_gz_uic_gate_report[12]++;uic_wait_clear();_gz_uic_gate_report[5]=5;return 5;}
 /* The ordinary frame dispatcher already consumed pending8. Requeue only
  * that same request; there is no host/controller input or snapshot write. */
 _gz_pending=uic_kind;_gz_uic_gate_report[13]++;_gz_uic_gate_report[14]=delta;_gz_uic_gate_report[5]=5;return 5;
}
/* Native capture may itself return transient BUSY while lazy resources or
 * previous travel settle. Its original discard path ran before this callback.
 * Retry the same request under the original deadline, never a new deadline. */
u32 _gz_uic_capture_result(u32 result){u32 delta=_gz_pbac_frame-uic_first_frame;
 _gz_uic_gate_report[18]=result;
 if(result==5&&uic_waiting){
  if(!uic_same_world()||delta>120||delta>=0x80000000u){_gz_uic_gate_report[10]++;uic_wait_clear();return 27;}
  if(_gz_pending){_gz_uic_gate_report[12]++;uic_wait_clear();return result;}
  _gz_pending=uic_kind;_gz_uic_gate_report[19]++;return 5;
 }
 if(result==14)_gz_uic_gate_report[20]++;
 else _gz_uic_gate_report[21]++;
 uic_wait_clear();return result;
}
u32 _gz_uic_publication_check(void){u32 p=(u32)_gz_camera_saved;
 if(R32(p,1536)!=0x55495331u||R32(p,1540)>1||R32(p,1544)>2){_gz_uic_gate_report[15]++;return 27;}
 _gz_uic_gate_report[16]++;return 0;
}

/* The legacy single-state request and selected-slot request have distinct
 * retry identities. Opening/canceling a chooser never enters either capture. */
u32 _gz_uic_capture_gate(void){
 if(uic_kind!=8){uic_wait_clear();uic_kind=8;}return uic_capture_gate();
}
u32 _gz_uic_selected_capture_gate(void){
 if(uic_kind!=10){uic_wait_clear();uic_kind=10;}
 if(_gz_slots_selected>=10){uic_wait_clear();return 28;}
 return uic_capture_gate();
}
