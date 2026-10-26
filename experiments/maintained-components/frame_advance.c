/* Frame-advance requests are authored control metadata. Only the existing
 * native pause predicate is intercepted; no actor, timer or save byte is set.
 * VPAD queues intent. The completed-frame callback owns pause/step grants. */
typedef unsigned int U;
extern volatile U _gz_state[],_gz_page,_gz_pending,_gz_game_counter;
extern volatile U _gz_life_phase,_gz_mht_phase;
extern volatile U _gz_tools_flags;
extern volatile unsigned char _gz_loading_store;
extern U _gz_roh_user_guard(void);
extern U _gz_bind_masks[];
extern unsigned char _gz_fad_native_system_slot[];
extern U _gz_fad_native_ui_request(void),_gz_fad_native_ui_primary(U),_gz_fad_native_ui_secondary(U);
extern void _gz_game_text(U,U,U,const char*);
enum {PAGE=53,DOWN=0x100,UP=0x200,TOGGLE=4};
volatile U _gz_fad_enabled,_gz_fad_paused,_gz_fad_forced,_gz_fad_commands;
U _gz_fad_report[16]={0x46414431,64};
static U down_held,up_held,last_frame,have_frame,steps,boundaries,last_step_frame,have_step;
/* A held/released D-pad sample belongs to Frame Advance only after its
 * pause/step/resume command was accepted. Merely enabling the feature must
 * leave ordinary Up (Wind Waker) and its release with the native game. */
static U owned_dpad;
/* Native input is polled while simulation is paused. Retain fresh action
 * edges until the requested simulation frame, rather than losing them in
 * an earlier paused poll. These are controller bits, never actor stores. */
enum {ACTIONS=0xfcf0};
static U action_held,pending_edges,step_edges,simulated_hold;
U _gz_fad_replay_edges;
static void queue(U bits){__sync_fetch_and_or(&_gz_fad_commands,bits);}
static U match(U edge,U held,U mask){return mask&&(edge&mask)&&(held&mask)==mask;}
void _gz_fad_open(void){_gz_page=PAGE;_gz_state[2]=0;}
U _gz_fad_input(U edges,U held,U unused,U native_ui){U now_down=held&DOWN,now_up=held&UP;U was=owned_dpad;U d=now_down&&!down_held,u=now_up&&!up_held;U i,rising=(held&~action_held)&ACTIONS;
 (void)unused;down_held=now_down;up_held=now_up;action_held=held&ACTIONS;
 owned_dpad&=held;
 if(_gz_state[0]){
  pending_edges=0;owned_dpad=0;
  if(_gz_page!=PAGE)return 0;
  if(match(edges,held,_gz_bind_masks[6])){_gz_page=58;_gz_state[2]=0;return 1;}
  if(match(edges,held,_gz_bind_masks[5]))queue(TOGGLE);
  return 1;
 }
 if(!_gz_fad_enabled||native_ui){pending_edges=0;owned_dpad=0;return 0;}
 /* Existing position, state and menu chords retain priority. In particular
  * a shoulder prefix must not accidentally pause on its D-pad component. */
 for(i=0;i<3;i++)if(_gz_bind_masks[i]&&(held&_gz_bind_masks[i])==_gz_bind_masks[i]){pending_edges=0;return 0;}
 if((held&0x30u)==0x30u){pending_edges=0;return 0;} /* existing shoulder shortcuts */
 if((_gz_tools_flags&8u)&&((held&_gz_bind_masks[7])||held&0x400u)){pending_edges=0;return 0;}
 if(held==0x230u&&(_gz_tools_flags&64u)){pending_edges=0;return 0;} /* Zombie Hover Auto start */
 if(_gz_fad_paused&&_gz_fad_forced)pending_edges=(pending_edges|rising)&held;
 else pending_edges=0;
 if(u&&(edges&UP)&&_gz_fad_paused){owned_dpad|=UP;queue(2);}
 else if(d&&(edges&DOWN)){owned_dpad|=DOWN;if(_gz_fad_paused){step_edges=pending_edges;pending_edges=0;}queue(1);}
 return (owned_dpad||was)?2:0;
}
/* Controller transport only. Preserve all action buttons, floating-point
 * stick bytes and non-button fields. Counts remain within the native read's
 * validated batch. Pro and VPAD have distinct, qualified D-pad encodings. */
void _gz_fad_mask_vpad(U *sample,U count){U i;if(!sample||count>32)return;
 for(i=0;i<count;i++,sample+=0xac/4){sample[0]&=~0x300u;sample[1]&=~0x300u;sample[2]&=~0x300u;}
}
void _gz_fad_mask_pro(U *sample,U count){U i;if(!sample||count>32)return;
 for(i=0;i<count;i++,sample+=0xf0/4)if(((unsigned char*)sample)[0x5c]==0x1f){sample[24]&=~0x4001u;sample[25]&=~0x4001u;sample[26]&=~0x4001u;}
}
void _gz_fad_replay_sample(U *sample){U e=_gz_fad_replay_edges;if(!sample)return;
 sample[0]|=e;sample[1]|=e;sample[2]&=~e;
}
/* The game computes its own button transitions from hold history. During
 * stopped simulation, keep that history at the last simulated input. UI
 * buttons and all stick bytes remain live. A step exposes the new hold and
 * its retained edge together, so the native pad manager sees the transition. */
void _gz_fad_project_sample(U *sample){if(!sample)return;
 if(_gz_fad_paused&&_gz_fad_forced){sample[0]=(sample[0]&~ACTIONS)|simulated_hold;sample[1]&=~ACTIONS;sample[2]&=~ACTIONS;}
 else{_gz_fad_replay_sample(sample);simulated_hold=sample[0]&ACTIONS;}
}
void _gz_fad_replay_vpad(U *sample,U count){U i,*first=sample;if(!sample||count>32)return;
 for(i=0;i<count;i++,sample+=0xac/4){_gz_fad_project_sample(sample);sample[0]&=~0x300u;sample[1]&=~0x300u;sample[2]&=~0x300u;}
 if(count&&!(_gz_fad_paused&&_gz_fad_forced))simulated_hold=first[0]&ACTIONS;
}
/* Inverse of the existing authored canonical Pro-button mapping. */
extern U _gz_pro_buttons(U);
U _gz_fad_pro_edges(U edges){U i,bits=0;for(i=0;i<32;i++)if(_gz_pro_buttons(1u<<i)&edges)bits|=1u<<i;return bits;}
void _gz_fad_replay_pro(U *sample,U count){U i,e,mask,hold,*first=sample;if(!sample||count>32)return;
 e=_gz_fad_pro_edges(_gz_fad_replay_edges);
 mask=_gz_fad_pro_edges(ACTIONS);hold=_gz_fad_pro_edges(simulated_hold);
 for(i=0;i<count;i++,sample+=0xf0/4)if(((unsigned char*)sample)[0x5c]==0x1f){
  if(_gz_fad_paused&&_gz_fad_forced){sample[24]=(sample[24]&~mask)|hold;sample[25]&=~mask;sample[26]&=~mask;}
  else{sample[24]|=e;sample[25]|=e;sample[26]&=~e;}
  sample[24]&=~0x4001u;sample[25]&=~0x4001u;sample[26]&=~0x4001u;
 }
 if(!(_gz_fad_paused&&_gz_fad_forced))for(i=0;i<count;i++)if(((unsigned char*)(first+i*(0xf0/4)))[0x5c]==0x1f){simulated_hold=_gz_pro_buttons(first[i*(0xf0/4)+24])&ACTIONS;break;}
}
/* Exact native UI portion of the ordinary pause predicate, excluding the
 * damage/hit-stop timer and our forced-pause bit. Use original query APIs;
 * no saved/current heap pointer is substituted and no UI field is written. */
static U native_ui_active(void){U system=*(volatile U*)_gz_fad_native_system_slot;
 if(!system)return 0;
 return _gz_fad_native_ui_request()||_gz_fad_native_ui_primary(system)||_gz_fad_native_ui_secondary(system);
}
U _gz_fad_input_transport(U *sample,U edge,U repeat){U held=sample[0],ui=0,result;
 if(!_gz_state[0]&&_gz_fad_enabled&&((_gz_fad_paused)||held&(DOWN|UP)))ui=native_ui_active();
 result=_gz_fad_input(edge,held,repeat,ui);
 if(_gz_state[0]||ui||!_gz_fad_enabled){simulated_hold=held&ACTIONS;return result;}
 if(_gz_fad_paused||_gz_fad_replay_edges){
  /* Reserved commands remain authoritative even on a granted step. */
  U i;for(i=0;i<3;i++)if(_gz_bind_masks[i]&&(held&_gz_bind_masks[i])==_gz_bind_masks[i])return result;
  if((held&0x30u)==0x30u)return result;
  if((_gz_tools_flags&8u)&&((held&_gz_bind_masks[7])||held&0x400u))return result;
  if(held==0x230u&&(_gz_tools_flags&64u))return result;
  _gz_fad_project_sample(sample);return 3;
 }
 simulated_hold=held&ACTIONS;
 return result;
}
/* Pure metadata transition, shared by the real boundary and emitted-code
 * tests. Calling this never grants restore/heap/native-write authority. */
void _gz_fad_boundary(U frame,U blocked){U cmd,grant=0;
 if(have_frame&&frame==last_frame)return;
 have_frame=1;last_frame=frame;boundaries++;
 _gz_fad_replay_edges=0;
 cmd=__sync_lock_test_and_set(&_gz_fad_commands,0);
 if(cmd&TOGGLE){_gz_fad_enabled^=1;_gz_fad_paused=0;have_step=0;}
 if(!_gz_fad_enabled){_gz_fad_paused=0;_gz_fad_forced=0;pending_edges=0;step_edges=0;}
 else{
  if(cmd&2){_gz_fad_paused=0;step_edges=0;pending_edges=0;}
  else if(cmd&1){if(_gz_fad_paused)grant=1;else{_gz_fad_paused=1;have_step=0;}}
  if(blocked){
   _gz_fad_forced=0;
   if(grant)queue(1); /* defer one requested step until the transition is safe */
  }else{_gz_fad_forced=_gz_fad_paused&&!grant;if(grant){steps++;_gz_fad_replay_edges=step_edges;step_edges=0;last_step_frame=frame;have_step=1;}}
 }
 _gz_fad_report[2]=_gz_fad_enabled;_gz_fad_report[3]=_gz_fad_paused;
 _gz_fad_report[4]=_gz_fad_forced;_gz_fad_report[5]=cmd;_gz_fad_report[6]=frame;
 _gz_fad_report[7]=steps;_gz_fad_report[8]=blocked;_gz_fad_report[9]=boundaries;
 _gz_fad_report[10]=pending_edges;_gz_fad_report[11]=_gz_fad_replay_edges;
}
void _gz_fad_tick(void){U blocked=0;
 if(_gz_fad_enabled||_gz_fad_commands){
  blocked=_gz_loading_store||_gz_life_phase||_gz_mht_phase||_gz_roh_user_guard()||_gz_pending==3||_gz_pending==4||_gz_pending==7||native_ui_active();
 }
 _gz_fad_boundary(_gz_game_counter,blocked);
}
static void text(U x,U y,const char*s){_gz_game_text(0,x,y,s);}
void _gz_fad_root_draw(void){text(4,10,"Frame Advance");}
void _gz_fad_draw(void){
 text(2,1,"Frame Advance");text(2,4,">");text(4,4,"Frame Advance");text(22,4,_gz_fad_enabled?"On":"Off");
 text(4,7,"D-pad Down: Pause / advance one frame");text(4,8,"D-pad Up: Resume");
 text(4,11,"Menu and save-state hotkeys keep priority.");
 text(4,19,"A: Toggle   B: Back");
}
const char *_gz_fad_hud_text(void){
 if(!_gz_fad_paused)return "> Frame Advance: Playing";
 if(have_step&&_gz_game_counter-last_step_frame<10)return "> Frame Advance: Step";
 return "|| Frame Advance: Paused";
}
void _gz_fad_draw_hud(void){
 if(!_gz_fad_enabled||_gz_state[0])return;
 text(33,3,_gz_fad_hud_text());
}
