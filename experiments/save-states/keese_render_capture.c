/* Fixed capture of per-instance Ki derived skeletal results. All native
 * actor/heap/result reads run under the new authenticated root READ callback
 * and its genuine common allocator exclusion. Current process generations,
 * registered solid topology and observed heap generation/epoch are checked
 * again before commit. Serialized payload contains only control words and
 * thirteen floats per bone, never the current audit/heap/resource addresses.
 * This module captures data; it grants no saved-world restore admission.
 */
#include "native_lifetime/lifetime.h"
#include "world_slot.h"
#include "keese_private_pool.h"
enum {KRC_CONTEXT=600,KRC_PROCESS=601,KRC_HEAP=602,KRC_LAYOUT=603,KRC_CHANGED=604,
 KRC_ALIAS=605,KRC_NUMERIC=606,KRC_RECORD=607,KRC_OFFSET=6600,KRC_PAYLOAD=3608};
typedef struct {
 PoEntry process;GhIdentity heap;HlHeap stamp;
 u32 morph,results,bones,flags,body_audit;u8 data[64*56];u32 checksum;
} KrcRow;
typedef struct {
 u32 magic,bytes,valid,frame,world_hash,world_sequence,count,pool;
 KmpIdentity pool_stamp;KrcRow rows[EKS_CAP];
} KrcBank;
typedef char KrcRowSize[sizeof(KrcRow)==3784?1:-1];
typedef char KrcBankSize[sizeof(KrcBank)==363456?1:-1];
static KrcBank _gz_krc_stage;
static KrcRow _gz_krc_scratch;
static u32 _gz_krc_ready,_gz_krc_seal,_gz_krc_entered,_gz_krc_frame,_gz_krc_mode,_gz_krc_world_hash,_gz_krc_sequence;
u32 _gz_krc_report[32]={0x4b524331u,128};
extern u32 _gz_rkc_capture_access(u32),_gz_rkc_read_scope(u32,u32),_gz_rkc_private_apart(u32,u32);
extern volatile u32 _gz_mca_blend_work,_gz_mca_blend_saved,_gz_mca_blend_observed,_gz_mca_blend_work_count,_gz_inplace_enabled;
u32 _gz_krc_private_apart(u32 p,u32 n){u32 i;u32 a[]={
 (u32)&_gz_krc_ready,(u32)&_gz_krc_seal,(u32)&_gz_krc_entered,(u32)&_gz_krc_frame,
 (u32)&_gz_krc_mode,(u32)&_gz_krc_world_hash,(u32)&_gz_krc_sequence};
 if(!epj_ram(p,n)||!_gz_rkc_private_apart(p,n)||!_gz_kmp_private_apart(p,n)||
  !epj_apart(p,n,(u32)&_gz_krc_stage,sizeof(_gz_krc_stage))||
  !epj_apart(p,n,(u32)&_gz_krc_scratch,sizeof(_gz_krc_scratch))||
  !epj_apart(p,n,(u32)_gz_krc_report,sizeof(_gz_krc_report)))return 0;
 for(i=0;i<7;i++)if(!epj_apart(p,n,a[i],4))return 0;return 1;}
static u32 krc_quiet(void){return !_gz_pbac_pending&&!*(volatile u8*)&_gz_pbac_loading&&
 !_gz_pbac_world_resume_pending&&!_gz_roh_user_guard()&&!_gz_osa_pin_id()&&_gz_pbac_idle_for_capture()&&_gz_inplace_enabled!=0;}
u32 _gz_krc_kernel_scope(u32 mode,u32 frame){const VrWorld*w=&_gz_vws_stage;return
 _gz_krc_entered==1&&mode==_gz_krc_mode&&(mode==1||mode==2)&&frame==_gz_krc_frame&&
 frame==_gz_pbac_frame&&_gz_vws_ready==1&&w->complete==1&&w->frame==frame&&
 w->checksum==_gz_krc_world_hash&&w->sequence==_gz_krc_sequence&&krc_quiet();}
static u32 krc_bank_hash(const KrcBank*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<p->bytes;i++)h=(h^((i>=8&&i<12)?0:q[i]))*16777619u;return h;}
static u32 krc_sealed(void){const KrcBank*p=&_gz_krc_stage;return _gz_krc_ready==1&&p->magic==0x4b524331u&&p->valid==1&&
 p->count<=EKS_CAP&&p->bytes==192+p->count*sizeof(KrcRow)&&_gz_krc_seal==krc_bank_hash(p);}
/* Heap metadata is copied under the real shared observer lock. It remains
 * missing31 evidence; the entered root callback supplies the separate native
 * mutator exclusion and same-frame Management boundary needed for these reads.
 */
static u32 krc_stamp(u32 heap,HlHeap*out){u32 i,start=((heap>>2)*0x9e3779b1u)>>20,z=KRC_HEAP;
 if(_gz_hl_control.magic!=HL_MAGIC||_gz_hl_control.bytes!=64||_gz_hl_control.version!=1||!_gz_hl_meta_try_lock())return KRC_HEAP;
 if(_gz_hl_control.flags||_gz_hl_control.active_heaps>HL_HEAPS||_gz_hl_control.active_allocations>HL_ALLOCS)goto done;
 for(i=0;i<HL_HEAPS;i++){const HlHeap*h=_gz_hl_heaps+((start+i)&4095u);
  if(!h->heap)break;if(h->heap!=heap)continue;
  /* Epoch0 is the fresh constructor state: lifetime.c zeroes HlHeap and
   * mutators.inc assigns a nonzero token only after an observed mutation.
   * Equality is checked again; zero is not absence of a generation.
   * A completed, observed rebirth has a new generation and is usable.
   * Global HL_REPLACED still refuses an overlapping LIVE replacement. */
  if(h->state!=HL_LIVE||h->kind!=HL_SOLID||!h->generation||(h->flags&~HL_REBORN)||h->end_frame||h->dtor_site)break;
  btr_copy(out,h,sizeof(*out));z=0;break;}
done:_gz_hl_meta_unlock();return z;}
static u32 krc_pool(KmpIdentity*out){u32 p=_gz_mca_blend_work;
 if(_gz_kmp_validate()||!_gz_krc_private_apart(p,0x2d0000))return KRC_HEAP;
 btr_copy(out,&_gz_kmp_identity,sizeof(*out));
 return 0;}
static u32 krc_heap(const GhIdentity*expected,u32 heap,u32 pointer,u32 bytes){
 if(!_gz_krc_private_apart(pointer,bytes)||_gz_gh_solid(heap,pointer,bytes)||!_gz_gh_identity.valid)return KRC_HEAP;
 return expected&&!btr_equal(expected,(const void*)&_gz_gh_identity,40)?KRC_CHANGED:0;}
static u32 krc_numbers(u32 p,u32 bones){u32 i,j;
 for(i=0;i<bones;i++)for(j=4;j<56;j+=4)if(!epj_finite(R32(p,i*56+j)))return 0;return 1;}
static u32 krc_row(const VrActor*actor,KrcRow*r){u32 a=actor->actor,m,heap,n,p,i,z;GhIdentity current;HlHeap last;
 const PoEntry*process;
 eks_zero(r,sizeof(*r));
 if(_gz_po_lookup(a)||_gz_po_observation.valid!=1||_gz_po_observation.flags)return KRC_PROCESS;
 process=&_gz_po_observation.entry;
 if(process->object!=a||process->pid!=actor->pid||process->process_name!=215||process->state!=PO_LIVE||
  !process->generation||process->flags||process->end_frame||process->allocation_bytes<0x10bc||process->allocation_bytes>0x100000||
  process->base_bytes+process->other_bytes<process->base_bytes||process->allocation_bytes!=process->base_bytes+process->other_bytes||
  !_gz_krc_private_apart(a,process->allocation_bytes))return KRC_PROCESS;
 btr_copy(&r->process,process,sizeof(*process));
 if(R32(a,4)!=process->pid||R16(a,8)!=215||R8(a,12)!=2||R16(a,14)!=process->profile||R32(a,16)!=process->profile_entry)return KRC_PROCESS;
 heap=R32(a,0xf4);m=R32(a,0x3d0);
 z=krc_heap(0,heap,m,0xc8);if(z)return z;btr_copy(&current,(const void*)&_gz_gh_identity,sizeof(current));
 z=krc_stamp(heap,&r->stamp);if(z)return z;
 if(r->stamp.start!=heap||r->stamp.bytes!=current.bytes||r->stamp.vtable!=current.vtable||r->stamp.parent!=current.parent)return KRC_HEAP;
 n=R32(m,0x50);p=R32(m,0x78);
 if(R32(m,0)!=0x10058cb8u||!n||n>64||R16(m,0x42)!=n||R16(m,0x7c)!=n||R16(m,0x7e)!=n||
  R32(m,0x84)!=p||R32(m,0x88)!=p||R32(m,0x8c)!=p||(R32(m,0x80)&~15u))return KRC_LAYOUT;
 z=krc_heap(&current,heap,p,n*56);if(z)return z;
 if(!epj_apart(p,n*56,a,r->process.allocation_bytes)||!epj_apart(p,n*56,m,0xc8)||
  !epj_apart(heap,current.bytes,_gz_krc_stage.pool,0x2d0000))return KRC_ALIAS;
 /* Qualify every other complete object/range before comparing disjointness.
  * This reader does not parse their contents or follow a resource wrapper. */
 {u32 targets[5]={R32(m,0x90),R32(a,0x4e4),R32(m,0x34),R32(m,0xa8),R32(m,0xac)};
  u32 sizes[5]={0xb0,0x74,n*48,n*32,n*16};
  for(i=0;i<5;i++){z=krc_heap(&current,heap,targets[i],sizes[i]);if(z)return z;
   if(!epj_apart(p,n*56,targets[i],sizes[i]))return KRC_ALIAS;}}
 /* Exclude complete evaluator result/binding/cache ranges, not just their
  * numeric row prefixes. A cache contains resource addresses and cannot be
  * reclassified as a typed renderer bank merely because it lies in this heap.
  * All header reads below are inside the already qualified morph/face objects.
  */
 {u32 buf=R32(m,0x34),bind=R32(m,0x3c),cache=R32(m,0x20),cap=R32(m,0x24),end;
  if(!cap||cap>512||R32(m,0x30)!=buf||bind!=buf+n*48||cache!=bind+n*4)return KRC_LAYOUT;
  end=cache+cap*12;if(end<cache||end<buf||end-buf>8192)return KRC_LAYOUT;
  z=krc_heap(&current,heap,buf,end-buf);if(z)return z;if(!epj_apart(p,n*56,buf,end-buf))return KRC_ALIAS;
 }
 {u32 face=R32(a,0x4e4)+0x1c,buf=R32(face,0x30),cap=R32(face,0x20),cache=R32(face,0x1c);
  u32 ac=R32(face,0x4c),rc=R32(face,0x50),mat=R16(face,0x3e),bind=R32(face,0x38),map=R32(face,0x54),end;
  if(!cap||cap>512||!ac||ac>64||!rc||rc>64||!mat||mat>64||R32(face,0x2c)!=buf||
   bind!=buf+((ac*2+3)&~3u)||map!=bind+mat*4||cache!=((map+rc+3)&~3u))return KRC_LAYOUT;
  end=cache+cap*12;if(end<cache||end<buf||end-buf>8192)return KRC_LAYOUT;
  z=krc_heap(&current,heap,buf,end-buf);if(z)return z;if(!epj_apart(p,n*56,buf,end-buf))return KRC_ALIAS;
 }
 if(!krc_numbers(p,n))return KRC_NUMERIC;
 btr_copy(&r->heap,&current,sizeof(current));r->morph=m;r->results=p;r->bones=n;r->flags=R32(m,0x80);r->body_audit=R32(m,0x94);
 btr_copy(r->data,(const void*)p,n*56);
 z=krc_stamp(heap,&last);if(z)return z;if(!btr_equal(&last,&r->stamp,sizeof(last)))return KRC_CHANGED;
 if(_gz_po_lookup(a)||_gz_po_observation.valid!=1||_gz_po_observation.flags||
  !btr_equal(&r->process,&_gz_po_observation.entry,sizeof(r->process)))return KRC_CHANGED;
 r->checksum=btr_hash(r,sizeof(*r)-4);return 0;}
/* Called only by the fixed root reader after its actual lease is acquired.
 * No supplied address selects a native read or a staging destination. */
u32 _gz_krc_read_under_root(u32 mode,u32 frame){KrcBank*p=&_gz_krc_stage;const VrWorld*w=&_gz_vws_stage;
 u32 i,count=0,z;KmpIdentity pool;
 if(!_gz_krc_kernel_scope(mode,frame)||!_gz_rkc_read_scope(mode,frame))return KRC_CONTEXT;
 z=krc_pool(&pool);if(z)return z;
 if(mode==1){p->pool=_gz_mca_blend_work;btr_copy(&p->pool_stamp,&pool,sizeof(pool));}
 else if(!krc_sealed()||p->pool!=_gz_mca_blend_work||!btr_equal(&pool,&p->pool_stamp,sizeof(pool)))return KRC_CHANGED;
 if(w->count>VR_CAPACITY)return KRC_CONTEXT;
 for(i=0;i<w->count;i++)if(w->actors[i].profile==215){
  if(count==EKS_CAP)return KRC_LAYOUT;_gz_krc_report[6]=i;
  z=krc_row(w->actors+i,&_gz_krc_scratch);if(z)return z;
  if(mode==1)btr_copy(p->rows+count,&_gz_krc_scratch,sizeof(KrcRow));
  else if(count>=p->count||!btr_equal(p->rows+count,&_gz_krc_scratch,sizeof(KrcRow)))return KRC_CHANGED;
  count++;}
 if(mode==1){p->count=count;p->bytes=192+count*sizeof(KrcRow);}
 else if(count!=p->count)return KRC_CHANGED;
 return _gz_rkc_read_scope(mode,frame)?0:KRC_CONTEXT;}
static u32 krc_read(u32 mode){const VrWorld*w=&_gz_vws_stage;u32 z;
 if(_gz_krc_entered||!krc_quiet()||w->frame!=_gz_pbac_frame)return KRC_CONTEXT;
 _gz_krc_frame=w->frame;_gz_krc_world_hash=w->checksum;_gz_krc_sequence=w->sequence;_gz_krc_mode=mode;_gz_krc_entered=1;
 z=_gz_rkc_capture_access(mode);_gz_krc_entered=_gz_krc_mode=0;_gz_krc_report[3]=z;return z;}
u32 _gz_krc_stage_capture(void){KrcBank*p=&_gz_krc_stage;u32 z;
 if(_gz_krc_entered||!krc_quiet())return KRC_CONTEXT;
 _gz_krc_ready=_gz_krc_seal=0;p->valid=0;z=_gz_vws_validate_ready();if(z)return z;
 eks_zero(p,192);p->magic=0x4b524331u;p->bytes=192;p->frame=_gz_vws_stage.frame;
 p->world_hash=_gz_vws_stage.checksum;p->world_sequence=_gz_vws_stage.sequence;
 z=krc_read(1);if(z)return z;_gz_krc_seal=krc_bank_hash(p);
 __asm__ volatile("sync":::"memory");p->valid=1;_gz_krc_ready=1;
 _gz_krc_report[2]=p->frame;_gz_krc_report[4]=p->count;_gz_krc_report[5]=_gz_krc_seal;return 0;}
static u32 krc_payload_hash(u32 p,u32 bytes){u32 i,h=2166136261u;
 for(i=0;i<bytes;i++)h=(h^((i<4||(i>=20&&i<24))?0:*(const u8*)(p+i)))*16777619u;return h;}
static u32 krc_record(const KrcRow*r,u32 row){u32 p=row+KRC_OFFSET,n=24+r->bones*56;
 return R32(p,0)==0x4b525331u&&R32(p,4)==n&&R32(p,8)==r->bones&&R32(p,12)==r->flags&&
  R32(p,16)==R32(row,4832)&&R32(p,20)==krc_payload_hash(p,n)&&
  btr_equal((const void*)(p+24),r->data,r->bones*56);}
u32 _gz_krc_capture_record(u32 actor,u32 row){const KrcBank*p=&_gz_krc_stage;const KrcRow*r=0;u32 i,q,n;
 if(!krc_sealed()||p->frame!=_gz_pbac_frame||p->world_hash!=_gz_vws_stage.checksum||
  p->world_sequence!=_gz_vws_stage.sequence||!krc_quiet()||_gz_mca_blend_work!=p->pool||
  !epj_ram(row,10240)||row<p->pool||row-p->pool>=0xf0000||(row-p->pool)%10240)return KRC_CONTEXT;
 for(i=0;i<p->count;i++)if(p->rows[i].process.object==actor){if(r)return KRC_RECORD;r=p->rows+i;}
 if(!r||R32(row,0)!=r->process.pid||R32(row,48)!=r->bones||R32(row,52)!=215||R32(row,56)!=0x3d0||
  R32(row,4800)!=1||R32(row,4808)!=r->body_audit||R32(row,4832)>4095)return KRC_RECORD;
 q=row+KRC_OFFSET;n=24+r->bones*56;eks_zero((void*)q,KRC_PAYLOAD);
 *(u32*)(q+4)=n;*(u32*)(q+8)=r->bones;*(u32*)(q+12)=r->flags;*(u32*)(q+16)=R32(row,4832);
 btr_copy((void*)(q+24),r->data,r->bones*56);*(u32*)(q+20)=krc_payload_hash(q,n);
 __asm__ volatile("sync":::"memory");*(volatile u32*)q=0x4b525331u;return 0;}
u32 _gz_krc_validate_ready(void){const KrcBank*p=&_gz_krc_stage;u32 i,j,row,found,count=0,z,seen[3]={0,0,0};
 if(!_gz_inplace_enabled)return 0;
 if(!krc_sealed()||p->frame!=_gz_pbac_frame||p->world_hash!=_gz_vws_stage.checksum||
  p->world_sequence!=_gz_vws_stage.sequence)return KRC_CONTEXT;
 z=_gz_vws_validate_ready();if(z)return z;z=krc_read(2);if(z)return z;
 if(_gz_mca_blend_work_count>EKS_CAP)return KRC_RECORD;
 for(i=0;i<_gz_mca_blend_work_count;i++){row=p->pool+i*10240;if(R32(row,52)!=215)continue;found=EKS_CAP;
  for(j=0;j<p->count;j++)if(R32(row,0)==p->rows[j].process.pid){if(found!=EKS_CAP)return KRC_RECORD;found=j;}
  if(found==EKS_CAP||(seen[found/32]&(1u<<(found%32)))||R32(row,48)!=p->rows[found].bones||R32(row,56)!=0x3d0||
   R32(row,4800)!=1||R32(row,4808)!=p->rows[found].body_audit||
   !krc_record(p->rows+found,row))return KRC_RECORD;
  seen[found/32]|=1u<<(found%32);count++;}
 return count==p->count?0:KRC_RECORD;}
void _gz_krc_discard(void){if(!_gz_krc_entered&&!_gz_roh_user_guard())_gz_krc_ready=_gz_krc_stage.valid=_gz_krc_seal=0;}
