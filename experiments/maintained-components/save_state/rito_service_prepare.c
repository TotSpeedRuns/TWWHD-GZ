/* Native read-only preparation under the already acquired genuine NSV HG
 * lease and source pin. All before-images remain private; no Rito setter,
 * application/recovery caller or whole-world publication is added here.
 */
enum {RTV_CONTEXT=1430,RTV_UNRECORDED=1431,RTV_STORAGE=1432,RTV_CHANGED=1433,
 RTV_OFFSET=2585152,RTV_CAP=16,RTV_ROW=147552,RTV_BYTES=2361984};
typedef struct {KbjPlan body;KrjPlan render;KbtPlan binding;RbpPlan numeric;RrpPlan result;u8 pad[4];} RtvRole;
typedef char RtvRoleSize[sizeof(RtvRole)==22688?1:-1];
typedef struct {u32 index,pid,source_row,source_core;PoEntry process;GhIdentity heap;HlHeap stamp;ErnResource archive;
 u32 source_hash,roles,source_frame,seal;RriBinding identity;RtvRole controllers[4];RbcPlan core;RmjPlan models;RtjPlan texture;RpjPlan physics;u8 pad[4];} RtvRow;
typedef char RtvRowSize[sizeof(RtvRow)==RTV_ROW?1:-1];
typedef struct {u32 magic,bytes,valid,entered,token,frame,phase,count,prepared,error;
 u32 source,used,pin,bank,bank_bytes,bank_hash,rows_hash,missing,reserved[14];} RtvHeader;
typedef char RtvHeaderSize[sizeof(RtvHeader)==128?1:-1];
typedef char RtvPoolFits[RTV_OFFSET+RTV_BYTES<=NCB_POOL?1:-1];
extern u32 _gz_nsv_heap_scope(u32,u32);
static RtvHeader*rtv_header(void){return (RtvHeader*)ncb_pointer(NCR_RTV);}
static u32 rtv_capacity(const RtvHeader*h){return h&&h==rtv_header()?ncb_capacity(NCR_RTV):0;}
static u32 rtv_row_capacity(const RtvHeader*h){u32 n=rtv_capacity(h);return n>=128&&!((n-128)%(RTV_ROW+64))&&(n-128)/(RTV_ROW+64)<=16?(n-128)/(RTV_ROW+64):0;}
static RtvRow*rtv_row(RtvHeader*h,u32 i){return i<rtv_row_capacity(h)?(RtvRow*)((u32)h+128+i*RTV_ROW):0;}
static u32 rtv_run_address(RtvHeader*h,u32 i){return i<rtv_row_capacity(h)?(u32)h+128+rtv_row_capacity(h)*RTV_ROW+i*64:0;}
static u32 rtv_row_member(u32 p){RtvHeader*h=rtv_header();u32 first=(u32)h+128,n=rtv_row_capacity(h),d;
 if(!h||p<first)return 0;d=p-first;return !(d%RTV_ROW)&&d/RTV_ROW<n;}
static u32 rtv_hash(const RtvRow*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p);i++)h=(h^((i>=332&&i<336)?0:q[i]))*16777619u;return h;}
/* Independently rediscover the current actor from the genuine native queue.
 * Source addresses are audit keys only. The source bank and birth catalog
 * have already been authenticated under this same real NSV read hold.
 */
#include "rito_role_relation.c"
static u32 rri_context(RtvHeader*h){
 if(!h||h->magic!=0x52545631u||!h->token||!h->frame||h->count>rtv_row_capacity(h))return 0;
 if(h->entered)return h->phase==1&&_gz_nsv_heap_scope(h->token,h->frame);
 return h->valid==1&&h->phase==2&&!h->error&&h->prepared==h->count&&_gz_nsg_retained_frame(h->frame);
}
static u32 rri_identity(const PoEntry*e,u32 packet){RtvHeader*h;RtvRow*r;u32 i,found=0;
 if(R32(packet,28)!=e->profile)return 0;
 if(R32(packet,16)==e->pid&&R32(packet,20)==e->generation)return 1;
 h=rtv_header();if(!rri_context(h))return 0;
 for(i=0;i<h->count;i++){
  r=rtv_row(h,i);
  if(r->identity.current_frame==h->frame&&btr_equal(&r->process,e,sizeof(*e))&&
   _gz_rri_record_match(&r->identity,e,R32(packet,16),R32(packet,20),R32(packet,24),_gz_pbac_epoch))found++;
 }
 return found==1;
}
static u32 rri_source_context(RtvHeader*h,const VrWorld**out){const VrWorld*w;const BbBank*b;u32 o;
 if(R32(h->source,172)!=sizeof(VrWorld))return 0;o=R32(h->source,168);
 if(o<216||(o&3)||o>h->used||sizeof(VrWorld)>h->used-o)return 0;w=(const VrWorld*)(h->source+o);
 o=R32(h->source,192);if(o<216||(o&3)||o>h->used||BB_HEADER>h->used-o)return 0;b=(const BbBank*)(h->source+o);
 /* The bank is variably sized, not a full-capacity BbBank allocation. */
 if(b->bytes>h->used-o||_gz_vws_validate(w,sizeof(*w))||b->process_session!=_gz_po_header.session||
  !lpc_sealed()||_gz_lpc_run.phase!=LPC_PREPARED||_gz_lpc_plan.token!=h->token||_gz_lpc_plan.frame!=h->frame||
  _gz_lpc_plan.source!=h->source||_gz_lpc_plan.used!=h->used||_gz_lpc_plan.pin!=h->pin||
  _gz_lpc_plan.actor!=_gz_roh_owner[29]||_gz_lpc_plan.pid!=_gz_roh_owner[30]||
  R32(_gz_lpc_plan.saved,4)!=w->link_pid||R32(_gz_lpc_plan.saved,8)!=w->link_actor||
  w->stage0!=_gz_roh_owner[41]||w->stage1!=_gz_roh_owner[42]||w->room!=_gz_roh_owner[43]||w->layer!=_gz_roh_owner[44]||
  _gz_roh_owner[45]!=_gz_pbac_epoch||!w->epoch||w->epoch>_gz_pbac_epoch)return 0;
 *out=w;return 1;
}
/* A surviving source process already has the original strict identity.
 * It does not borrow Link's separate scene-rebirth plan: the bomb service
 * prepares NPCs before that plan exists. Rediscover queue membership, repeat
 * the entire PoEntry and native class checks under this real read hold, then
 * retain every original model/resource/core/physics predicate unchanged. */
static u32 rri_surviving(RtvHeader*h,RtvRow*out,const RtbRow*source,u32 index){u32 q,next,left=4096,count=0,head,z;PoEntry now;
 if(!_gz_nsv_heap_scope(h->token,h->frame))return RTV_CONTEXT;
 if(_gz_po_lookup(source->process.object)||!_gz_po_observation.valid||_gz_po_observation.flags)return 0;
 btr_copy(&now,&_gz_po_observation.entry,sizeof(now));
 if(!btr_equal(&now,&source->process,sizeof(now)))return 0;
 head=q=_gz_pba_native_actor_queue;
 while(q){if(!left--||!epj_ram(q,16))return RTV_CHANGED;next=R32(q,8);
  if(R32(q,12)==now.object&&++count!=1)return RTV_CHANGED;q=next;}
 if(count!=1||head!=_gz_pba_native_actor_queue)return RTV_CHANGED;
 z=rtb_live(now.object,&out->process);if(z)return z;
 if(!btr_equal(&out->process,&source->process,sizeof(now))||!_gz_nsv_heap_scope(h->token,h->frame))return RTV_CHANGED;
 out->identity.magic=0x52524931u;out->identity.relation=1;
 out->identity.source_pid=now.pid;out->identity.source_generation=now.generation;
 out->identity.source_frame=R32((u32)source->core,24);out->identity.current_frame=h->frame;
 out->identity.source_epoch=R32(h->bank,28);out->identity.current_epoch=_gz_pbac_epoch;
 out->identity.actor=now.object;out->identity.pid=now.pid;out->identity.generation=now.generation;
 out->identity.seal=rri_hash(&out->identity);
 h->reserved[0]=index;h->reserved[1]=1;h->reserved[2]=h->reserved[3]=now.pid;return 1;
}
static u32 rri_prepare_current(RtvHeader*h,RtvRow*out,const RtbRow*source,u32 index){
 const VrWorld*w;const VrActor*saved=0;VrActor current;PoEntry process;u32 i,q,a,next,left=4096,found=0,relation=0,head;
 relation=rri_surviving(h,out,source,index);if(relation)return relation==1?0:relation;
 if(!_gz_nsv_heap_scope(h->token,h->frame)||!rri_source_context(h,&w))return RTV_CONTEXT;
 for(i=0;i<w->count;i++)if(w->actors[i].pid==source->process.pid){if(saved)return RTV_CHANGED;saved=w->actors+i;}
 if(!saved||saved->actor!=source->process.object||saved->profile!=source->process.profile)return RTV_CHANGED;
 head=q=_gz_pba_native_actor_queue;
 while(q){
  if(!left--||!epj_ram(q,16))return RTV_CHANGED;next=R32(q,8);a=R32(q,12);
  if(a){
   if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return RTV_CHANGED;
   btr_copy(&process,&_gz_po_observation.entry,sizeof(process));
   if(process.profile==source->process.profile){
    if(rtb_live(a,&process))return RTV_CHANGED;
    current.actor=a;current.pid=process.pid;current.profile=process.profile;current.parameters=R32(a,0xb0);
    current.set_id=R16(a,0x2d8);current.parent_pid=R32(a,0x2e8);current.init_state=R8(a,12);
    for(i=0;i<5;i++)current.home[i]=R32(a,0x2ec+i*4);
    relation=_gz_rir_relation(&source->process,&process,saved,&current,w->frame,h->frame,w->epoch,_gz_pbac_epoch,w->session,w->session);
    if(relation){
     if(found++)return RTV_CHANGED;btr_copy(&out->process,&process,sizeof(process));
     out->identity.magic=0x52524931u;out->identity.relation=relation;
     out->identity.source_pid=source->process.pid;out->identity.source_generation=source->process.generation;
     out->identity.source_frame=w->frame;out->identity.current_frame=h->frame;
     out->identity.source_epoch=w->epoch;out->identity.current_epoch=_gz_pbac_epoch;
     out->identity.actor=a;out->identity.pid=process.pid;out->identity.generation=process.generation;
     out->identity.seal=rri_hash(&out->identity);
    }
   }
  }
  q=next;
 }
 if(found!=1||head!=_gz_pba_native_actor_queue||!_gz_nsv_heap_scope(h->token,h->frame)||!rri_source_context(h,&w))return RTV_CHANGED;
 h->reserved[0]=index;h->reserved[1]=out->identity.relation;h->reserved[2]=out->identity.source_pid;h->reserved[3]=out->process.pid;
 return 0;
}

#include "rito_child_relation.c"
static u32 rri_edge_word(const RtvRow*r,u32 i){return R32((u32)r->controllers[i].pad,0);}
static u32 rri_edge_hash(const RtvRow*r){u32 i,h=r->identity.seal;for(i=0;i<4;i++)h=(h^rri_edge_word(r,i))*16777619u;return h;}
static u32 rri_child_match(const PoEntry*e,u32 saved,u32 current){RtvHeader*h=rtv_header();RtvRow*r;PoEntry child;u32 i,found=0,a;
 if(saved==current)return 1;
 if(!rri_context(h))return 0;
 for(i=0;i<h->count;i++){
  r=rtv_row(h,i);if(!btr_equal(&r->process,e,sizeof(*e))||R32((u32)r->pad,0)!=rri_edge_hash(r)||
   rri_edge_word(r,0)!=saved||rri_edge_word(r,1)!=current)continue;
  a=rri_edge_word(r,2);
  if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return 0;
  btr_copy(&child,&_gz_po_observation.entry,sizeof(child));
  if(child.object!=a||child.pid!=current||child.generation!=rri_edge_word(r,3)||child.profile!=426||child.process_name!=426||
   child.state!=PO_LIVE||child.flags||child.end_frame||child.allocation_bytes!=0xd74u||
   child.base_bytes+child.other_bytes<child.base_bytes||child.base_bytes+child.other_bytes!=0xd74u||
   child.profile_entry!=_gz_po_profiles[426]||!epj_ram(a,0xd74)||R32(a,4)!=child.pid||R16(a,8)!=426||R16(a,14)!=426||
   R8(a,12)!=2||R32(a,16)!=child.profile_entry)return 0;
  found++;
 }
 return found==1;
}
static u32 rri_prepare_child(RtvHeader*h,RtvRow*out,const RtbRow*source){
 const VrWorld*w;const VrActor*saved=0;const BbBank*birth;const PoEntry*old=0;PoEntry child;VrActor live;
 u32 saved_pid=R32((u32)source->core,92),current_pid=R32(out->process.object,0x978),i,q,next,a,left=4096,found=0,relation,head;
 if(saved_pid==current_pid)return 0;
 if(!_gz_nsv_heap_scope(h->token,h->frame)||!rri_source_context(h,&w))return RTV_CONTEXT;
 birth=(const BbBank*)(h->source+R32(h->source,192));
 if(birth->count!=w->count)return RTV_CHANGED;
 for(i=0;i<w->count;i++)if(w->actors[i].pid==saved_pid){if(saved)return RTV_CHANGED;saved=w->actors+i;old=&birth->rows[i].process;}
 if(!saved||!old||saved->profile!=426||old->object!=saved->actor||old->pid!=saved_pid||old->profile!=426)return RTV_CHANGED;
 head=q=_gz_pba_native_actor_queue;
 while(q){
  if(!left--||!epj_ram(q,16))return RTV_CHANGED;next=R32(q,8);a=R32(q,12);
  if(a){
   if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return RTV_CHANGED;
   btr_copy(&child,&_gz_po_observation.entry,sizeof(child));
   if(child.profile==426){
    if(child.object!=a||child.state!=PO_LIVE||child.flags||child.end_frame||!child.generation||child.process_name!=426||
     child.profile_entry!=_gz_po_profiles[426]||child.allocation_bytes!=0xd74u||
     child.base_bytes+child.other_bytes<child.base_bytes||child.base_bytes+child.other_bytes!=0xd74u||!epj_ram(a,0xd74)||
     R32(a,4)!=child.pid||R16(a,8)!=426||R16(a,14)!=426||R8(a,12)!=2||R32(a,16)!=child.profile_entry)return RTV_CHANGED;
    live.actor=a;live.pid=child.pid;live.profile=426;live.parameters=R32(a,0xb0);live.set_id=R16(a,0x2d8);
    live.parent_pid=R32(a,0x2e8);live.init_state=2;for(i=0;i<5;i++)live.home[i]=R32(a,0x2ec+4*i);
    relation=_gz_rid_relation(old,&child,saved,&live,w->frame,h->frame,w->epoch,_gz_pbac_epoch);
    if(relation){
     if(found++||child.pid!=current_pid)return RTV_CHANGED;
     *(u32*)out->controllers[0].pad=saved_pid;*(u32*)out->controllers[1].pad=current_pid;
     *(u32*)out->controllers[2].pad=a;*(u32*)out->controllers[3].pad=child.generation;
     *(u32*)out->pad=rri_edge_hash(out);
    }
   }
  }
  q=next;
 }
 if(found!=1||head!=_gz_pba_native_actor_queue||!_gz_nsv_heap_scope(h->token,h->frame)||!rri_child_match(&out->process,saved_pid,current_pid))return RTV_CHANGED;
 h->reserved[6]=saved_pid;h->reserved[7]=current_pid;h->reserved[8]=rri_edge_word(out,3);return 0;
}

#include "rito_transaction.c"
static u32 rtv_archive_same(const ErnResource*a,const ErnResource*b){return a->control==b->control&&a->node==b->node&&a->info==b->info&&a->table==b->table&&
 a->count==b->count&&a->container==b->container&&a->header==b->header&&a->header_bytes==b->header_bytes;}
static u32 rtv_prepare_row(RtvHeader*h,RtvRow*out,const RtbRow*source,u32 index){
 PoEntry again;GhIdentity end_heap;HlHeap last;ErnResource archive;EpjBounds b;EptResources r;u32 a,key,name,m,z,role,target,model,found,i;
 if(!_gz_nsv_heap_scope(h->token,h->frame))return RTV_CONTEXT;
 if(!gz_source_extent(h->source,_gz_osa_capacity_for(h->source))||h->used>_gz_osa_capacity_for(h->source))return RTV_CHANGED;
 z=rri_prepare_current(h,out,source,index);if(z)return z;a=out->process.object;
 out->index=index;out->pid=out->process.pid;out->source_row=(u32)source;out->source_core=(u32)source->core;
 out->source_hash=btr_hash(source,sizeof(*source));out->source_frame=R32((u32)source->core,24);
 z=rtb_family(a,&key,&name);if(z)return z;if(key!=source->archive_key||name!=source->archive_name)return RTV_CHANGED;
 m=R32(a,0x44c);z=krc_heap(0,R32(a,0xf4),m,0xc8);if(z)return z;btr_copy(&out->heap,(const void*)&_gz_gh_identity,sizeof(GhIdentity));
 z=krc_stamp(out->heap.heap,&out->stamp);if(z)return z;
 if(out->stamp.bytes!=out->heap.bytes||out->stamp.parent!=out->heap.parent||out->stamp.vtable!=out->heap.vtable)return RTV_CHANGED;
 out->archive.profile=326;out->archive.key=key;z=ern_archive_issued(&out->archive,name,h->source,_gz_osa_capacity_for(h->source));if(z)return z;
 if(out->archive.count!=source->resource_count||!_gz_krc_private_apart(out->archive.header,out->archive.header_bytes)||!_gz_krc_private_apart(out->archive.table,out->archive.count*4))return RTV_CHANGED;
 for(role=1;role<=4;role++){
  h->reserved[4]=role;h->reserved[5]=out->roles;
  RtvRole*p=out->controllers+role-1;u32 packet=(u32)source->controllers[role-1];
  m=R32(a,rbs_member(role));z=krc_heap(&out->heap,out->heap.heap,m,0xc8);if(z)return z;model=R32(m,0x90);
  z=krc_heap(&out->heap,out->heap.heap,model,0xb0);if(z)return z;
  eks_zero(&b,sizeof(b));b.actor=a;b.pid=out->pid;b.actor_bytes=0xa7c;b.morph=m;b.heap=out->heap.heap;b.head=out->heap.head;b.tail=out->heap.tail;
  b.end=out->heap.heap+out->heap.bytes;b.model=model;b.source=h->source;b.source_bytes=_gz_osa_capacity_for(h->source);
  r.header=out->archive.header;r.bytes=out->archive.header_bytes;r.table=out->archive.table;r.count=out->archive.count;r.model_wrapper=R32(model,0xac);
  if(!epj_ram(r.model_wrapper,0x3c)||!_gz_krc_private_apart(r.model_wrapper,0x3c))return RTV_CHANGED;
  found=0;for(i=0;i<r.count;i++)if(R32(r.table,i*4)==r.model_wrapper)found++;if(found!=1)return RTV_CHANGED;r.model=R32(r.model_wrapper,0);
  if(R32(packet,48)>=r.count)return RTV_CHANGED;target=R32(r.table,R32(packet,48)*4);
  z=_gz_kbj_prepare(&b,&r,R32(m,0x94),target,&p->body);if(z)return z;
  z=_gz_krj_prepare(&p->body,&p->render);if(z)return z;
  z=_gz_kbt_contract(&p->body,&p->binding);if(z)return z;
  z=_gz_rbp_prepare(&p->body,packet,role,h->frame,&p->numeric);if(z)return z;
  z=_gz_rrp_prepare(&p->body,&p->render,&p->numeric,(u32)source->results[role-1],&p->result);if(z)return z;
  z=_gz_kbj_before_matches(&p->body);if(z)return z;z=_gz_krj_before_matches(&p->body,&p->render);if(z)return z;
  out->roles++;
 }
 h->reserved[4]=9;z=rri_prepare_child(h,out,source);if(z)return z;
 h->reserved[4]=5;h->reserved[5]=out->roles;z=_gz_rbc_prepare(&out->controllers[0].body,(u32)source->core,h->frame,&out->core);if(z)return z;
 z=_gz_rbc_before_matches(&out->controllers[0].body,&out->core);if(z)return z;
 h->reserved[4]=6;h->reserved[5]=out->roles;z=_gz_rmj_prepare(&out->controllers[0].body,(u32)source->models,h->frame,&out->models);if(z)return z;
 z=_gz_rmj_before_matches(&out->controllers[0].body,&out->models);if(z)return z;
 h->reserved[4]=7;h->reserved[5]=out->roles;z=_gz_rtj_prepare(&out->controllers[0].body,(u32)source->texture,h->frame,&out->texture);if(z)return z;
 z=_gz_rtj_before_matches(&out->controllers[0].body,&out->texture);if(z)return z;
 h->reserved[4]=8;h->reserved[5]=out->roles;z=_gz_rpj_prepare(&out->controllers[0].body,(u32)source->physics,h->frame,&out->physics);if(z)return z;
 z=_gz_rpj_before_matches(&out->controllers[0].body,&out->physics);if(z)return z;
 z=krc_stamp(out->heap.heap,&last);if(z)return z;if(!btr_equal(&out->stamp,&last,sizeof(last)))return RTV_CHANGED;
 z=krc_heap(&out->heap,out->heap.heap,R32(a,0x44c),0xc8);if(z)return z;btr_copy(&end_heap,(const void*)&_gz_gh_identity,sizeof(end_heap));
 if(!btr_equal(&out->heap,&end_heap,sizeof(end_heap)))return RTV_CHANGED;
 eks_zero(&archive,sizeof(archive));archive.profile=326;archive.key=key;z=ern_archive_issued(&archive,name,h->source,_gz_osa_capacity_for(h->source));if(z)return z;if(!rtv_archive_same(&out->archive,&archive))return RTV_CHANGED;
 z=rtb_live(a,&again);if(z)return z;
 if(!btr_equal(&out->process,&again,sizeof(again))||btr_hash(source,sizeof(*source))!=out->source_hash||out->roles!=4||!_gz_nsv_heap_scope(h->token,h->frame))return RTV_CHANGED;
 out->seal=rtv_hash(out);return 0;
}
u32 _gz_rtv_prepare(u32 token,u32 frame){RtvHeader*h=rtv_header();const TxObserveSource*tx;const RtbBank*bank;
 u32 ncb,z=0,i,hash=2166136261u;
 if(!h)return RTV_STORAGE;if(!_gz_nsv_heap_scope(token,frame)||h->entered)return RTV_CONTEXT;
 eks_zero(h,128);h->magic=0x52545631u;h->bytes=128;h->entered=1;h->token=token;h->frame=frame;h->phase=1;h->missing=63;
 tx=tx_observe_source(token,frame);if(!tx){z=RTV_CONTEXT;goto done;}h->source=tx->source;h->used=tx->used;h->pin=tx->pin;
 z=_gz_ncb_archive_validate(h->source,h->used,1);if(z)goto done;
 if(R32(h->source,8)<30){z=RTV_UNRECORDED;goto done;}ncb=h->source+R32(h->source,248);
 if(R32(ncb,8)!=5&&R32(ncb,8)!=6){z=RTV_UNRECORDED;goto done;}
 h->bank=ncb+R32(ncb,64);h->bank_bytes=R32(ncb,68);h->bank_hash=btr_hash((const void*)h->bank,h->bank_bytes);bank=(const RtbBank*)h->bank;
 if(bank->version!=5&&bank->version!=6){z=RTV_UNRECORDED;goto done;}
 z=_gz_rtb_restore_coverage(bank,h->bank_bytes,(const VrWorld*)(h->source+R32(h->source,168)));if(z)goto done;
 h->count=bank->count;if(h->count>RTV_CAP||h->count>rtv_row_capacity(h)){z=RTV_STORAGE;goto done;}h->bytes=128+h->count*RTV_ROW;
 for(i=0;i<h->count;i++){
  RtvRow*r=rtv_row(h,i);eks_zero(r,sizeof(*r));z=rtv_prepare_row(h,r,bank->rows+i,i);if(z)goto done;
  z=_gz_rtx_prepare(r,(RtxRun*)rtv_run_address(h,i));if(z)goto done;
  hash=(hash^r->seal)*16777619u;h->prepared++;
 }
 tx=tx_observe_source(token,frame);
 if(!_gz_nsv_heap_scope(token,frame)||!tx||tx->source!=h->source||tx->used!=h->used||tx->pin!=h->pin||
  btr_hash((const void*)h->bank,h->bank_bytes)!=h->bank_hash||_gz_ncb_archive_validate(h->source,h->used,1)){z=RTV_CHANGED;goto done;}
 h->rows_hash=hash;h->phase=2;__asm__ volatile("sync":::"memory");h->valid=1;
done:h->error=z;h->entered=0;return z;
}
u32 _gz_rtv_storage(u32 part){RtvHeader*h=rtv_header();if(!h)return 0;
 if(part==0)return (u32)h;if(part==1)return (u32)h+128;if(part==2)return rtv_row_capacity(h);if(part==3)return RTV_ROW;if(part==4)return rtv_capacity(h);if(part==6)return (u32)h+128+rtv_row_capacity(h)*RTV_ROW;return 0;}
