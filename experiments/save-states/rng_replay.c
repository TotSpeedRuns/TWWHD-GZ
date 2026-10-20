/* Three fixed native cM RNG seeds. Native provenance is the unchanged owned
 * cM_initRnd/cM_rnd instruction ranges. No saved/native pointer is accepted.
 * Constructors/retirement may consume RNG: rollback therefore restores the
 * original pre-attempt vector only AFTER all owned native deletion completes.
 * Final successful world publication must likewise publish saved RNG last. */
#define RGR_NATIVE 0x101ff9d4u
enum {RGR_PREPARED=1,RGR_APPLYING=2,RGR_APPLIED=3,RGR_ROLLED=5};
typedef struct {
 u32 magic,bytes,version,valid,phase,token,frame,source,used,pin,row,row_hash;
 u32 before[3],saved[3],applied_current[3],applied_frame,rolled_frame,error;
} RgrPlan;
typedef char RgrSize[sizeof(RgrPlan)==96?1:-1];
static RgrPlan _gz_rgr_plan;
static u32 _gz_rgr_seal,_gz_rgr_entered;
u32 _gz_rgr_report[32]={0x52475231u,128};
static void rgr_read(u32*p){u32 i;for(i=0;i<3;i++)p[i]=*(volatile u32*)(RGR_NATIVE+4*i);}
static void rgr_write(const u32*p){u32 i;for(i=0;i<3;i++)*(volatile u32*)(RGR_NATIVE+4*i)=p[i];}
static void rgr_seal(void){_gz_rgr_seal=btr_hash(&_gz_rgr_plan,sizeof(RgrPlan));}
static u32 rgr_sealed(void){const RgrPlan*p=&_gz_rgr_plan;
 return p->magic==0x52475231u&&p->bytes==sizeof(*p)&&p->version==1&&p->valid==1&&p->token&&
  p->phase>=1&&p->phase<=5&&p->phase!=4&&_gz_rgr_seal==btr_hash(p,sizeof(*p));
}
static void rgr_report(u32 z){const RgrPlan*p=&_gz_rgr_plan;u32 i;
 _gz_rgr_report[2]=p->token;_gz_rgr_report[3]=p->phase;_gz_rgr_report[4]=z;
 for(i=0;i<3;i++){_gz_rgr_report[5+i]=p->before[i];_gz_rgr_report[8+i]=p->saved[i];_gz_rgr_report[11+i]=p->applied_current[i];}
 _gz_rgr_report[20]=p->applied_frame;_gz_rgr_report[21]=p->rolled_frame;
}
static u32 rgr_apart(u32 a,u32 n,u32 b,u32 m){return a>=b?a-b>=m:b-a>=n;}
static u32 rgr_destinations(u32 source){u32 q[]={(u32)&_gz_rgr_seal,(u32)&_gz_rgr_entered};u32 i;
 if(!span(source,0x200000u)||!rgr_apart(source,0x200000u,RGR_NATIVE,12)||
  !rgr_apart(source,0x200000u,(u32)&_gz_rgr_plan,sizeof(RgrPlan))||
  !rgr_apart(source,0x200000u,(u32)_gz_rgr_report,sizeof(_gz_rgr_report)))return 0;
 for(i=0;i<2;i++)if(!rgr_apart(source,0x200000u,q[i],4))return 0;return 1;
}
static u32 rgr_rows(u32 source,u32 used,u32*row){u32 a,b;
 if(!span(source,used)||used<232||R32(source,8)!=27||R32(source,12)!=27)return 42;
 a=R32(source,56);b=R32(source,64);
 if(a<232||a>used||4>used-a||R32(source,60)!=4||b!=a+4||b>used||12>used-b||
  R32(source,68)!=12||R32(source+a,0)!=1)return 42;
 *row=source+b;return 0;
}
static u32 rgr_source(u32 token,u32 frame,u32 service){const RgrPlan*p=&_gz_rgr_plan;u32 row,z;
 if(!rgr_sealed()||p->token!=token)return 42;
 if(service){if(!_gz_hbp_service_read(token,frame))return 42;}
 else if(!_gz_hbp_read(token,frame))return 42;
 if(p->source!=_gz_hbp_recipe.source||p->used!=_gz_hbp_recipe.used||p->pin!=_gz_hbp_recipe.pin)return 42;
 z=rgr_rows(p->source,p->used,&row);if(z)return z;
 return row==p->row&&btr_hash((const void*)row,12)==p->row_hash&&btr_equal((const void*)row,p->saved,12)?0:42;
}
u32 _gz_rgr_prepare(u32 token,u32 frame){RgrPlan*p=&_gz_rgr_plan;u32 source,used,row,z,i;
 if(_gz_rgr_entered||p->valid||_gz_rgr_seal)return 28;
 if(!token||_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame))return 42;
 source=_gz_hbp_recipe.source;used=_gz_hbp_recipe.used;
 z=rgr_rows(source,used,&row);if(z)return z;if(!rgr_destinations(source))return 73;
 for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;
 p->magic=0x52475231u;p->bytes=sizeof(*p);p->version=1;p->valid=1;p->phase=RGR_PREPARED;
 p->token=token;p->frame=frame;p->source=source;p->used=used;p->pin=_gz_hbp_recipe.pin;p->row=row;
 p->row_hash=btr_hash((const void*)row,12);btr_copy(p->saved,(const void*)row,12);rgr_read(p->before);rgr_seal();
 z=rgr_source(token,frame,0);if(!z&&_gz_roh_gl_context(token,frame,1)!=1)z=42;
 rgr_report(z);if(z){for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;_gz_rgr_seal=0;}return z;
}
u32 _gz_rgr_apply(u32 token){RgrPlan*p=&_gz_rgr_plan;u32 frame=_gz_pbac_frame,z,readback[3],i;
 if(_gz_rgr_entered||!rgr_sealed()||p->token!=token||p->phase!=RGR_PREPARED)return 28;
 if(_gz_roh_owner[12]!=7||!_gz_bmh_scope(token,frame)||_gz_roh_replay_common(token,frame,0))return 49;
 z=rgr_source(token,frame,1);if(z)return z;
 _gz_rgr_entered=1;rgr_read(p->applied_current);p->phase=RGR_APPLYING;p->applied_frame=frame;rgr_seal();
 /* Fixed native scalar stores, no callbacks, SDK calls, yield or source read. */
 rgr_write(p->saved);rgr_read(readback);
 for(i=0;i<3;i++)_gz_rgr_report[14+i]=readback[i];
 z=btr_equal(readback,p->saved,12)?0:77;
 if(!z)z=rgr_source(token,frame,1);if(!z&&_gz_roh_replay_common(token,frame,0))z=42;
 if(!z)p->phase=RGR_APPLIED;p->error=z;rgr_seal();rgr_report(z);_gz_rgr_entered=0;return z;
}
u32 _gz_rgr_recovery_rollback(u32 token){RgrPlan*p=&_gz_rgr_plan;u32 frame=_gz_pbac_frame,z,readback[3],i;
 if(_gz_rgr_entered||!rgr_sealed()||p->token!=token||p->phase==RGR_ROLLED)return 28;
 if(!bor_original()||_gz_boh_recovery.phase!=BOR_RETIRING||_gz_boh_recovery.token!=token)return 40;
 z=_gz_roh_recovery_baseline(token,frame);if(z)return z;
 _gz_rgr_entered=1;
 /* Native creation and deletion may consume any number of random values.
  * The exact pre-attempt vector is restored last, independently of the source
  * and current vector, while original owner/context remain held and sealed. */
 rgr_write(p->before);rgr_read(readback);
 for(i=0;i<3;i++)_gz_rgr_report[17+i]=readback[i];
 z=btr_equal(readback,p->before,12)?0:77;
 if(!z)z=_gz_roh_recovery_common(token,frame);
 if(!z){p->phase=RGR_ROLLED;p->rolled_frame=frame;p->error=0;}else p->error=z;
 rgr_seal();rgr_report(z);_gz_rgr_entered=0;return z;
}
u32 _gz_rgr_recovery_check(u32 token,u32 frame){const RgrPlan*p=&_gz_rgr_plan;u32 values[3],z;
 if(_gz_rgr_entered||!rgr_sealed()||p->token!=token||p->phase!=RGR_ROLLED||p->error||
  !bor_original()||_gz_boh_recovery.phase!=BOR_DONE)return 77;
 z=_gz_roh_recovery_common(token,frame);if(z)return z;rgr_read(values);
 if(!btr_equal(values,p->before,12))return 77;return _gz_roh_recovery_common(token,frame);
}
