/* Read-only current-body witness under the real CLEAN/OBSERVE owner. This
 * exercises the same process, heap, resident-resource and numeric capture as
 * preparation, without requiring/reading a source record or granting replay.
 * The independent private witness is never the live LPC replay journal. */
static LpcPlan _gz_lpc_probe_plan;
static volatile u32 _gz_lpc_probe_seal;
static u32 _gz_lpc_probe_entered;
u32 _gz_lpc_probe_report[32]={0x4c505731u,128};
static void lpc_probe_zero(LpcPlan*p){u32 i;for(i=0;i<sizeof(*p)/4;i++)((volatile u32*)p)[i]=0;}
u32 _gz_lpc_probe_current(u32 token,u32 frame){LpcPlan*p=&_gz_lpc_plan;u32 z=28,i;
 if(_gz_lpc_probe_entered||_gz_lpc_entered||p->valid||_gz_lpc_run.phase||_gz_lpc_seal||
  !token||_gz_roh_gl_context(token,frame,1)!=1)return 28;
 _gz_lpc_probe_entered=1;_gz_lpc_entered=1;
 for(i=2;i<32;i++)_gz_lpc_probe_report[i]=0;
 _gz_lpc_probe_report[2]=token;_gz_lpc_probe_report[3]=frame;
 lpc_probe_zero(p);p->magic=0x4c504331u;p->bytes=sizeof(*p);p->version=2;p->token=token;p->frame=frame;
 z=lpc_binding(1);if(z)goto done;z=lpc_capture();if(z)goto done;
 p->before_hash=btr_hash(p->before,8192);z=lpc_binding(0);if(z)goto done;
 z=lpc_match(p->before,p->before_bind);if(z)goto done;
 if(_gz_roh_gl_context(token,frame,1)!=1){z=42;goto done;}
 p->valid=1;btr_copy(&_gz_lpc_probe_plan,p,sizeof(*p));_gz_lpc_probe_seal=btr_hash(&_gz_lpc_probe_plan,sizeof(*p));
 _gz_lpc_probe_report[5]=p->before_role;_gz_lpc_probe_report[6]=p->actor;_gz_lpc_probe_report[7]=p->pid;
 _gz_lpc_probe_report[8]=p->before_gameover_ID;_gz_lpc_probe_report[9]=p->before_demo;_gz_lpc_probe_report[10]=p->before_event;
 _gz_lpc_probe_report[11]=p->before_hash;_gz_lpc_probe_report[12]=_gz_lpc_probe_seal;
 _gz_lpc_probe_report[13]=p->archive.count;_gz_lpc_probe_report[14]=p->process.generation;
 for(i=0;i<5;i++)_gz_lpc_probe_report[16+i]=p->before_bind[i];
done:
 /* These private scratch objects were unpublished on entry. No native state, live
  * source or former prepared journal is cleared, rebound or discarded. */
 lpc_probe_zero(p);_gz_lpc_probe_report[4]=z;_gz_lpc_entered=_gz_lpc_probe_entered=0;return z;
}
