/* Typed camera participant for the real held operation. Only audited scalar,
 * work, actor-link, View, angle and CameraInfo fields are written. Camera
 * objects/vtables, BG state, native parameter caches and UI state are excluded.
 * Style/algorithm transitions need a separate native cache participant.
 *
 * Prepare runs in CLEAN/OBSERVE before constructor mutation. Apply requires
 * the sealed real service5 handoff. Undo has its own source-free FAILED_DIRTY
 * ticket and runs before the owned replacement bomb can be retired. */
#include "process.h"
extern u32 _gz_roh_gl_context(u32,u32,u32);
extern volatile u8 _gz_roh_live_camera_index;
extern volatile u32 _gz_roh_live_camera_table[];
extern const u32 _gz_cct_native_styles[];
extern volatile u8 _gz_cct_native_info[];
extern u32 _gz_camera_read(void*),_gz_camera_tower_safe(const void*);

typedef struct {u16 offset,bytes;} CctField;
static const CctField cct_fields[]={
 {4,96},{100,24},{124,8},{132,96},{228,20},{256,40},{304,4},{312,18},
 {332,104},{492,62},{556,52},{792,1},{796,6},{804,28},{852,40},{1296,28},
 {1328,1},{1332,20},{1352,15},{1368,8},{1376,42},{1420,52},{1476,44},
 {1528,28},{1840,9},{2224,48},{0,0}};
static const CctField cct_work1[]={{0,99},{100,24},{0,0}};
static const CctField cct_work2[]={{0,4},{8,13},{24,14},{40,28},{0,0}};
static const CctField cct_work4[]={{0,6},{8,20},{32,12},{48,4},{64,6},{72,8},{0,0}};
static const CctField cct_work7[]={{0,42},{44,48},{92,2},{96,24},{0,0}};
static const CctField cct_work10[]={{4,32},{40,1},{44,24},{0,0}};
static const u16 cct_link_fields[]={0x128,0x12c,0x134,0x340,0x52c};
enum {CCT_PREPARED=1,CCT_APPLYING=2,CCT_APPLIED=3,CCT_UNDOING=4,CCT_ROLLED=5};
typedef struct {
 u32 magic,bytes,version,valid,phase,token,frame,applied_frame,rolled_frame;
 u32 camera,body,pid,index,style,algorithm,link,link_pid,source,source_used,pin,source_hash,before_hash;
 u32 before_force,after_force,before_links[5],after_links[5];
 PoEntry process;
 u8 param[12],before[2048],saved[2048];
} CctJournal;
typedef char CctSize[sizeof(CctJournal)==4308?1:-1];
static CctJournal _gz_cct_journal;
static u32 _gz_cct_seal,_gz_cct_entered;
u32 _gz_cct_report[64]={0x43435431u,256};
static void cct_seal(void){_gz_cct_seal=btr_hash(&_gz_cct_journal,sizeof(CctJournal));}
static u32 cct_sealed(void){const CctJournal*j=&_gz_cct_journal;
 return j->magic==0x43435431u&&j->bytes==sizeof(*j)&&j->version==1&&j->valid==1&&
  j->phase>=CCT_PREPARED&&j->phase<=CCT_ROLLED&&j->token&&j->index<2&&
  j->body==j->camera+0x248&&j->style<=144&&
  _gz_cct_seal==btr_hash(j,sizeof(*j))&&j->source_hash==btr_hash(j->saved,2048)&&
  j->before_hash==btr_hash(j->before,2048);
}
static const CctField*cct_work(u32 a){
 if(a==1)return cct_work1;if(a==2)return cct_work2;if(a==4)return cct_work4;
 if(a==7)return cct_work7;if(a==10)return cct_work10;return 0;
}
/* The native camera table is rediscovered before the observed process extent
 * is consulted. A stored camera address is only a comparison until then.
 * The process ledger adds generation/complete allocation extent to the old
 * owner's current-table, Link and camera-self bindings; it grants no SDK API. */
static u32 cct_current(PoEntry*out,u32*camera,u32*index){u32 i,c;const PoEntry*p;const volatile u32*o=_gz_roh_owner;
 i=_gz_roh_live_camera_index;
 if(i>1||i!=o[36]||_gz_roh_live_camera_table[i*13]!=o[37])return 45;
 c=_gz_roh_live_camera_table[i*13];
 if(!span(c,0xb28)||_gz_po_lookup(c)||!_gz_po_observation.valid||_gz_po_observation.flags)return 45;
 p=&_gz_po_observation.entry;
 if(p->object!=c||p->state!=PO_LIVE||!p->generation||p->flags||p->pid>=0xfffffffeu||
  p->profile>=484||p->end_frame||p->allocation_bytes<0xb28||p->allocation_bytes>0x100000u||
  p->base_bytes+p->other_bytes<p->base_bytes||p->base_bytes+p->other_bytes!=p->allocation_bytes)return 45;
 if(R32(c,4)!=p->pid||R16(c,8)!=p->process_name||R16(c,14)!=p->profile||R32(c,0x10)!=p->profile_entry||
  R32(c,0x248)!=c||R8(c,0x24c)!=1||R8(c,0x24d)||R32(c,0x368)!=i||
  R32(c,0x370)!=o[29]||o[38]!=c||o[39]!=o[29]||o[40]!=i)return 45;
 btr_copy(out,p,sizeof(*out));*camera=c;*index=i;return 0;
}
static u32 cct_binding(void){const CctJournal*j=&_gz_cct_journal;PoEntry p;u32 c,i,z;
 z=cct_current(&p,&c,&i);if(z)return z;
 if(c!=j->camera||i!=j->index||!btr_equal(&p,&j->process,sizeof(p))||
  R32(c+0x248,0x514)!=j->style||_gz_cct_native_styles[j->style*33]!=j->algorithm||
  !btr_equal((const void*)(j->body+0x8a4),j->param,12))return 45;
 return 0;
}
static u32 cct_record(const u8*r,const CctJournal*j){u32 i,v;
 if(R32(r,0)!=1||R32(r,8)!=j->camera||R32(r,12)!=j->pid||R32(r,16)!=j->index||
  R32(r,20)!=j->style||R32(r,24)!=j->algorithm||!cct_work(j->algorithm)||
  _gz_camera_tower_safe(r))return 45;
 for(i=0;i<13;i++)if(!finite(R32(r,1400+i*4)))return 45;
 v=R32(r,1408);if(v<0x3f800000u||v>0x43340000u)return 45;
 if(R32(r,1536)!=0x55495331u||R32(r,1540)>1||R32(r,1544)>2)return 45;
 return 0;
}
/* Only fresh current queue membership supplies an actor pointer. Saved IDs
 * and before-link numeric audits are never followed as allocations. */
static u32 cct_resolve(u32 pid,u32*out){u32 q=_gz_pba_native_actor_queue,left=4096,a,found=0;
 *out=0;if(pid==BAD)return 0;
 while(q){if(!left--||!span(q,16))return 27;a=R32(q,12);
  if(a){if(!span(a,12))return 27;if(R32(a,4)==pid){if(found)return 27;found=a;}}
  q=R32(q,8);
 }
 if(!found)return 27;*out=found;return 0;
}
static u32 cct_before_links(void){const CctJournal*j=&_gz_cct_journal;u32 i,a,z;
 for(i=0;i<5;i++){z=cct_resolve(R32(j->before,32+i*4),&a);if(z||a!=j->before_links[i])return 27;}
 return j->before_links[0]==j->link?0:45;
}
/* Pure typed byte comparison, only after current allocation/owner checks.
 * Preserve native active/pause bytes; UI restoration is a distinct service. */
static u32 cct_match(const u8*r,const u32*links,u32 force){const CctJournal*j=&_gz_cct_journal;u32 k,p=128,o,n;
 const CctField*w=cct_work(j->algorithm);if(!w)return 45;
 if(R8(j->body,4)!=j->before[128]||R8(j->body,5)!=j->before[129])return 45;
 for(k=0;cct_fields[k].bytes;k++){
  o=cct_fields[k].offset;n=cct_fields[k].bytes;
  if(o==4){if(!btr_equal((const void*)(j->body+6),r+p+2,n-2))return 45;}
  else if(o==0x130){if(R32(j->body,o)!=force)return 45;}
  else if(!btr_equal((const void*)(j->body+o),r+p,n))return 45;
  p+=n;
 }
 p=1200;for(k=0;w[k].bytes;k++){if(!btr_equal((const void*)(j->body+0x37c+w[k].offset),r+p,w[k].bytes))return 45;p+=w[k].bytes;}
 for(k=0;k<5;k++)if(R32(j->body,cct_link_fields[k])!=links[k])return 45;
 if(!btr_equal((const void*)(j->camera+0xcc),r+1400,56)||
  !btr_equal((const void*)(j->camera+0x234),r+1460,6)||
  !btr_equal((const void*)((u32)_gz_cct_native_info+j->index*52),r+1472,12))return 45;
 return 0;
}
/* No branch/native call capable of failing remains inside this fixed copy
 * kernel. The complete preimage is sealed before its first native store. */
static void cct_write(const u8*r,const u32*links,u32 force){const CctJournal*j=&_gz_cct_journal;u32 k,p=128,o,n;
 const CctField*w=cct_work(j->algorithm);
 for(k=0;cct_fields[k].bytes;k++){
  o=cct_fields[k].offset;n=cct_fields[k].bytes;
  if(o==4)btr_copy((void*)(j->body+6),r+p+2,n-2);
  else if(o==0x130)*(volatile u32*)(j->body+o)=force;
  else btr_copy((void*)(j->body+o),r+p,n);
  p+=n;
 }
 p=1200;for(k=0;w[k].bytes;k++){btr_copy((void*)(j->body+0x37c+w[k].offset),r+p,w[k].bytes);p+=w[k].bytes;}
 for(k=0;k<5;k++)*(volatile u32*)(j->body+cct_link_fields[k])=links[k];
 btr_copy((void*)(j->camera+0xcc),r+1400,56);
 btr_copy((void*)(j->camera+0x234),r+1460,6);
 btr_copy((void*)((u32)_gz_cct_native_info+j->index*52),r+1472,12);
}
u32 _gz_cct_prepare(u32 token,u32 frame){CctJournal*j=&_gz_cct_journal;PoEntry p;u32 c,index,z,i,s,off,n;
 if(_gz_cct_entered||j->phase||j->valid||_gz_cct_seal)return 28;
 if(!token||_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame))return 42;
 _gz_cct_entered=1;z=cct_current(&p,&c,&index);if(z)goto done;
 s=_gz_hbp_recipe.source;n=_gz_hbp_recipe.used;off=R32(s,48);
 if(R32(s,52)!=2048||off<216||(off&3)||off>n||2048>n-off){z=42;goto done;}
 j->phase=CCT_PREPARED;j->magic=0x43435431u;j->bytes=sizeof(*j);j->version=1;
 j->token=token;j->frame=frame;j->camera=c;j->body=c+0x248;j->pid=p.pid;j->index=index;
 j->style=R32(j->body,0x514);if(j->style>144){z=45;goto done;}
 j->algorithm=_gz_cct_native_styles[j->style*33];j->link=_gz_roh_owner[29];j->link_pid=_gz_roh_owner[30];
 j->source=s;j->source_used=n;j->pin=_gz_hbp_recipe.pin;btr_copy(&j->process,&p,sizeof(p));
 btr_copy(j->param,(const void*)(j->body+0x8a4),12);
 for(i=0;i<2048;i++)j->before[i]=0;
 z=_gz_camera_read(j->before);if(z)goto done;
 btr_copy(j->saved,(const void*)(s+off),2048);
 z=cct_record(j->before,j);if(z)goto done;z=cct_record(j->saved,j);if(z)goto done;
 for(i=0;i<5;i++)j->before_links[i]=R32(j->body,cct_link_fields[i]);
 j->before_force=R32(j->body,0x130);j->after_force=R32(j->saved,412);
 j->source_hash=btr_hash(j->saved,2048);j->before_hash=btr_hash(j->before,2048);
 z=cct_before_links();if(z)goto done;
 z=cct_binding();if(z)goto done;z=cct_match(j->before,j->before_links,j->before_force);if(z)goto done;
 if(_gz_roh_gl_context(token,frame,1)!=1||!_gz_hbp_read(token,frame)||
  !btr_equal(j->saved,(const void*)(s+off),2048)){z=42;goto done;}
 j->valid=1;cct_seal();
 _gz_cct_report[2]=token;_gz_cct_report[3]=CCT_PREPARED;_gz_cct_report[5]=frame;
 _gz_cct_report[8]=c;_gz_cct_report[9]=p.pid;_gz_cct_report[10]=p.generation;
 _gz_cct_report[11]=j->style;_gz_cct_report[12]=j->algorithm;
 _gz_cct_report[13]=j->source_hash;_gz_cct_report[14]=j->before_hash;
 _gz_cct_report[20]=j->pin;_gz_cct_report[21]=s;
done:
 _gz_cct_report[4]=z;_gz_cct_entered=0;return z;
}
static u32 cct_apply_gate(u32 token,u32 frame){const CctJournal*j=&_gz_cct_journal;u32 p;
 if(!cct_sealed()||j->token!=token||_gz_roh_owner[12]!=5||!_gz_bmh_scope(token,frame)||
  _gz_roh_replay_common(token,frame,1)||_gz_boh_service_validate(token)||!_gz_hbp_service_read(token,frame))return 42;
 if(j->source!=_gz_hbp_recipe.source||j->source_used!=_gz_hbp_recipe.used||j->pin!=_gz_hbp_recipe.pin||
  j->source_hash!=btr_hash(j->saved,2048))return 42;
 p=j->source+R32(j->source,48);
 if(!btr_equal((const void*)p,j->saved,2048))return 42;
 return cct_binding();
}
u32 _gz_cct_apply(u32 token){CctJournal*j=&_gz_cct_journal;u32 z,i,p,links[5],force,frame=_gz_pbac_frame;
 if(_gz_cct_entered||!cct_sealed()||j->phase!=CCT_PREPARED||j->token!=token)return 28;
 _gz_cct_entered=1;z=cct_apply_gate(token,frame);if(z)goto done;
 z=cct_before_links();if(z)goto done;z=cct_match(j->before,j->before_links,j->before_force);if(z)goto done;
 for(i=0;i<5;i++){
  p=R32(j->saved,32+i*4);if(p==_gz_boh_plan.source_pid)p=_gz_boh_journal.pid;
  z=cct_resolve(p,&links[i]);if(z)goto done;
 }
 if(links[0]!=j->link){z=45;goto done;}
 force=j->after_force;if(force==_gz_boh_plan.source_pid)force=_gz_boh_journal.pid;
 for(i=0;i<5;i++)j->after_links[i]=links[i];j->after_force=force;
 j->applied_frame=frame;j->phase=CCT_APPLYING;cct_seal();
 cct_write(j->saved,j->after_links,j->after_force);
 z=cct_match(j->saved,j->after_links,j->after_force);if(z)goto done;
 z=cct_apply_gate(token,frame);if(z)goto done;
 j->phase=CCT_APPLIED;cct_seal();_gz_cct_report[3]=CCT_APPLIED;
 _gz_cct_report[6]=frame;_gz_cct_report[18]=1;
 _gz_cct_report[23]=j->before_force;_gz_cct_report[24]=j->after_force;
done:
 _gz_cct_report[4]=z;_gz_cct_entered=0;return z;
}
/* No source allocation, pin API, saved actor pointer or normal service/deadline
 * is consulted here. The independent recovery ticket certifies the fresh
 * current camera/Link and exact completed-constructor native queue delta. */
u32 _gz_cct_recovery_rollback(u32 token){CctJournal*j=&_gz_cct_journal;u32 z,frame=_gz_pbac_frame;
 if(_gz_cct_entered||!cct_sealed()||j->token!=token||j->phase==CCT_ROLLED||j->phase==CCT_UNDOING)return 28;
 _gz_cct_entered=1;
 if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,j->link,frame)){z=40;goto done;}
 z=cct_binding();if(z)goto done;z=cct_before_links();if(z)goto done;
 if(j->phase==CCT_PREPARED){z=cct_match(j->before,j->before_links,j->before_force);if(z)goto done;}
 else{
  j->phase=CCT_UNDOING;cct_seal();
  cct_write(j->before,j->before_links,j->before_force);
  z=cct_match(j->before,j->before_links,j->before_force);if(z)goto done;
 }
 z=cct_binding();if(z)goto done;
 if(!_gz_boh_recovery_authority(token,_gz_boh_journal.actor,j->link,frame)){z=40;goto done;}
 j->rolled_frame=frame;j->phase=CCT_ROLLED;cct_seal();_gz_cct_report[3]=CCT_ROLLED;
 _gz_cct_report[7]=frame;_gz_cct_report[19]=1;
done:
 _gz_cct_report[4]=z;_gz_cct_entered=0;return z;
}
