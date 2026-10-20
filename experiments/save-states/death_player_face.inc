/* Direct death-role composition of the existing, unchanged typed face leaf.
 * No HBP or expired-bomb recipe, no fabricated lifetime receipt. Original
 * LPC, LPF and face SDK guards stay intact for their existing call paths. */
typedef struct {
 u32 magic,bytes,valid,token,frame,body_seal,face_seal,actor_hash;
 u32 buffer_hash[3],reserved;
} DfrRole;
typedef char DfrRoleSize[sizeof(DfrRole)==48?1:-1];
static DfrRole _gz_dfr_role;
static u32 _gz_dfr_seal,_gz_dfr_active,_gz_dfr_entered,_gz_dfr_dirty_seal;
u8 _gz_dfr_before_buffers[3][8192];
u8 _gz_dfr_applied_actor[0x8284],_gz_dfr_applied_buffers[3][8192];
/* Comparison-only after witnesses. They are never raw native replay images. */
u8 _gz_dfr_dirty_actor[0x8284],_gz_dfr_dirty_buffers[3][8192];
static u32 _gz_dfr_dirty_blink,_gz_dfr_dirty_valid;
LpfPlan _gz_dfr_witness_plan;
FaceReplayRun _gz_dfr_applied_run,_gz_dfr_rolled_run;
u32 _gz_dfr_report[32]={0x44465231u,128};
static u32 dfr_wanted(void){return _gz_dck_mode==6||_gz_dck_mode==7;}
static u32 dfr_sealed(void){const DfrRole*d=&_gz_dfr_role;const LpfPlan*p=&_gz_lpf_plan;u32 i;
 if(!dfr_wanted()||!_gz_dfr_active||d->magic!=0x44465031u||d->bytes!=sizeof(*d)||d->valid!=1||d->reserved||
  d->token!=_gz_dfr_active||d->token!=p->token||d->frame!=p->frame||d->frame!=_gz_pbac_frame||
  d->body_seal!=_gz_lpc_seal||d->face_seal!=_gz_lpf_seal||d->actor_hash!=_gz_dlr_role.actor_hash||
  _gz_dfr_seal!=btr_hash(d,sizeof(*d))||!lpf_sealed())return 0;
 for(i=0;i<3;i++)if(d->buffer_hash[i]!=btr_hash(_gz_dfr_before_buffers[i],8192))return 0;
 return 1;
}
static u32 dfr_binding(void){return dfr_sealed()?lpf_binding(0):28;}
static u32 dfr_dirty_hash(void){u32 h[5],i;
 h[0]=btr_hash(_gz_dfr_dirty_actor,sizeof(_gz_dfr_dirty_actor));
 for(i=0;i<3;i++)h[1+i]=btr_hash(_gz_dfr_dirty_buffers[i],8192);
 h[4]=_gz_dfr_dirty_blink;return btr_hash(h,sizeof(h));
}
static void dfr_dirty_capture(void){u32 i;const LpfPlan*p=&_gz_lpf_plan;
 _gz_dfr_dirty_valid=0;
 btr_copy(_gz_dfr_dirty_actor,(const void*)p->process.object,sizeof(_gz_dfr_dirty_actor));
 for(i=0;i<3;i++)btr_copy(_gz_dfr_dirty_buffers[i],(const void*)p->leaf.before_plans[i].buffer,8192);
 _gz_dfr_dirty_blink=_gz_face_native_blink;_gz_dfr_dirty_seal=dfr_dirty_hash();_gz_dfr_dirty_valid=1;
}
static u32 dfr_dirty_match(void){u32 i;const LpfPlan*p=&_gz_lpf_plan;
 if(_gz_dfr_dirty_valid!=1||_gz_dfr_dirty_seal!=dfr_dirty_hash()||
  !btr_equal(_gz_dfr_dirty_actor,(const void*)p->process.object,sizeof(_gz_dfr_dirty_actor))||
  _gz_face_native_blink!=_gz_dfr_dirty_blink)return 0;
 for(i=0;i<3;i++)if(!btr_equal(_gz_dfr_dirty_buffers[i],(const void*)p->leaf.before_plans[i].buffer,8192))return 0;
 return 1;
}
/* Called from the independent current-Link role query. No owner/source API
 * calls, so that query cannot recurse into its own admission checks. */
u32 _gz_dfr_current_match(void){u32 phase;
 if(!dfr_wanted())return !_gz_dfr_active;
 if(!_gz_dfr_active)return _gz_lpc_run.phase==LPC_PREPARED&&!_gz_dlr_role.reserved1&&_gz_roh_owner[3]==1&&_gz_roh_owner[12]==1;
 if(dfr_binding()||fr_current(&_gz_lpf_plan.leaf))return 0;
 phase=_gz_lpf_run.phase;
 if(phase==1)return !fr_before_matches(&_gz_lpf_plan.leaf,0)||!fr_before_matches(&_gz_lpf_plan.leaf,1);
 if(phase==3)return !fr_saved_matches(&_gz_lpf_plan.leaf);
 if(phase==5)return !fr_before_matches(&_gz_lpf_plan.leaf,0);
 /* A returned SDK failure gets an exact observed recovery-only role. No
  * arbitrary intermediate values are admitted while a native call runs. */
 return (phase==2||phase==4)&&!_gz_dfr_entered&&dfr_dirty_match();
}
u32 _gz_dfr_partial_owned(void){return dfr_wanted()&&_gz_dfr_active&&!_gz_dfr_entered&&
 (_gz_lpf_run.phase==2||_gz_lpf_run.phase==4)&&!dfr_binding()&&dfr_dirty_match();}
u32 _gz_dfr_role_seal(void){return dfr_wanted()&&_gz_dfr_active?_gz_dfr_seal:0;}
u32 _gz_dfr_prepare(u32 token,u32 frame){LpfPlan*p=&_gz_lpf_plan;DfrRole*d=&_gz_dfr_role;FaceSource source;
 u32 z,i,id,flags,data,wrappers[2];
 if(!dfr_wanted()||_gz_dfr_entered||_gz_dfr_active||_gz_lpf_entered||p->valid||_gz_lpf_run.phase||_gz_lpf_seal||
  !_gz_dck_body_context(token,frame)||!_gz_dck_body_gate(token,frame)||!dlr_sealed()||
  _gz_roh_gl_context(token,frame,1)!=1||_gz_lpc_plan.token!=token||_gz_lpc_run.phase!=LPC_PREPARED)return 28;
 _gz_dfr_entered=_gz_lpf_entered=1;dlr_zero(d,sizeof(*d));_gz_dfr_seal=0;_gz_dfr_dirty_valid=_gz_dfr_dirty_seal=0;
 for(i=2;i<32;i++)_gz_dfr_report[i]=0;
 p->magic=0x4c504631u;p->bytes=sizeof(*p);p->token=token;p->frame=frame;
 p->source=_gz_lpc_plan.source;p->used=_gz_lpc_plan.used;p->pin=_gz_lpc_plan.pin;p->body_plan_seal=_gz_lpc_seal;
 z=lpf_binding(1);if(z)goto done;
 if(!lpc_value(_gz_lpc_plan.saved,0x6980,&flags)){z=LPC_SOURCE;goto done;}
 z=_gz_face_source_normalize(_gz_lpc_plan.saved,flags,&source);if(z)goto done;
 z=fpc_observe(p->process.object);if(z)goto done;
 for(i=0;i<2;i++){
  z=fr_recipe(source.recipes+8*i,&id);if(z)goto done;
  z=fpc_wrapper(&p->arcs[0],id,!i,&wrappers[i],&data);if(z)goto done;
 }
 z=_gz_face_replay_prepare(&_gz_fpc_observed_bounds,wrappers,&source,&p->leaf,&_gz_lpf_run);if(z)goto done;
 z=lpf_binding(0);if(z)goto done;
 if(_gz_roh_gl_context(token,frame,1)!=1||!_gz_dck_body_gate(token,frame)||_gz_osa_validate(p->pin)||
  _gz_osa_pin_id()!=p->pin||_gz_osa_source(p->pin)!=p->source||_gz_osa_bytes(p->pin)!=p->used||
  !btr_equal((const void*)(p->source+R32(p->source,40)),_gz_lpc_plan.saved,8192)){z=42;goto done;}
 p->valid=1;_gz_lpf_seal=btr_hash(p,sizeof(*p));lpf_run_seal();
 d->magic=0x44465031u;d->bytes=sizeof(*d);d->valid=1;d->token=token;d->frame=frame;
 d->body_seal=_gz_lpc_seal;d->face_seal=_gz_lpf_seal;d->actor_hash=_gz_dlr_role.actor_hash;
 for(i=0;i<3;i++){
  btr_copy(_gz_dfr_before_buffers[i],(const void*)p->leaf.before_plans[i].buffer,8192);
  d->buffer_hash[i]=btr_hash(_gz_dfr_before_buffers[i],8192);
 }
 _gz_dfr_active=token;_gz_dfr_seal=btr_hash(d,sizeof(*d));
 _gz_dlr_role.reserved1=_gz_dfr_seal;_gz_dlr_seal=btr_hash(&_gz_dlr_role,sizeof(_gz_dlr_role));
 btr_copy(&_gz_dfr_witness_plan,p,sizeof(*p));
 _gz_dfr_report[2]=token;_gz_dfr_report[3]=frame;_gz_dfr_report[5]=_gz_dfr_seal;
 _gz_lpf_report[2]=token;_gz_lpf_report[3]=1;_gz_lpf_report[5]=frame;
 _gz_lpf_report[8]=p->process.object;_gz_lpf_report[9]=p->process.pid;
 z=0;
done:
 if(z){dlr_zero(p,sizeof(*p));dlr_zero(&_gz_lpf_run,sizeof(_gz_lpf_run));_gz_lpf_seal=_gz_lpf_run_seal=0;}
 _gz_lpf_report[4]=_gz_dfr_report[4]=z;_gz_dfr_entered=_gz_lpf_entered=0;return z;
}
u32 _gz_dfr_apply(u32 token,u32 frame){u32 z,i;
 if(!dfr_wanted())return 0;
 if(_gz_dfr_entered||_gz_lpf_entered||dfr_binding()||_gz_lpf_plan.token!=token||_gz_lpf_run.phase!=1||
  _gz_lpc_run.phase!=LPC_APPLIED||!_gz_dck_body_gate(token,frame)||_gz_roh_direct_body_common(token,frame,1))return 40;
 _gz_dfr_entered=_gz_lpf_entered=1;_gz_dfr_dirty_valid=0;
 z=_gz_face_replay_apply(&_gz_lpf_plan.leaf,&_gz_lpf_run,1);lpf_run_seal();
 dfr_dirty_capture();
 btr_copy(_gz_dfr_applied_actor,_gz_dfr_dirty_actor,sizeof(_gz_dfr_applied_actor));
 _gz_dfr_report[9]=btr_hash(_gz_dfr_applied_actor,sizeof(_gz_dfr_applied_actor));
 for(i=0;i<3;i++){btr_copy(_gz_dfr_applied_buffers[i],_gz_dfr_dirty_buffers[i],8192);_gz_dfr_report[10+i]=btr_hash(_gz_dfr_applied_buffers[i],8192);}
 _gz_dfr_report[13]=_gz_lpf_run_seal;_gz_dfr_report[14]=_gz_lpf_seal;
 _gz_dfr_entered=_gz_lpf_entered=0;
 if(!z)z=dfr_binding();if(!z&&(!_gz_dck_body_gate(token,frame)||_gz_roh_direct_body_common(token,frame,1)))z=40;
 btr_copy(&_gz_dfr_applied_run,&_gz_lpf_run,sizeof(_gz_lpf_run));
 _gz_lpf_report[3]=_gz_lpf_run.phase;_gz_lpf_report[4]=z;_gz_lpf_report[6]=frame;_gz_lpf_report[10]=_gz_lpf_run.native_calls;
 _gz_dfr_report[6]=z;_gz_dfr_report[7]=_gz_lpf_run.native_calls;return z;
}
u32 _gz_dfr_rollback(u32 token,u32 frame){u32 z;
 if(!dfr_wanted())return 0;
 if(_gz_dfr_entered||_gz_lpf_entered||dfr_binding()||_gz_lpf_plan.token!=token||_gz_lpf_run.phase==5||
  _gz_lpc_run.phase!=LPC_UNDOING||!_gz_dck_body_gate(token,frame)||_gz_roh_direct_recovery_common(token,frame))return 40;
 _gz_dfr_entered=_gz_lpf_entered=1;_gz_dfr_dirty_valid=0;
 z=_gz_face_replay_undo(&_gz_lpf_plan.leaf,&_gz_lpf_run);lpf_run_seal();dfr_dirty_capture();
 _gz_dfr_entered=_gz_lpf_entered=0;
 if(!z)z=dfr_binding();if(!z&&(!_gz_dck_body_gate(token,frame)||_gz_roh_direct_recovery_common(token,frame)))z=40;
 btr_copy(&_gz_dfr_rolled_run,&_gz_lpf_run,sizeof(_gz_lpf_run));
 _gz_lpf_report[3]=_gz_lpf_run.phase;_gz_lpf_report[4]=z;_gz_lpf_report[7]=frame;_gz_dfr_report[8]=z;_gz_dfr_report[15]=_gz_lpf_run_seal;return z;
}
u32 _gz_dfr_prepared_exact(void){u32 i;
 if(!dfr_wanted())return 1;
 if(!_gz_dfr_active)return !_gz_lpf_plan.valid&&!_gz_lpf_run.phase&&!_gz_lpf_seal&&!_gz_lpf_run_seal;
 if(dfr_binding()||_gz_lpf_run.phase!=1||_gz_lpf_run.native_calls||fr_before_matches(&_gz_lpf_plan.leaf,0))return 0;
 for(i=0;i<3;i++)if(!btr_equal(_gz_dfr_before_buffers[i],(const void*)_gz_lpf_plan.leaf.before_plans[i].buffer,8192))return 0;
 return 1;
}
u32 _gz_dfr_closure_key(u32 token,u32 frame){u32 i,w[6];
 if(!dfr_wanted())return 0;
 if(dfr_binding()||token!=_gz_dfr_active||frame!=_gz_dfr_role.frame||_gz_lpf_run.phase!=5||_gz_lpf_run.error||
  fr_before_matches(&_gz_lpf_plan.leaf,0))return 0;
 for(i=0;i<3;i++)if(!btr_equal(_gz_dfr_before_buffers[i],(const void*)_gz_lpf_plan.leaf.before_plans[i].buffer,8192))return 0;
 w[0]=0x44464331u;w[1]=token;w[2]=frame;w[3]=_gz_dfr_seal;w[4]=_gz_lpf_seal;w[5]=_gz_lpf_run_seal;
 return btr_hash(w,sizeof(w));
}
void _gz_dfr_retire_leaf(void){
 _gz_dfr_active=_gz_dfr_entered=0;
 dlr_zero(&_gz_lpf_plan,sizeof(_gz_lpf_plan));dlr_zero(&_gz_lpf_run,sizeof(_gz_lpf_run));
 _gz_lpf_seal=_gz_lpf_run_seal=_gz_lpf_entered=0;
}
