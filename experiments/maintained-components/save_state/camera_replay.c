/* Publish UI intent only after a completed successful world commit. */
extern void _gz_snapshot_ui_arm(void);
/* Typed camera participant for the real held operation. Only audited scalar,
 * work, actor-link, View, angle and CameraInfo fields are written. Camera
 * objects/vtables, BG state, native parameter caches and UI state are excluded.
 * Style/algorithm transitions use the attested native dCamParam::Change.
 * Its exact two-word cache footprint is reversed by the same native API;
 * private before-only work bytes close the union of both algorithm maps.
 *
 * Prepare runs in CLEAN/OBSERVE before constructor mutation. Apply requires
 * the sealed real service5 handoff. Undo has its own source-free FAILED_DIRTY
 * ticket and runs before the owned replacement bomb can be retired. */
#include "process.h"
#include "camera_process_identity.c"
extern u32 _gz_roh_gl_context(u32,u32,u32);
extern volatile u8 _gz_roh_live_camera_index;
extern volatile u32 _gz_roh_live_camera_table[];
extern const u32 _gz_cct_native_styles[];
extern volatile u8 _gz_cct_native_info[];
extern u32 _gz_dck_native_change(u32,int);
#include "camera_native_cache.h"
static u32 cct_cache_code(void){return _gz_native_camera_change_code_valid();}
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
 u32 retired_mask;PoEntry retired_bomb;
 u32 source_link_pid,source_camera_relation;
} CctJournal;
typedef char CctSize[sizeof(CctJournal)==4384?1:-1];
static CctJournal _gz_cct_journal;
static u32 _gz_cct_seal,_gz_cct_entered;
u32 _gz_cct_report[64]={0x43435431u,256};
/* Pure numeric tombstone validation: never dereference the ended object.
 * This predicate grants neither replay nor constructor authority. */
u32 _gz_cct_retired_record_ok(const PoEntry*e,u32 raw,u32 pid,u32 frame){
 return e&&raw&&pid&&pid<0xfffffffeu&&e->object==raw&&e->pid==pid&&
  e->state==PO_ENDED&&e->generation&&e->process_name==294&&e->profile==294&&
  !e->flags&&e->end_frame&&frame-e->end_frame<0x80000000u&&
  e->base_bytes+e->other_bytes>=e->base_bytes&&e->allocation_bytes>=0xb50&&
  e->allocation_bytes<=0x100000u&&e->base_bytes+e->other_bytes==e->allocation_bytes;
}
static void cct_seal(void){_gz_cct_seal=btr_hash(&_gz_cct_journal,sizeof(CctJournal));}
static u32 cct_sealed(void){const CctJournal*j=&_gz_cct_journal;
 return j->magic==0x43435431u&&j->bytes==sizeof(*j)&&j->version==1&&j->valid==1&&
  j->phase>=CCT_PREPARED&&j->phase<=CCT_ROLLED&&j->token&&j->index<2&&
  j->body==j->camera+0x248&&j->style<=144&&j->source_link_pid!=BAD&&
  (j->source_camera_relation==1||j->source_camera_relation==2)&&
  (!j->retired_mask||(j->retired_mask==4&&
   _gz_cct_retired_record_ok(&j->retired_bomb,j->before_links[2],R32(j->before,40),j->frame)))&&
  _gz_cct_seal==btr_hash(j,sizeof(*j))&&j->source_hash==btr_hash(j->saved,2048)&&
  j->before_hash==btr_hash(j->before,2048);
}

/* Called by the existing camera reader, before publishing its valid marker.
 * Only authored private record buffers are writable. No native object is
 * changed. The official camera table/self/Link bindings and observed complete
 * LIVE process extent are reacquired; pointer equality alone is insufficient. */
extern u8 _gz_camera_working[],_gz_camera_observed[];
u32 _gz_cct_capture_identity(u8*r){CpiPacket*p=(CpiPacket*)(r+CPI_OFFSET);PoHeader ph;PoEntry e;
 u32 c,index,link;
 if(r!=_gz_camera_working&&r!=_gz_camera_observed&&r!=_gz_cct_journal.before)return 45;
 p->magic=0;c=R32(r,8);index=R32(r,16);link=_gz_pbac_player;
 if(index>1||index!=_gz_roh_live_camera_index||_gz_roh_live_camera_table[index*13]!=c||
  !span(c,0xb28)||!span(link,0x300)||R32(r,12)!=R32(c,4)||R32(r,32)!=R32(link,4)||
  R32(c,0x248)!=c||R32(c,0x368)!=index||R32(c,0x370)!=link)return 45;
 btr_copy(&ph,(const void*)&_gz_po_header,sizeof(ph));
 if(ph.magic!=PO_MAGIC||ph.bytes!=sizeof(ph)||ph.version!=1||ph.capacity!=PO_CAP||
  ph.rows!=(u32)_gz_po_rows||ph.flags||ph.busy||!ph.serial||_gz_po_lookup(c)||
  !_gz_po_observation.valid||_gz_po_observation.flags||_gz_po_observation.session!=ph.session)return 45;
 btr_copy(&e,&_gz_po_observation.entry,sizeof(e));
 if(e.object!=c||e.pid!=R32(c,4)||e.process_name!=R16(c,8)||e.profile!=R16(c,14)||
  e.profile_entry!=R32(c,16)||e.profile_entry!=_gz_po_profiles[476]||!span(c,e.allocation_bytes))return 45;
 p->magic=CPI_MAGIC;p->bytes=CPI_BYTES;p->version=1;p->reserved=0;
 btr_copy(&p->process,&e,sizeof(e));p->session=ph.session;p->serial=ph.serial;p->frame=_gz_pbac_frame;p->checksum=cpi_hash(p);
 if(_gz_cpi_validate(p,c,R32(r,12),_gz_pbac_frame)||!btr_equal(&ph,(const void*)&_gz_po_header,sizeof(ph))||
  _gz_roh_live_camera_index!=index||_gz_roh_live_camera_table[index*13]!=c||_gz_pbac_player!=link)return 45;
 return 0;
}

extern const CctField _gz_camera_work_fields_15[];
static const CctField*cct_work(u32 a){
 if(a==15)return _gz_camera_work_fields_15;
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
 if(c!=j->camera||i!=j->index||!btr_equal(&p,&j->process,sizeof(p)))return 45;
 {u32 style=R32(j->body,0x514),cache=R32(j->body,0x8ac),saved=R32(j->saved,20);
  if(style>144||cache>144||(style!=j->style&&style!=saved)||(cache!=j->style&&cache!=saved)||
   R32(j->body,0x8a4)!=R32(j->param,0)||R32(j->body,0x8a8)!=camera_native_cache_base()+cache*132||
   _gz_cct_native_styles[j->style*33]!=j->algorithm||!cct_cache_code())return 45;
  if((j->phase==CCT_PREPARED||j->phase==CCT_ROLLED)&&(style!=j->style||cache!=j->style))return 45;
  if(j->phase==CCT_APPLIED&&(style!=saved||cache!=saved))return 45;
 }
 return 0;
}
static u32 cct_record_values(const u8*r,u32 index){u32 i,v;
 if(R32(r,0)!=1||R32(r,16)!=index||
  R32(r,20)>144||R32(r,24)!=_gz_cct_native_styles[R32(r,20)*33]||!cct_work(R32(r,24))||
  _gz_camera_tower_safe(r))return 45;
 for(i=0;i<13;i++)if(!finite(R32(r,1400+i*4)))return 45;
 v=R32(r,1408);if(v<0x3f800000u||v>0x43340000u)return 45;
 if(R32(r,1536)!=0x55495331u||R32(r,1540)>1||R32(r,1544)>2)return 45;
 return 0;
}
static u32 cct_record(const u8*r,const CctJournal*j){
 if(R32(r,8)!=j->camera||R32(r,12)!=j->pid)return 45;
 return cct_record_values(r,j->index);
}
/* Source identity is authenticated under the actual source pin. Destination
 * identity remains the independently rooted LIVE camera from cct_current.
 * CPI1 never makes a saved address a readable/current native allocation. */
static u32 cct_source_role(CctJournal*j){const VrWorld*w;const CpiPacket*p;u32 s=j->source,n=j->source_used,o,z;
 z=_gz_bcb_archive_validate(s,n,1);if(z)return z;
 o=R32(s,168);w=(const VrWorld*)(s+o);
 if(w->stage0!=_gz_roh_owner[41]||w->stage1!=_gz_roh_owner[42]||w->room!=_gz_roh_owner[43]||w->layer!=_gz_roh_owner[44]||
  R32(j->saved,32)!=w->link_pid||!j->link_pid)return 45;
 z=cct_record_values(j->saved,j->index);if(z)return z;
 j->source_link_pid=w->link_pid;j->source_camera_relation=0;
 p=(const CpiPacket*)(j->saved+CPI_OFFSET);
 if(!p->magic){
  /* Compatibility cannot invent missing native generation evidence. */
  if(w->epoch!=_gz_pbac_epoch||w->link_pid!=j->link_pid||w->link_actor!=j->link)return 45;
  z=cct_record(j->saved,j);if(z)return z;j->source_camera_relation=1;return 0;
 }
 z=_gz_cpi_validate(p,R32(j->saved,8),R32(j->saved,12),w->frame);if(z)return z;
 if(_gz_po_header.flags||_gz_po_header.busy||p->session!=_gz_po_header.session||p->serial>_gz_po_header.serial||
  j->process.generation>_gz_po_header.serial||j->process.profile_entry!=_gz_po_profiles[476])return 45;
 z=_gz_cpi_relation(p,&j->process,j->frame,w->epoch,_gz_pbac_epoch);if(!z)return 45;
 j->source_camera_relation=z;
 _gz_cct_report[57]=z;_gz_cct_report[58]=p->process.pid;_gz_cct_report[59]=j->pid;
 _gz_cct_report[60]=p->process.generation;_gz_cct_report[61]=j->process.generation;
 _gz_cct_report[62]=w->epoch;_gz_cct_report[63]=_gz_pbac_epoch;
 return 0;
}

/* Called only when the current queue traversal naturally ends without
 * finding a non-null weak lock-on actor. Ordinary capture never receives this
 * exception. The current weak bomb reference, observed ENDED generation and real source
 * lease must agree before the before-image can retain an opaque weak value. */
u32 _gz_cct_read_retired_bomb(u32 raw,u32 index,u32 destination){
 CctJournal*j=&_gz_cct_journal;const PoEntry*e;u32 pid;
 if(index!=2||destination!=(u32)j->before||_gz_cct_entered!=1||j->valid||j->retired_mask||
  j->phase!=CCT_PREPARED||!j->token||j->frame!=_gz_pbac_frame||
  _gz_roh_gl_context(j->token,j->frame,1)!=1||
  !tx_observe_matches(j->token,j->frame,j->source,j->source_used,j->pin)||
  !boh_sealed()||_gz_boh_plan.token!=j->token||_gz_boh_plan.role!=BOH_CREATED||
  _gz_boh_journal.phase!=BOH_PREPARED||R32(j->body,cct_link_fields[2])!=raw)return BAD;
 if(_gz_po_lookup(raw)||!_gz_po_observation.valid||_gz_po_observation.flags)return BAD;
 e=&_gz_po_observation.entry;pid=e->pid;
 if(!_gz_cct_retired_record_ok(e,raw,pid,j->frame)||e->profile_entry!=_gz_po_profiles[294])return BAD;
 btr_copy(&j->retired_bomb,e,sizeof(*e));j->retired_mask=4;
 _gz_cct_report[52]=index;_gz_cct_report[53]=pid;_gz_cct_report[54]=e->generation;
 _gz_cct_report[55]=raw;_gz_cct_report[56]=j->frame;
 return pid;
}
/* The old address may be reused by this operation's native bomb constructor.
 * Compare current observer records without following the retired pointer.
 * No unrelated actor or generation can satisfy this before-image binding. */
static u32 cct_retired_binding(void){const CctJournal*j=&_gz_cct_journal;const PoEntry*e;u32 a=j->before_links[2];
 if(j->retired_mask!=4||R32(j->body,cct_link_fields[2])!=a||_gz_po_lookup(a)||
  !_gz_po_observation.valid||_gz_po_observation.flags)return 0;
 e=&_gz_po_observation.entry;
 if(btr_equal(e,&j->retired_bomb,sizeof(*e)))return 1;
 return boh_sealed()&&_gz_boh_plan.token==j->token&&_gz_boh_journal.token==j->token&&
  _gz_boh_plan.role==BOH_CREATED&&_gz_boh_journal.actor==a&&
  _gz_boh_journal.pid&&_gz_boh_journal.pid!=j->retired_bomb.pid&&
  e->object==a&&e->pid==_gz_boh_journal.pid&&e->generation>j->retired_bomb.generation&&
  (e->state==PO_LIVE||e->state==PO_ENDED)&&!e->flags&&e->profile==294&&e->process_name==294&&
  e->profile_entry==j->retired_bomb.profile_entry&&e->allocation_bytes>=0xb50&&
  _gz_roh_owner[4]==1&&_gz_roh_owner[5]==j->token&&_gz_roh_owner[6]==1&&
  (_gz_roh_owner[3]==ROH_MUTATING||_gz_roh_owner[3]==ROH_FAILED_DIRTY);
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
 for(i=0;i<5;i++){
  if(i==2&&j->retired_mask==4){if(!cct_retired_binding())return 27;continue;}
  z=cct_resolve(R32(j->before,32+i*4),&a);if(z||a!=j->before_links[i])return 27;
 }
 return j->before_links[0]==j->link?0:45;
}
/* Pure typed byte comparison, only after current allocation/owner checks.
 * Preserve native active/pause bytes; UI restoration is a distinct service. */
static u32 cct_match(const u8*r,const u32*links,u32 force){const CctJournal*j=&_gz_cct_journal;u32 k,p=128,o,n;
 const CctField*w=cct_work(R32(r,24));if(!w)return 45;
 if(R32(j->body,0x514)!=R32(r,20)||R32(j->body,0x8ac)!=R32(r,20)||R32(j->body,0x8a8)!=camera_native_cache_base()+R32(r,20)*132)return 45;
 if(r==j->before&&!btr_equal((const void*)(j->body+0x37c),j->before+1600,128))return 45;
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
 const CctField*w=cct_work(R32(r,24));
 for(k=0;cct_fields[k].bytes;k++){
  o=cct_fields[k].offset;n=cct_fields[k].bytes;
  if(o==4)btr_copy((void*)(j->body+6),r+p+2,n-2);
  else if(o==0x130)*(volatile u32*)(j->body+o)=force;
  else btr_copy((void*)(j->body+o),r+p,n);
  p+=n;
 }
 p=1200;for(k=0;w[k].bytes;k++){btr_copy((void*)(j->body+0x37c+w[k].offset),r+p,w[k].bytes);p+=w[k].bytes;}
 /* Raw bytes here are ONLY this current allocation's sealed before-image,
  * never a saved-state pointer/work dump. This also reverses numeric bytes
  * used by the target algorithm but absent from the original algorithm map. */
 if(r==j->before)btr_copy((void*)(j->body+0x37c),j->before+1600,128);
 for(k=0;k<5;k++)*(volatile u32*)(j->body+cct_link_fields[k])=links[k];
 btr_copy((void*)(j->camera+0xcc),r+1400,56);
 btr_copy((void*)(j->camera+0x234),r+1460,6);
 btr_copy((void*)((u32)_gz_cct_native_info+j->index*52),r+1472,12);
}
u32 _gz_cct_prepare(u32 token,u32 frame){CctJournal*j=&_gz_cct_journal;PoEntry p;u32 c,index,z,i,s,off,n;const TxObserveSource*v;
 if(_gz_cct_entered||j->phase||j->valid||_gz_cct_seal)return 28;
 if(!token||_gz_roh_gl_context(token,frame,1)!=1||!(v=tx_observe_source(token,frame)))return 42;
 _gz_cct_report[30]=1;
 _gz_cct_report[42]=_gz_cct_report[43]=0;
 for(i=52;i<=56;i++)_gz_cct_report[i]=0;
 _gz_cct_entered=1;z=cct_current(&p,&c,&index);if(z)goto done;
 s=v->source;n=v->used;off=R32(s,48);
 if(R32(s,52)!=2048||off<216||(off&3)||off>n||2048>n-off){z=42;goto done;}
 j->phase=CCT_PREPARED;j->magic=0x43435431u;j->bytes=sizeof(*j);j->version=1;
 j->token=token;j->frame=frame;j->camera=c;j->body=c+0x248;j->pid=p.pid;j->index=index;
 j->style=R32(j->body,0x514);if(j->style>144){z=45;goto done;}
 j->algorithm=_gz_cct_native_styles[j->style*33];j->link=_gz_roh_owner[29];j->link_pid=_gz_roh_owner[30];
 j->source=s;j->source_used=n;j->pin=v->pin;btr_copy(&j->process,&p,sizeof(p));
 btr_copy(j->param,(const void*)(j->body+0x8a4),12);
 for(i=0;i<2048;i++)j->before[i]=0;
 _gz_cct_report[30]=2;
 for(i=0;i<5;i++)_gz_cct_report[31+i]=R32(j->body,cct_link_fields[i]);
 z=_gz_camera_read(j->before);
 for(i=0;i<5;i++)_gz_cct_report[36+i]=R32(j->before,32+i*4);
 _gz_cct_report[41]=z;if(z)goto done;
 btr_copy(j->saved,(const void*)(s+off),2048);
 _gz_cct_report[46]=j->style;_gz_cct_report[47]=R32(j->saved,20);
 btr_copy(j->before+1600,(const void*)(j->body+0x37c),128);
 if(!cct_cache_code()||R32(j->param,4)!=camera_native_cache_base()+j->style*132||R32(j->param,8)!=j->style){z=45;goto done;}
 z=cct_record(j->before,j);if(z)goto done;z=cct_source_role(j);if(z)goto done;
 for(i=0;i<5;i++)j->before_links[i]=R32(j->body,cct_link_fields[i]);
 j->before_force=R32(j->body,0x130);j->after_force=R32(j->saved,412);
 j->source_hash=btr_hash(j->saved,2048);j->before_hash=btr_hash(j->before,2048);
 _gz_cct_report[30]=3;z=cct_before_links();if(z)goto done;
 z=cct_binding();if(z)goto done;z=cct_match(j->before,j->before_links,j->before_force);if(z)goto done;
 if(_gz_roh_gl_context(token,frame,1)!=1||!tx_observe_matches(token,frame,s,n,j->pin)||
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
 if(tx_core_has(token))return tx_core_forward(token,frame,5)?0:42;
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
  p=R32(j->saved,32+i*4);if(p==j->source_link_pid)p=j->link_pid;if(_gz_boh_has_owner()&&p==_gz_boh_plan.source_pid)p=_gz_boh_journal.pid;
  z=cct_resolve(p,&links[i]);if(z)goto done;
 }
 if(links[0]!=j->link){z=45;goto done;}
 force=j->after_force;if(force==j->source_link_pid)force=j->link_pid;if(_gz_boh_has_owner()&&force==_gz_boh_plan.source_pid)force=_gz_boh_journal.pid;
 for(i=0;i<5;i++)j->after_links[i]=links[i];j->after_force=force;
 j->applied_frame=frame;j->phase=CCT_APPLYING;cct_seal();
 if(R32(j->saved,20)!=j->style){
  if(!cct_cache_code()){z=45;goto done;}
  _gz_cct_report[42]=_gz_dck_native_change(j->body+0x8a4,(int)R32(j->saved,20));
  if(_gz_cct_report[42]!=1){z=45;goto done;}
 }
 cct_write(j->saved,j->after_links,j->after_force);
 z=cct_match(j->saved,j->after_links,j->after_force);if(z)goto done;
 _gz_cct_report[48]=R32(j->body,0x514);_gz_cct_report[49]=R32(j->body,0x8ac);
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
 if(!(tx_core_has(token)?tx_core_recovery(token,frame):_gz_boh_recovery_authority(token,_gz_boh_journal.actor,j->link,frame))){z=40;goto done;}
 z=cct_binding();if(z)goto done;z=cct_before_links();if(z)goto done;
 if(j->phase==CCT_PREPARED){z=cct_match(j->before,j->before_links,j->before_force);if(z)goto done;}
 else{
  j->phase=CCT_UNDOING;cct_seal();
  if(R32(j->body,0x8ac)!=j->style){
   if(!cct_cache_code()){z=45;goto done;}
   _gz_cct_report[43]=_gz_dck_native_change(j->body+0x8a4,(int)j->style);
   if(_gz_cct_report[43]!=1){z=45;goto done;}
  }
  cct_write(j->before,j->before_links,j->before_force);
  z=cct_match(j->before,j->before_links,j->before_force);if(z)goto done;
 }
 z=cct_binding();if(z)goto done;
 if(!(tx_core_has(token)?tx_core_recovery(token,frame):_gz_boh_recovery_authority(token,_gz_boh_journal.actor,j->link,frame))){z=40;goto done;}
 _gz_cct_report[50]=R32(j->body,0x514);_gz_cct_report[51]=R32(j->body,0x8ac);
 j->rolled_frame=frame;j->phase=CCT_ROLLED;cct_seal();_gz_cct_report[3]=CCT_ROLLED;
 _gz_cct_report[7]=frame;_gz_cct_report[19]=1;
done:
 _gz_cct_report[4]=z;_gz_cct_entered=0;return z;
}
