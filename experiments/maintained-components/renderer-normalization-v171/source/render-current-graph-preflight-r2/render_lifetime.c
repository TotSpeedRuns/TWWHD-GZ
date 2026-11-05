/* Successor guard plus normalized lifetime exporter. Fixed native providers
 * only. No reset is inferred from zeros: the wrapper's actual-reset receipt
 * is a separate fixed prerequisite. No native heap/body write or SDK call.
 */
#include "render_graph.c"
#include "lifetime_witness.h"
extern U _gz_rcl_normalized_scope(U,U);
#ifdef RCG_HOST_TEST
extern U _gz_rcl_test_pending_ea(void);
#define PENDING_EA() _gz_rcl_test_pending_ea()
#else
#define PENDING_EA() ((U)(uintptr_t)&_gz_rcl_pending)
#endif
typedef struct {Work w;RclWitness*v;U parent[RCL_HEAPS],header[RCL_HEAPS],level[RCL_HEAPS];} Life;
static U rcl_heap(Life*l,U p){U h[16],scope[8],slot,i,j;Work*w=&l->w;
 if(!heap(w,p,h,&slot))return 0;
 for(i=0;i<8;i++)scope[i]=meta(w,SYM(6)+slot*32+i*4);
 if(w->error||scope[5]||scope[6]||scope[7])return fail(w,RCG_UNSUPPORTED,p);
 for(i=0;i<l->v->heap_count;i++)if(l->v->heaps[i].heap[0]==p){
  for(j=0;j<16;j++)if(l->v->heaps[i].heap[j]!=h[j])return fail(w,RCG_CHANGED,p);
  for(j=0;j<8;j++)if(l->v->heaps[i].scope[j]!=scope[j])return fail(w,RCG_CHANGED,p);
  return i+1;
 }
 if(i==RCL_HEAPS)return fail(w,RCG_UNSUPPORTED,p);
 for(j=0;j<16;j++)l->v->heaps[i].heap[j]=h[j];
 for(j=0;j<8;j++)l->v->heaps[i].scope[j]=scope[j];
 l->v->heap_count=i+1;return i+1;
}
static U rcl_alloc(Life*l,U p,U n,U generation,U owner,RclAllocation*out){Work*w=&l->w;U i,j,start=(((p>>2)*0x9e3779b1u)>>14),found=0,ended=0;
 if(!span(p,n)||!apart(p,n,w->scratch,RCG_SCRATCH_BYTES))return fail(w,RCG_ALIAS,p);
 for(i=0;i<RCG_ALLOC_PROBES&&!w->error;i++){
  U base=SYM(4)+((start+i)&262143)*60,a[15],policy,kind,hi;RclHeap*h;
  a[0]=meta(w,base);if(!a[0]){ended=1;break;}if(a[0]!=p)continue;
  for(j=1;j<15;j++)a[j]=meta(w,base+j*4);
  policy=a[14];if((policy>>29)!=1)continue;
  /* Strict unique physical scope: a trusted generation stamp is not used to
   * ignore another LIVE generation/heap alias at this same address. */
  if(generation&&a[1]!=generation)return fail(w,RCG_UNSUPPORTED,p);
  kind=(policy>>26)&7;
  if(found++||!a[1]||!a[3]||(owner&&a[2]!=owner)||a[7]<n||a[7]<a[4]||!span(p,a[7])||
   kind<1||kind>3||a[9]||a[11]||a[12]||(((policy>>17)&511)&~8u)||((policy>>12)&31)!=31||(policy&4095))return fail(w,RCG_UNSUPPORTED,p);
  hi=rcl_heap(l,a[2]);if(!hi)return 0;h=&l->v->heaps[hi-1];
  if(h->heap[1]!=a[3]||h->heap[3]!=kind||h->heap[15]!=a[13]||!inside(p,a[7],h->heap[4],h->heap[5]))return fail(w,RCG_UNSUPPORTED,p);
  if(kind==1?(!span(a[6],16)||p<a[6]||p-a[6]<16||!inside(a[6],16,h->heap[4],h->heap[5])):a[6]!=p)return fail(w,RCG_UNSUPPORTED,p);
  out->pointer=p;out->generation=a[1];out->heap=a[2];out->heap_generation=a[3];out->heap_epoch=a[13];out->requested=a[4];out->capacity=a[7];out->block=a[6];out->kind=kind;
 }
 if(w->error)return 0;if(!ended)return fail(w,RCG_LIMIT,p);return found?1:fail(w,RCG_UNSUPPORTED,p);
}
static U rcl_normalized(Life*l,U second){Work*w=&l->w;U i,j,owner=w->owner,q,count,array,used;RclWitness*v=l->v;
 if(meta(w,SYM(1))!=1)return fail(w,RCG_UNSUPPORTED,SYM(1));
 for(i=0;i<24;i++){
  q=meta(w,owner+offsets[i]);
  if(!second){
   if(!rcl_alloc(l,q,64,0,0,&v->allocations[i*2])||v->allocations[i*2].requested!=64)return fail(w,RCG_UNSUPPORTED,q);
  }else if(q!=v->allocations[i*2].pointer)return fail(w,RCG_CHANGED,q);
  count=meta(w,q);array=meta(w,q+4);used=meta(w,q+0x20);
  if(w->error||count!=counts[i]||used)return fail(w,RCG_GRAPH,q);
  if(!second){
   if(!rcl_alloc(l,array,count*4,0,0,&v->allocations[i*2+1])||v->allocations[i*2+1].requested!=count*4)return fail(w,RCG_UNSUPPORTED,array);
   v->queues[i].count=count;v->queues[i].array=array;v->queues[i].used=0;
  }else if(array!=v->queues[i].array||count!=v->queues[i].count)return fail(w,RCG_CHANGED,q);
  for(j=0;j<count;j++)if(meta(w,array+j*4))return fail(w,RCG_GRAPH,array+j*4);
 }
 if(meta(w,SYM(2)+0x74)!=v->allocations[11*2].pointer||meta(w,SYM(2)+0x78)!=v->allocations[12*2].pointer)return fail(w,RCG_GRAPH,SYM(2)+0x74);
 for(i=0;i<4;i++){
  const U cursor[4]={0x8c,0xd4,0x1dc,0x264},base[4]={0x7c,0x94,0xdc,0x1e4};
  if(meta(w,owner+cursor[i])!=owner+base[i])return fail(w,RCG_GRAPH,owner+cursor[i]);
 }
 for(i=0;i<2;i++){U e=owner+(i?0x320:0x284);
  if(meta(w,e+0x94)||meta(w,e+0x98))return fail(w,RCG_GRAPH,e+0x94);
 }
 return !w->error;
}
static U rcl_closure(Life*l){U i,j,round,closed=0;Work*w=&l->w;RclWitness*v=l->v;
 for(i=0;i<v->heap_count;i++){
  RclHeap*h=&v->heaps[i];RclAllocation managed,header;U parent,ho,head=h->heap[3]==1?0xb8:h->heap[3]==2?0x9c:0xac;
  if(!h->heap[6]){for(j=0;j<8;j++)if(h->scope[j])return fail(w,RCG_UNSUPPORTED,h->heap[0]);l->level[i]=1;continue;}
  if(!h->heap[7]||!h->scope[1]||!h->scope[2]||!h->scope[3]||!h->scope[4])return fail(w,RCG_UNSUPPORTED,h->heap[0]);
  parent=rcl_heap(l,h->heap[6]);if(!parent)return 0;
  if(v->heaps[parent-1].heap[1]!=h->heap[7]||v->heaps[parent-1].heap[15]!=h->scope[0])return fail(w,RCG_UNSUPPORTED,h->heap[0]);
  if(!rcl_alloc(l,h->scope[1],4,h->scope[2],h->heap[6],&managed)||
   !inside(h->heap[4],h->heap[5],managed.pointer,managed.capacity)||managed.heap_epoch!=h->scope[0])return fail(w,RCG_UNSUPPORTED,h->heap[0]);
  if(!rcl_alloc(l,h->scope[3],4,h->scope[4],0,&header)||!inside(h->heap[0],head,header.pointer,header.capacity))return fail(w,RCG_UNSUPPORTED,h->heap[0]);
  ho=rcl_heap(l,header.heap);if(!ho)return 0;l->parent[i]=parent;l->header[i]=ho;
 }
 /* Dependency closure includes separate header-owner chains. A finite LIVE
  * cycle is not a physical backing proof. Only parentless, zero-scope roots
  * close initially; every dependency must reach one within depth32. */
 for(round=0;round<RCL_DEPTH;round++){
  closed=0;
  for(i=0;i<v->heap_count;i++){
   U a=l->parent[i],b=l->header[i],level;
   if(!l->level[i]&&a&&b&&l->level[a-1]&&l->level[b-1]){level=l->level[a-1]>l->level[b-1]?l->level[a-1]:l->level[b-1];if(level<RCL_DEPTH)l->level[i]=level+1;}
   if(l->level[i])closed++;
  }
  if(closed==v->heap_count)return 1;
 }
 return fail(w,RCG_UNSUPPORTED,SYM(5));
}
U _gz_rcl_export(U req,U frame,U scratch,U bytes){Life l;U i,hg,z,pending=PENDING_EA();RclWitness*v=0;
 if(!req||!frame||bytes<RCG_SCRATCH_BYTES||!span(scratch,RCG_SCRATCH_BYTES)||!span(pending,sizeof(RclWitness))||!apart(scratch,RCG_SCRATCH_BYTES,pending,sizeof(RclWitness))){
  _gz_rns_trace(req,frame,RT_RCL_ARGUMENT,RCG_ARGUMENT,scratch);return RCG_ARGUMENT;}
 if(!_gz_ghs_scope(req,frame,2)){
  _gz_rns_trace(req,frame,RT_RCL_SCOPE,RCG_SCOPE,1);return RCG_SCOPE;}
 if(!(hg=_gz_ghs_hg(req,frame))){
  _gz_rns_trace(req,frame,RT_RCL_SCOPE,RCG_SCOPE,2);return RCG_SCOPE;}
 if(!_gz_rcl_normalized_scope(req,frame))return RCG_SCOPE;
 if(!_gz_ois_heap_stage_match(req,frame,scratch,RCG_SCRATCH_BYTES)){
  _gz_rns_trace(req,frame,RT_RCL_SCOPE,RCG_SCOPE,4);return RCG_SCOPE;}
 if(!apart(pending,sizeof(RclWitness),SYM(0),0x3bc)||!apart(pending,sizeof(RclWitness),SYM(1),4)||
  !apart(pending,sizeof(RclWitness),SYM(2),0x100)||!apart(pending,sizeof(RclWitness),SYM(3),64)||
  !apart(pending,sizeof(RclWitness),SYM(4),262144u*60u)||!apart(pending,sizeof(RclWitness),SYM(5),4096u*64u)||
  !apart(pending,sizeof(RclWitness),SYM(6),4096u*32u)||!apart(pending,sizeof(RclWitness),SYM(7),148)){
  _gz_rns_trace(req,frame,RT_RCL_PENDING_APART,RCG_ALIAS,pending);return RCG_ALIAS;}
 _gz_rns_trace(req,frame,RT_RCL_GUARD_ENTER,0,0);
 z=_gz_rcg_preflight(req,frame,scratch,bytes);if(z)return z;
 _gz_rns_trace(req,frame,RT_RCL_GUARD_OK,0,0);
 for(i=0;i<(U)sizeof(l);i++)((unsigned char*)&l)[i]=0;
 l.w.scratch=scratch;l.w.owner=SYM(0);l.w.initial_raw=load(SYM(7)+12);l.w.initial_bytes=load(SYM(7)+16);
 if(!span(l.w.initial_raw,l.w.initial_bytes)){
  _gz_rns_trace(req,frame,RT_RCL_RAW_SPAN,RCG_SCOPE,l.w.initial_raw);return RCG_SCOPE;}
 if(!apart(pending,sizeof(RclWitness),l.w.initial_raw,l.w.initial_bytes)){
  _gz_rns_trace(req,frame,RT_RCL_RAW_PENDING_APART,RCG_SCOPE,l.w.initial_raw);return RCG_SCOPE;}
 if(!_gz_ois_heap_apart(req,frame,l.w.initial_raw,l.w.initial_bytes)){
  _gz_rns_trace(req,frame,RT_RCL_RAW_APART,RCG_SCOPE,l.w.initial_raw);return RCG_SCOPE;}
 if(!_gz_hl_meta_try_lock()){
  _gz_rns_trace(req,frame,RT_RCL_BUSY,RCG_BUSY,SYM(3));return RCG_BUSY;}
 l.w.stage=1;if(!controls(&l.w)){z=l.w.error?l.w.error:RCG_CHANGED;goto traced_unlock;}
 l.w.stage=2;if(!proposal(&l.w,req,frame)){z=l.w.error?l.w.error:RCG_CHANGED;goto traced_unlock;}
 /* Scratch has not been written before the locked raw/root/private checks. */
 v=(RclWitness*)scratch_pointer(scratch,RCG_SCRATCH_BYTES);l.v=v;
 for(i=0;i<sizeof(*v)/4;i++)((U*)v)[i]=0;
 v->magic=RCL_MAGIC;v->version=1;v->bytes=sizeof(*v);v->request=req;v->frame=frame;v->hg=hg;v->owner=l.w.owner;v->initialized=1;v->allocation_count=48;v->queue_count=24;v->raw=l.w.raw;v->raw_bytes=l.w.bytes;
 l.w.stage=3;if(!rcl_normalized(&l,0)){z=l.w.error?l.w.error:RCG_CHANGED;goto traced_unlock;}
 l.w.stage=4;if(!rcl_closure(&l)){z=l.w.error?l.w.error:RCG_CHANGED;goto traced_unlock;}
 l.w.stage=5;if(!rcl_normalized(&l,1)){z=l.w.error?l.w.error:RCG_CHANGED;goto traced_unlock;}
 v->j3d_opa=v->allocations[22].pointer;v->j3d_xlu=v->allocations[24].pointer;
 l.w.pass=1;l.w.stage=6;if(!controls(&l.w)){z=l.w.error?l.w.error:RCG_CHANGED;goto traced_unlock;}
 l.w.stage=7;if(!proposal(&l.w,req,frame)){z=l.w.error?l.w.error:RCG_CHANGED;goto traced_unlock;}
 z=RCG_OK;
 goto unlock;
traced_unlock:
 _gz_rns_trace(req,frame,RT_RCL_WALK|(l.w.pass<<4)|l.w.stage,z,l.w.bad);
unlock:
 _gz_hl_meta_unlock();
 if(!_gz_ghs_scope(req,frame,2)){
  _gz_rns_trace(req,frame,RT_RCL_POST_SCOPE,RCG_SCOPE,1);return RCG_SCOPE;}
 if(_gz_ghs_hg(req,frame)!=hg){
  _gz_rns_trace(req,frame,RT_RCL_POST_SCOPE,RCG_SCOPE,2);return RCG_SCOPE;}
 if(!_gz_rcl_normalized_scope(req,frame))return RCG_SCOPE;
 if(!_gz_ois_heap_stage_match(req,frame,scratch,RCG_SCRATCH_BYTES)){
  _gz_rns_trace(req,frame,RT_RCL_POST_SCOPE,RCG_SCOPE,4);return RCG_SCOPE;}
 if(z)return z;
 if(!v){_gz_rns_trace(req,frame,RT_RCL_NO_WITNESS,RCG_CHANGED,0);return RCG_CHANGED;}
 _gz_rcl_pending.valid=0;
 for(i=0;i<sizeof(*v)/4;i++)((U*)&_gz_rcl_pending)[i]=((const U*)v)[i];
 __sync_synchronize();_gz_rcl_pending.valid=1;
 _gz_rns_trace(req,frame,RT_RCL_PUBLISHED,0,sizeof(*v));return RCG_OK;
}
