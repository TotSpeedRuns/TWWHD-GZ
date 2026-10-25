/* Bind saved logical BG owners to their independently rediscovered current
 * placements. Historical object/resource addresses are equality audit keys,
 * never dereference inputs. Resource equivalence remains deliberately exact:
 * relocated or changed geometry needs a separate content/resource contract.
 */
#include "background_owner_relation.c"
static u32 rri_bg_world(RtvHeader*h,const VrWorld**w,const BbBank**b){const TxObserveSource*tx;u32 o,n;
 if(!rri_context(h)||!h->entered||!_gz_nsv_heap_scope(h->token,h->frame))return 0;
 tx=tx_observe_source(h->token,h->frame);
 if(!tx||tx->source!=h->source||tx->used!=h->used||tx->pin!=h->pin||_gz_bcb_archive_validate(h->source,h->used,0))return 0;
 o=R32(h->source,168);n=R32(h->source,172);
 if(n!=sizeof(VrWorld)||o>h->used||n>h->used-o)return 0;*w=(const VrWorld*)(h->source+o);
 o=R32(h->source,192);if(o>h->used||BB_HEADER>h->used-o)return 0;*b=(const BbBank*)(h->source+o);
 return (*b)->count==(*w)->count&&(*b)->process_session==_gz_po_header.session&&(*w)->epoch&&(*w)->epoch<=_gz_pbac_epoch;
}
static u32 rri_bg_owner(RtvHeader*h,u32 pid,u32 generation,PoEntry*out){const VrWorld*w;const BbBank*b;const VrActor*old=0;const PoEntry*po=0;
 VrActor live;PoEntry now,last;u32 i,q,next,head,left=4096,a,found=0;
 if(!rri_bg_world(h,&w,&b))return RPH_CHANGED;
 for(i=0;i<w->count;i++)if(w->actors[i].pid==pid){if(old)return RPH_CHANGED;old=w->actors+i;po=&b->rows[i].process;}
 if(!old||!po||po->object!=old->actor||po->pid!=pid||po->generation!=generation)return RPH_CHANGED;
 head=q=_gz_pba_native_actor_queue;
 while(q){
  if(!left--||!epj_ram(q,16))return RPH_CHANGED;next=R32(q,8);a=R32(q,12);
  if(a){
   if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return RPH_CHANGED;btr_copy(&now,&_gz_po_observation.entry,sizeof(now));
   if(now.profile==po->profile){
    if(now.object!=a||now.profile>=484||now.profile_entry!=_gz_po_profiles[now.profile]||now.state!=PO_LIVE||now.flags||now.end_frame||
     now.allocation_bytes<0x300||!epj_ram(a,now.allocation_bytes)||R32(a,4)!=now.pid||R16(a,8)!=now.profile||R16(a,14)!=now.process_name||
     R32(a,16)!=now.profile_entry||R8(a,12)!=2)return RPH_CHANGED;
    live.actor=a;live.pid=now.pid;live.profile=now.profile;live.parameters=R32(a,0xb0);live.set_id=R16(a,0x2d8);
    live.parent_pid=R32(a,0x2e8);live.init_state=2;for(i=0;i<5;i++)live.home[i]=R32(a,0x2ec+4*i);
    if(_gz_bgo_relation(po,&now,old,&live,w->frame,h->frame,w->epoch,_gz_pbac_epoch)){if(found++)return RPH_CHANGED;btr_copy(out,&now,sizeof(now));}
   }
  }q=next;
 }
 if(found!=1||head!=_gz_pba_native_actor_queue||!rri_bg_world(h,&w,&b)||_gz_po_lookup(out->object)||!_gz_po_observation.valid)return RPH_CHANGED;
 btr_copy(&last,&_gz_po_observation.entry,sizeof(last));return btr_equal(out,&last,sizeof(last))?0:RPH_CHANGED;
}
static u32 rri_bg_current(RtvHeader*h,u32 pid,u32 owner_generation,u32 signature,u32 old_generation,const RwmMeta*wall,BgEntry*out){PoEntry owner;
 BgEntry bg,last;u32 i,found=0,z=rri_bg_owner(h,pid,owner_generation,&owner);if(z)return z;
 if(_gz_bg_header.flags||_gz_bg_header.inflight||_gz_bg_header.busy)return RPH_CHANGED;
 for(i=0;i<256;i++){
  if(_gz_bg_rows[i].state!=BG_LIVE||_gz_bg_rows[i].owner!=owner.object||_gz_bg_rows[i].pid!=owner.pid)continue;
  if(_gz_bg_lookup(i)||!_gz_bg_observation.valid)return RPH_CHANGED;btr_copy(&bg,&_gz_bg_observation.entry,sizeof(bg));
  if(bg.owner_generation!=owner.generation||bg.resource_signature!=signature||bg.generation<old_generation)continue;
  if(wall&&(bg.bgw_vtable!=wall->vtable||bg.resource!=wall->resource||bg.epoch<wall->epoch||!epj_ram(bg.resource,0x30)||R32(bg.resource,8)!=wall->triangles))continue;
  if(found++)return RPH_CHANGED;btr_copy(out,&bg,sizeof(bg));
 }
 if(found!=1||_gz_bg_lookup(out->index)||!_gz_bg_observation.valid)return RPH_CHANGED;
 btr_copy(&last,&_gz_bg_observation.entry,sizeof(last));return btr_equal(out,&last,sizeof(last))?0:RPH_CHANGED;
}
static u32 rri_physics_target(const PoEntry*process,u32 source,u32 target,u32 frame){RtvHeader*h=rtv_header();RtvRow*r;BgEntry bg;RwmMeta*m;
 u32 i,found=0,w,z;
 if(!rph_join(process,source,1))return 0;
 if(R32(source,8)!=2||!rri_context(h)||h->frame!=frame||!h->entered)return RPH_CHANGED;
 for(i=0;i<h->count;i++){r=rtv_row(h,i);if(btr_equal(process,&r->process,sizeof(*process))&&source==(u32)((const RtbRow*)r->source_row)->physics)found++;}
 if(found!=1||rph_ref(source,0x458)!=R32(source,16)||rph_ref(source,0x52c)!=R32(source,16))return RPH_CHANGED;
 rph_set_ref(target,0x458,process->pid);rph_set_ref(target,0x52c,process->pid);
 for(i=0;i<4;i++){
  w=R32(source,RPH_META+16*i);if(w==0xffff0100u)continue;
  z=rri_bg_current(h,rph_ref(source,nph_poly[i]+8),R32(source,RPH_META+16*i+8),R32(source,RPH_META+16*i+12),R32(source,RPH_META+16*i+4),0,&bg);if(z)return z;
  if(!epj_ram(bg.resource,0x30)||(w>>16)>=R32(bg.resource,8))return RPH_CHANGED;
  *(u32*)(target+RPH_META+16*i)=(w&0xffff0000u)|bg.index;*(u32*)(target+RPH_META+16*i+4)=bg.generation;
  *(u32*)(target+RPH_META+16*i+8)=bg.owner_generation;*(u32*)(target+RPH_META+16*i+12)=bg.resource_signature;
  rph_set_ref(target,nph_poly[i]+4,bg.object);rph_set_ref(target,nph_poly[i]+8,bg.pid);
 }
 m=(RwmMeta*)(target+RPH_USED);
 if(rph_ref(source,0x4c4)||rph_ref(source,0x4c8)||rph_ref(source,0x4cc)){
  z=rri_bg_current(h,rph_ref(source,0x4cc),m->owner_generation,m->signature,m->generation,m,&bg);if(z)return z;
  rph_set_ref(target,0x4c4,bg.index);rph_set_ref(target,0x4c8,bg.object);rph_set_ref(target,0x4cc,bg.pid);
  m->generation=bg.generation;m->owner_generation=bg.owner_generation;m->epoch=bg.epoch;
 }
 *(u32*)(target+68)=btr_hash((const void*)(target+RPH_HEADER),4*nph_ref_count());*(u32*)(target+80)=rph_hash(target);
 return rph_bound_join(process,target);
}
