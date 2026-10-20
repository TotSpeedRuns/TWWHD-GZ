/* Camera-only no-lifetime recovery. Original BOH recovery stays unchanged.
 * This second ticket never reads an archive body or changes a native lifetime.
 * Only exact failed current-death camera work can request it; exact original
 * actor topology and all camera before-image bytes must close before release. */
extern u32 _gz_dck_recovery_scope(u32),_gz_dck_finish_scope(u32,u32);
extern u32 _gz_doh_recovery_scope(u32,u32),_gz_som_direct_recovery_finish_check(u32);
extern u32 _gz_dck_closure_seal;
RohRecovered _gz_roh_direct_recovered;
u32 _gz_roh_direct_recovered_seal;
static u32 drc_finish_entered;
static RohRecovery drc_recovery;
static u32 drc_recovery_seal,drc_recovery_entered;
static u32 drc_used(void){u32 i;
 if(drc_recovery_seal)return 1;
 for(i=0;i<sizeof(drc_recovery)/4;i++)if(((const u32*)&drc_recovery)[i])return 1;
 return 0;
}
static u32 drc_sealed(void){
 return drc_recovery.magic==0x52524331u&&drc_recovery.bytes==sizeof(RohRecovery)&&
  drc_recovery.valid==1&&drc_recovery.token&&drc_recovery.limit&&drc_recovery.limit<=ROH_MAX_FRAMES&&
  drc_recovery_seal==hash((u32)&drc_recovery,sizeof(drc_recovery));
}
/* Read-only original allocation identities. Topology and source contents are
 * deliberately absent; a separate sealed typed lane owns their comparison. */
static u32 drc_live_context(u32 token,u32 frame){
 const RohSample*s=&_gz_roh_owner.sample;u32 p,m,c;
 if(!owned(token)||_gz_roh_owner.phase!=ROH_FAILED_DIRTY||_gz_roh_owner.mutated!=1||
  !_gz_roh_owner.error||frame!=_gz_roh_live_frame||!scene_epoch_valid()||!base_quiet()||
  !choice_current(_gz_roh_pinned_choice)||_gz_roh_live_loading||(_gz_roh_live_event&&!_gz_doh_recovery_scope(token,frame))||
  _gz_roh_live_epoch!=s->epoch||_gz_roh_live_stage[0]!=s->stage0||_gz_roh_live_stage[1]!=s->stage1||
  _gz_roh_live_room!=s->room||_gz_roh_live_layer!=s->layer||s->slot>9||s->camera_index>1||
  _gz_roh_live_slots[s->slot]!=s->archive||_gz_roh_live_bflower!=s->working_base||
  (u32)_gz_roh_live_pot!=s->pot_work||_gz_roh_registry_revision!=s->registry_revision||
  !span(s->working_base,0x60000)||s->bank!=s->working_base+0x50000||
  s->registry!=s->working_base+0x4a600||!s->registry_revision)return 0;
 p=_gz_roh_live_link;
 if(p!=s->link||!span(p,0x6b00)||s->profile!=168||U16(p,8)!=168||U8(p,12)!=2||
  U32(p,4)!=s->pid||U32(p,0x444)!=s->resource444)return 0;
 m=U32(p,0x65cc);
 if(m!=s->morf||!span(m,0xb0)||U32(m,0x1c)!=s->model||U32(m,0x20)!=s->animation)return 0;
 if(_gz_roh_live_camera_index!=s->camera_index)return 0;
 c=_gz_roh_live_camera_table[s->camera_index*13];
 if(c!=s->camera||s->camera_self!=c||s->camera_link!=p||s->camera_bound_index!=s->camera_index||
  !span(c,0xa00)||U32(c,0x248)!=c||U8(c,0x24c)!=1||U8(c,0x24d)||
  U32(c,0x368)!=s->camera_index||U32(c,0x370)!=p)return 0;
 return frame==_gz_roh_live_frame&&owned(token)&&_gz_roh_owner.phase==ROH_FAILED_DIRTY&&
  _gz_roh_owner.mutated==1&&choice_current(_gz_roh_pinned_choice);
}
u32 _gz_roh_direct_recovery_begin(u32 token,u32 max_frames){
 u32 frame,h,z,i;RohRecovery next;
 if(drc_recovery_entered||!max_frames||max_frames>ROH_MAX_FRAMES)return ROH_BUSY;
 if(drc_used()&&(!drc_sealed()||token<=drc_recovery.token))return ROH_BUSY;
 frame=_gz_roh_live_frame;
 if(!drc_live_context(token,frame))return ROH_OWNER;
 h=hash((u32)&_gz_roh_owner,sizeof(_gz_roh_owner));
 drc_recovery_entered=1;z=_gz_dck_recovery_scope(token);drc_recovery_entered=0;
 if(z!=1||!drc_live_context(token,frame)||h!=hash((u32)&_gz_roh_owner,sizeof(_gz_roh_owner)))return ROH_OWNER;
 next.magic=0x52524331u;next.bytes=sizeof(next);next.valid=1;next.token=token;
 next.start=frame;next.limit=max_frames;next.owner_hash=h;next.choice=_gz_roh_pinned_choice;
 for(i=0;i<sizeof(next)/4;i++)((u32*)&drc_recovery)[i]=((const u32*)&next)[i];
 drc_recovery_seal=hash((u32)&drc_recovery,sizeof(drc_recovery));
 return 0;
}
u32 _gz_roh_direct_recovery_common(u32 token,u32 frame){
 u32 delta;
 if(drc_recovery_entered||!drc_sealed()||drc_recovery.token!=token)return ROH_OWNER;
 delta=frame-drc_recovery.start;
 if(delta>=0x80000000u||delta>drc_recovery.limit)return ROH_TIMEOUT;
 if(drc_recovery.choice!=_gz_roh_pinned_choice||
  drc_recovery.owner_hash!=hash((u32)&_gz_roh_owner,sizeof(_gz_roh_owner))||
  !drc_live_context(token,frame))return ROH_OWNER;
 return drc_sealed()&&frame==_gz_roh_live_frame&&
  drc_recovery.owner_hash==hash((u32)&_gz_roh_owner,sizeof(_gz_roh_owner))?0:ROH_OWNER;
}
u32 _gz_roh_direct_recovery_baseline(u32 token,u32 frame){
 const RohSample*s=&_gz_roh_owner.sample;u32 q,a,n=0,h=2166136261u,z;
 z=_gz_roh_direct_recovery_common(token,frame);if(z)return z;
 if(_gz_roh_live_create_head||_gz_roh_live_delete_head)return ROH_REGISTRY;
 q=_gz_roh_live_actor_head;if(q!=s->actor_head)return ROH_REGISTRY;
 while(q){
  if(n++==4096||!span(q,16))return ROH_REGISTRY;
  a=U32(q,12);h=mix(h,q);h=mix(h,U32(q,8));h=mix(h,a);
  if(a){if(!span(a,0x300))return ROH_REGISTRY;h=mix(h,U32(a,4));h=mix(h,U16(a,8));h=mix(h,U32(a,0xb0));h=mix(h,U32(a,0x2e8));}
  q=U32(q,8);
 }
 if(n!=s->actor_count||h!=s->actor_topology_hash||_gz_roh_live_actor_head!=s->actor_head||
  _gz_roh_live_create_head||_gz_roh_live_delete_head)return ROH_REGISTRY;
 return _gz_roh_direct_recovery_common(token,frame);
}

static u32 drc_recovered_used(void){u32 i;if(_gz_roh_direct_recovered_seal)return 1;
 for(i=0;i<sizeof(RohRecovered)/4;i++)if(((const u32*)&_gz_roh_direct_recovered)[i])return 1;return 0;
}
static u32 drc_recovered_sealed(void){const RohRecovered*r=&_gz_roh_direct_recovered;
 return r->magic==0x52464331u&&r->bytes==sizeof(*r)&&r->version==1&&r->valid==1&&r->token&&r->error&&r->pin&&
  r->closure_hash&&r->owner.token==r->token&&r->owner.error==r->error&&r->owner.phase==ROH_FAILED_DIRTY&&
  r->owner.hold==1&&r->owner.mutated==1&&r->owner_hash==hash((u32)&r->owner,sizeof(RohOwner))&&
  _gz_roh_direct_recovered_seal==hash((u32)r,sizeof(*r));
}
u32 _gz_roh_direct_recovery_finish(u32 token,u32 frame){RohRecovered*r=&_gz_roh_direct_recovered;u32 z,i,h,c,pin,committed;
 if(drc_finish_entered)return ROH_BUSY;
 if(drc_recovered_used()&&(!drc_recovered_sealed()||token<=r->token))return ROH_OWNER;
 drc_finish_entered=1;
 z=_gz_roh_direct_recovery_baseline(token,frame);if(z)goto denied;
 /* No native lifetime call occurs in this lane. Only the sealed, returned
  * camera callback with complete original before-image closure can retire
  * the camera service ID; the failed receipt stays unchanged until then. */
 if(_gz_roh_owner.inflight&&_gz_dck_finish_scope(token,frame)!=1){z=ROH_SERVICE;goto denied;}
 h=hash((u32)&_gz_roh_owner,sizeof(RohOwner));pin=_gz_osa_pin_id();
 if(!pin){z=ROH_OWNER;goto denied;}
 z=_gz_dck_finish_scope(token,frame);if(z!=1){z=ROH_OWNER;goto denied;}
 c=_gz_dck_closure_seal;if(!c){z=ROH_OWNER;goto denied;}
 z=_gz_som_direct_recovery_finish_check(token);if(z)goto denied;
 z=_gz_roh_direct_recovery_baseline(token,frame);if(z)goto denied;
 if(h!=hash((u32)&_gz_roh_owner,sizeof(RohOwner))||c!=_gz_dck_closure_seal||pin!=_gz_osa_pin_id()||
  _gz_dck_finish_scope(token,frame)!=1){z=ROH_OWNER;goto denied;}
 /* All fallible/native/source reads end above. This leaf helper have
  * fixed private destinations; none calls a native SDK, follows the archive,
  * frees its allocation or publishes a generic world-resume permission. */
 r->valid=0;r->magic=0x52464331u;r->bytes=sizeof(*r);r->version=1;r->token=token;r->frame=frame;
 r->error=_gz_roh_owner.error;r->pin=pin;r->closure_hash=c;r->owner_hash=h;
 for(i=0;i<sizeof(RohOwner)/4;i++)((u32*)&r->owner)[i]=((const u32*)&_gz_roh_owner)[i];
 r->valid=1;_gz_roh_direct_recovered_seal=hash((u32)r,sizeof(*r));
 committed=_gz_roh_owner.committed_token;
 _gz_som_recovery_finish_leaf();
 for(i=0;i<sizeof(drc_recovery)/4;i++)((u32*)&drc_recovery)[i]=0;
 drc_recovery_seal=0;
 for(i=3;i<sizeof(RohOwner)/4;i++)if(i!=4)((u32*)&_gz_roh_owner)[i]=0;
 _gz_roh_owner.committed_token=committed;
 __asm__ volatile("sync":::"memory");_gz_roh_owner.hold=0;
 drc_finish_entered=0;return 0;
denied:drc_finish_entered=0;return z;
}

/* Original common source/world checks under the camera-only MUTATING lane.
 * Success is consistency only, never SDK, lifetime or release permission. */
extern u32 _gz_dck_owner_role(u32,u32,u32);
u32 _gz_roh_direct_camera_common(u32 token,u32 frame,u32 deep){u32 z;
 if(deep>1||!owned(token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||
  _gz_roh_owner.mutated!=1||_gz_roh_owner.inflight!=ROH_CAMERA||!_gz_roh_owner.frame_open||
  _gz_roh_owner.last_frame!=frame||frame!=_gz_roh_live_frame||
  _gz_dck_owner_role(token,frame,ROH_MUTATING)!=1)return ROH_OWNER;
 z=common_validate(deep,1,0);if(z)return z;
 return owned(token)&&!failed()&&frame==_gz_roh_live_frame&&_gz_roh_owner.inflight==ROH_CAMERA&&
  _gz_roh_owner.phase==ROH_MUTATING&&_gz_roh_owner.mutated==1&&_gz_roh_owner.frame_open&&
  _gz_dck_owner_role(token,frame,ROH_MUTATING)==1?0:ROH_OWNER;
}

/* Independent no-lifetime body consistency under actual service2. Same
 * original source/actor/camera/topology guards, with the sealed direct role. */
u32 _gz_roh_direct_body_common(u32 token,u32 frame,u32 deep){u32 z;
 if(deep>1||!owned(token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||
  _gz_roh_owner.mutated!=1||_gz_roh_owner.inflight!=ROH_LIGHT||!_gz_roh_owner.frame_open||
  _gz_roh_owner.last_frame!=frame||frame!=_gz_roh_live_frame||
  _gz_dck_owner_role(token,frame,ROH_MUTATING)!=1)return ROH_OWNER;
 z=common_validate(deep,1,0);if(z)return z;
 return owned(token)&&!failed()&&frame==_gz_roh_live_frame&&_gz_roh_owner.inflight==ROH_LIGHT&&
  _gz_roh_owner.phase==ROH_MUTATING&&_gz_roh_owner.mutated==1&&_gz_roh_owner.frame_open&&
  _gz_dck_owner_role(token,frame,ROH_MUTATING)==1?0:ROH_OWNER;
}

/* Same original consistency checks for the fixed direct persistent/clock service. */
u32 _gz_roh_direct_state_common(u32 token,u32 frame,u32 deep){u32 z;
 if(deep>1||!owned(token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||
  _gz_roh_owner.mutated!=1||_gz_roh_owner.inflight!=ROH_FTRAVEL||!_gz_roh_owner.frame_open||
  _gz_roh_owner.last_frame!=frame||frame!=_gz_roh_live_frame||
  _gz_dck_owner_role(token,frame,ROH_MUTATING)!=1)return ROH_OWNER;
 z=common_validate(deep,1,0);if(z)return z;
 return owned(token)&&!failed()&&frame==_gz_roh_live_frame&&_gz_roh_owner.inflight==ROH_FTRAVEL&&
  _gz_roh_owner.phase==ROH_MUTATING&&_gz_roh_owner.mutated==1&&_gz_roh_owner.frame_open&&
  _gz_dck_owner_role(token,frame,ROH_MUTATING)==1?0:ROH_OWNER;
}

/* Dedicated no-lifetime UI cancellation consistency; no borrowed family ID. */
u32 _gz_roh_direct_ui_common(u32 token,u32 frame,u32 deep){u32 z;
 if(deep>1||!owned(token)||failed()||_gz_roh_owner.phase!=ROH_MUTATING||
  _gz_roh_owner.mutated!=1||_gz_roh_owner.inflight!=ROH_UI||!_gz_roh_owner.frame_open||
  _gz_roh_owner.last_frame!=frame||frame!=_gz_roh_live_frame||
  _gz_dck_owner_role(token,frame,ROH_MUTATING)!=1||_gz_duc_service_scope(token,frame)!=1)return ROH_OWNER;
 z=common_validate(deep,1,0);if(z)return z;
 return owned(token)&&!failed()&&frame==_gz_roh_live_frame&&_gz_roh_owner.inflight==ROH_UI&&
  _gz_roh_owner.phase==ROH_MUTATING&&_gz_roh_owner.mutated==1&&_gz_roh_owner.frame_open&&
  _gz_dck_owner_role(token,frame,ROH_MUTATING)==1&&_gz_duc_service_scope(token,frame)==1?0:ROH_OWNER;
}
