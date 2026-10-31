/* Private held-transaction algorithm. Actual game heap constructors and
 * destructors execute through lifecycle.c. Unknown resource destructors are
 * explicit unsupported providers, never silently skipped. Main admission and
 * engine/resource/OS ownership are NOT wired by this module. */
#include "rebuild.h"
#include "lifecycle.h"
#include "forest_snapshot.c"
#ifdef GZ_JOINT_RESTORE_DRIVER
static U32 jr_before_native(const ArDomain*,const void*,U32,const void*,U32);
static U32 jr_after_apply(const AfDomain*,const void*,U32,U32);
static U32 jr_entry_allowed(const ArDomain*);
#endif
extern void _gz_ar_native_dispose(U32,U32,U32);
#ifdef GZ_PRIVATE_NATIVE_PAYLOAD_FAULT
extern void _gz_nhf_after_payload(U32,U32);
#endif
volatile U32 _gz_ar_stage;
typedef struct {const AfDomain*domain;const void*image;ForestHeader header;U32 constructed;} Journal;
static Journal journal;
static const ForestIndex* index_of(const void*p,const ForestHeader*h){return(const ForestIndex*)((const U8*)p+h->index);}
static const AgHeap* graph_of(const void*p){return(const AgHeap*)((const U8*)p+128);}
static const AgObject* objects_of(const void*p,const ForestHeader*h){return(const AgObject*)(graph_of(p)+h->count);}
static U32 unit_recipe(const void*,const ForestHeader*,U32,AlUnitRecipe*);
static U32 has(U32 start,U32 bytes,U32 p,U32 n){return p>=start&&n<=bytes&&p-start<=bytes-n;}
static U32 part_byte(const void*p,const ForestHeader*h,U32 heap,U32 address,U32 bytes,const U8**out){
 const ForestIndex*ix=index_of(p,h);const AsHeader*ph=(const AsHeader*)((const U8*)p+ix[heap].offset);
 const AsRow*rs=(const AsRow*)((const U8*)ph+AS_HEADER);U32 i,begin,end;const AgHeap*gh=graph_of(p)+heap;
 for(i=0;i<ph->blocks;i++){
  const AsRow*r=rs+i;if(!r->kind)continue;
  begin=gh->start+r->offset;end=begin+r->stored;
  if(r->kind==1)begin+=16+R32((const U8*)ph+r->data,12);
  if(begin<=end&&address>=begin&&address<=end&&bytes<=end-address){
   *out=(const U8*)ph+r->data+(address-gh->start-r->offset);return 1;
  }
 }
 return 0;
}
static U32 intrinsic(const AgHeap*hs,U32 n,U32 object,U32 owner,U32 vt){U32 i;
 for(i=1;i<n;i++)if(hs[i].parent==owner){
  if(object==hs[i].heap)return vt==hs[i].vtable;
  if(object==hs[i].heap+0x54)return vt==0x1015e628u;
 }
 return 0;
}
static U32 closure(const AfDomain*d,const void*p,const ForestHeader*h){
 const AgHeap*hs=graph_of(p);const AgObject*os=objects_of(p,h);const ForestIndex*ix=index_of(p,h);U32 i,j,n=R32((const U8*)p+64,28),parent,owner;const U8*data;AlUnitRecipe ur;
 for(i=0;i<h->count;i++){
  if(i&&!_gz_al_policy_supported(hs[i].kind,hs[i].direction,hs[i].flags))return AR_UNSUPPORTED;
  if(hs[i].kind==3&&unit_recipe(p,h,i,&ur))return AR_UNSUPPORTED;
  if(!i)continue;
  for(parent=0;parent<i;parent++)if(hs[parent].heap==hs[i].parent)break;
  if(parent==i||(hs[parent].kind!=1&&hs[parent].kind!=2&&hs[parent].kind!=3))return AR_UNSUPPORTED;
  /* An Exp parent must cover this entire child in one allocated block;
   * a Solid parent must cover it in one committed front/back slab. Never
   * span the Solid free gap. The validated forest independently proves
   * parentage, header separation and non-overlapping sibling ranges.
   * This certifies archive backing only, not native lifetime ownership. */
  if(!part_byte(p,h,parent,hs[i].start,hs[i].span,&data))return AR_LAYOUT;
  for(j=0;j<n;j++)if(os[j].object==hs[i].heap&&os[j].owner==hs[i].parent&&os[j].vtable==hs[i].vtable)break;
  if(j==n)return AR_LAYOUT;
  for(j=0;j<n;j++)if(os[j].object==hs[i].heap+0x54&&os[j].owner==hs[i].parent&&os[j].vtable==0x1015e628u)break;
  if(j==n)return AR_LAYOUT;
 }
 for(i=0;i<n;i++){
  if(intrinsic(hs,h->count,os[i].object,os[i].owner,os[i].vtable))continue;
  if(os[i].vtable!=0x10145950u)return AR_UNSUPPORTED;
  owner=0xffffffffu;
  for(j=0;j<h->count;j++)if(has(hs[j].start,hs[j].span,os[i].object,16))owner=j;
  if(owner==0xffffffffu||!part_byte(p,h,owner,os[i].object,16,&data)||R32(data,12)!=os[i].vtable)return AR_UNSUPPORTED;
  /* Ordinary source objects cannot alias a protected native heap header. */
  for(j=0;j<h->count;j++)if(!apart(os[i].object,16,hs[j].heap,heap_header(hs[j].kind)))return AR_UNSUPPORTED;
 }
 (void)d;(void)ix;return 0;
}
static void recipe(const void*p,const ForestHeader*h,U32 i,AlRecipe*r){const AgHeap*g=graph_of(p)+i;const ForestIndex*ix=index_of(p,h);
 const AsHeader*ph=(const AsHeader*)((const U8*)p+ix[i].offset);
 r->heap=g->heap;r->start=g->start;r->span=g->span;r->parent=g->parent;r->kind=g->kind;r->direction=g->direction;r->flags=g->flags;r->name=ph->reserved[0];
}
/* Recreate an equivalent UnitHeap policy from its validated source geometry.
 * The exact original caller's alignment is not needed: choose a bounded
 * native initializer producing exactly the saved stride and first slot. */
static U32 unit_recipe(const void*p,const ForestHeader*h,U32 i,AlUnitRecipe*u){
 const ForestIndex*ix=index_of(p,h);const AsHeader*ph=(const AsHeader*)((const U8*)p+ix[i].offset);U32 a,e,first;
 zero(u,sizeof(*u));recipe(p,h,i,&u->heap);u->unit_size=ph->mutable_words[0];
 for(a=4;a<=0x10000;a*=2)if(!(u->unit_size%a))for(e=0;e<2;e++){
  first=(u->heap.heap+AS_UNIT_HEADER+a-1)&~(a-1);if(e&&first==u->heap.heap+AS_UNIT_HEADER)first+=a;
  if(first==ph->mutable_words[1]){u->alignment=a;u->extra_gap=e;return 0;}
 }
 return AR_UNSUPPORTED;
}
static U32 identity(const AgHeap*h){return R32(h->heap,12)==h->vtable&&R32(h->heap,0x1c)==h->start&&R32(h->heap,0x20)==h->span&&
 R32(h->heap,0x24)==h->parent&&R32(h->heap,0x50)==h->direction&&R32(h->heap,0x90)==h->flags;}
static void word(U32 p,U32 value){*(volatile U32*)p=value;}
static void node(U32 address,U32 previous,U32 next){word(address,previous);word(address+4,next);}
static U32 publish(const void*p,const ForestHeader*h){const AgHeap*hs=graph_of(p);const AgObject*os=objects_of(p,h);U32 i,j,n=R32((const U8*)p+64,28),head,first,count;
 /* Validate every typed target before the first registry store. */
 for(i=0;i<h->count;i++)if(!identity(hs+i))return AR_CHANGED;
 for(i=0;i<n;i++)if(R32(os[i].object,12)!=os[i].vtable)return AR_CHANGED;
 for(i=0;i<h->count;i++){
  head=hs[i].heap+0x28;first=hs[i].child_first;count=hs[i].child_count;
  node(head,count?hs[first+count-1].heap+0x38:head,count?hs[first].heap+0x38:head);word(head+8,count);word(head+12,0x38);
  for(j=0;j<count;j++)node(hs[first+j].heap+0x38,j?hs[first+j-1].heap+0x38:head,j+1<count?hs[first+j+1].heap+0x38:head);
  head=hs[i].heap+0x40;first=hs[i].object_first;count=hs[i].object_count;
  node(head,count?os[first+count-1].object+4:head,count?os[first].object+4:head);word(head+8,count);word(head+12,4);
  for(j=0;j<count;j++){
   word(os[first+j].object,hs[i].heap);
   node(os[first+j].object+4,j?os[first+j-1].object+4:head,j+1<count?os[first+j+1].object+4:head);
  }
 }
 __sync_synchronize();return 0;
}
/* Repair only the native heap/mutex graph known to have been constructed.
 * Typed base-disposer payloads have no external destructor side effects.
 * This is not a cleanup recipe for arbitrary model/resource objects. */
static U32 repair_intrinsics(void){const AgHeap*hs=graph_of(journal.image);const AgObject*os=objects_of(journal.image,&journal.header);
 U32 i,j,k,head,previous,child,n=journal.constructed,objects=R32((const U8*)journal.image+64,28),count;
 for(i=0;i<n;i++)if(!identity(hs+i))return AR_RECOVERY;
 for(i=0;i<objects;i++)if(!intrinsic(hs,journal.header.count,os[i].object,os[i].owner,os[i].vtable)){
  word(os[i].object,0);word(os[i].object+4,0);word(os[i].object+8,0);
 }
 for(i=0;i<n;i++){
  head=hs[i].heap+0x28;previous=head;count=0;
  for(j=1;j<n;j++)if(hs[j].parent==hs[i].heap){child=hs[j].heap+0x38;word(previous+4,child);word(child,previous);previous=child;count++;}
  word(previous+4,head);word(head,previous);word(head+8,count);word(head+12,0x38);
  head=hs[i].heap+0x40;previous=head;count=0;
  for(j=1;j<n;j++)if(hs[j].parent==hs[i].heap)for(k=0;k<2;k++){
   child=hs[j].heap+(k?0x54:0);word(child,hs[i].heap);word(previous+4,child+4);word(child+4,previous);previous=child+4;count++;
  }
  word(previous+4,head);word(head,previous);word(head+8,count);word(head+12,4);
 }
 __sync_synchronize();return 0;
}
static U32 rebuild(const AfDomain*d,const void*p,const ForestHeader*h){const ForestIndex*ix=index_of(p,h);AsDomain part;AsHeader ph;AlDomain lease;AlRecipe r;AlUnitRecipe ur;U32 i,z;
 z=establish(d,(const U8*)p+64,h->graph_bytes);if(z)return z;
 journal.domain=d;journal.image=p;copy(&journal.header,h,sizeof(*h));journal.constructed=1;
 lease.root=d->graph.root;lease.session=d->graph.session;lease.token=d->graph.token;lease.obligations=d->graph.obligations;
 for(i=0;i<h->count;i++){
  if(i){_gz_ar_stage=5;recipe(p,h,i,&r);
   if(r.kind==3){z=unit_recipe(p,h,i,&ur);if(!z)z=_gz_al_unit_construct(&lease,&ur);}else z=_gz_al_construct(&lease,&r);
   if(z)return AR_RECOVERY;
   journal.constructed=i+1;}
  _gz_ar_stage=4;part_domain(d,i,&part);copy(&ph,(const U8*)p+ix[i].offset,sizeof(ph));apply(&part,(const U8*)p+ix[i].offset,&ph);
#ifdef GZ_PRIVATE_NATIVE_PAYLOAD_FAULT
  _gz_nhf_after_payload(d->graph.root,i);
#endif
#ifdef GZ_JOINT_RESTORE_DRIVER
  z=jr_after_apply(d,p,h->bytes,i);if(z)return z;
#endif
 }
 _gz_ar_stage=6;z=publish(p,h);if(z)return z;
 _gz_ar_stage=7;
 for(i=0;i<h->count;i++){
  part_domain(d,i,&part);copy(&ph,(const U8*)p+ix[i].offset,sizeof(ph));if(!matches(&part,(const U8*)p+ix[i].offset,&ph))return AR_CHANGED;
 }
 return _gz_ag_matches_live(&d->graph,(const U8*)p+64,h->graph_bytes);
}
static U32 metadata(U32 p,U32 n){return _gz_ag_buffer_private(p,n)&&apart(p,n,(U32)rows,sizeof(rows))&&apart(p,n,(U32)&busy,4)&&
 apart(p,n,(U32)&forest_scope,4)&&apart(p,n,(U32)&forest_heaps,4)&&apart(p,n,(U32)&forest_count,4)&&
 apart(p,n,(U32)&forest_current,4)&&apart(p,n,(U32)&forest_busy,4)&&apart(p,n,(U32)&forest_validating,4)&&
 apart(p,n,(U32)&journal,sizeof(journal))&&apart(p,n,(U32)&_gz_ar_stage,4);}
static U32 outside(U32 p,U32 n,const ArDomain*d){return metadata(p,n)&&apart(p,n,(U32)d,sizeof(*d));}
/* Compare the immutable prepared before-image directly against the native
 * hierarchy. This checks every archived allocator word and allocated byte,
 * excluding the same descendant OS/header holes as capture/apply. It is not
 * a substitute for the caller's actual world writer exclusion. */
static U32 prepared_matches(const AfDomain*d,const void*p,const ForestHeader*h){
 AsDomain part;AsHeader ph;const ForestIndex*ix=index_of(p,h);U32 i,z;
 z=establish(d,(const U8*)p+64,h->graph_bytes);if(z)return z;
 z=_gz_ag_matches_live(&d->graph,(const U8*)p+64,h->graph_bytes);if(z)return z;
 for(i=0;i<h->count;i++){
  part_domain(d,i,&part);copy(&ph,(const U8*)p+ix[i].offset,sizeof(ph));
  if(!matches(&part,(const U8*)p+ix[i].offset,&ph))return AR_CHANGED;
 }
 return _gz_ag_matches_live(&d->graph,(const U8*)p+64,h->graph_bytes);
}
static U32 restore_core(const ArDomain*d,const void*source,U32 bytes,void*undo,U32 capacity,ArResult*out,U32 prepared){
 ForestHeader saved,current;ArResult result;U32 z,n,root_words=8,result_ok=0;const AgHeap*sroot,*croot;
 if(!_gz_image_write_allowed((U32)out,sizeof(*out))||(!prepared&&!_gz_image_write_allowed((U32)undo,capacity)))return AR_ARGUMENT;
#ifdef GZ_JOINT_RESTORE_DRIVER
 if(!jr_entry_allowed(d))return AR_BUSY;
#endif
 if(!d||!metadata((U32)d,sizeof(*d))||!outside((U32)source,bytes,d)||!outside((U32)undo,capacity,d)||!outside((U32)out,sizeof(*out),d)||
  !apart((U32)out,sizeof(*out),(U32)source,bytes)||!apart((U32)out,sizeof(*out),(U32)undo,capacity)||!apart((U32)source,bytes,(U32)undo,capacity))return AR_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&forest_busy,0,1))return AR_BUSY;
 zero(&result,sizeof(result));_gz_ar_stage=0;zero(&journal,sizeof(journal));
 if(d->source.graph.root!=d->current.graph.root||d->source.graph.parent!=d->current.graph.parent||d->source.graph.session!=d->current.graph.session||
  d->source.graph.token!=d->current.graph.token||!d->target_epoch||!d->rollback_epoch||d->target_epoch==d->rollback_epoch||
  d->target_epoch==d->current.generation||d->rollback_epoch==d->current.generation||
  d->target_epoch==d->source.generation||d->rollback_epoch==d->source.generation){z=AR_SCOPE;goto done;}
 _gz_ar_stage=1;z=forest_validate_locked(&d->source,source,bytes,&saved);if(z)goto done;
 z=closure(&d->source,source,&saved);if(z)goto done;
 if(!forest_private((U32)undo,capacity,&d->source)||!forest_private((U32)out,sizeof(*out),&d->source)||
  !apart((U32)d,sizeof(*d),forest_heaps[0].start,forest_heaps[0].span)){z=AR_ARGUMENT;goto done;}
 result_ok=1;
 _gz_ar_stage=2;
 if(prepared){
  n=capacity;z=forest_validate_locked(&d->current,undo,n,&current);if(z)goto done;
  z=prepared_matches(&d->current,undo,&current);if(z)goto done;
 }else{z=forest_capture_locked(&d->current,undo,capacity,&n);if(z)goto done;}
 result.undo_bytes=n;copy(&current,undo,64);z=closure(&d->current,undo,&current);if(z)goto done;
 sroot=graph_of(source);croot=graph_of(undo);
 if(!equal(sroot,croot,root_words*4)){z=AR_SCOPE;goto done;}
 z=forest_validate_locked(&d->source,source,bytes,&saved);if(z)goto done;
 if(prepared){
  /* Source validation may be lengthy. Revalidate the prepared data and all
   * current bytes again at the final pre-mutation boundary. */
  z=forest_validate_locked(&d->current,undo,n,&current);if(z)goto done;
  z=prepared_matches(&d->current,undo,&current);if(z)goto done;
 }else{z=_gz_ag_matches_live(&d->current.graph,(const U8*)undo+64,current.graph_bytes);if(z)goto done;}
#ifdef GZ_JOINT_RESTORE_DRIVER
 z=jr_before_native(d,source,bytes,undo,n);if(z)goto done;
#endif
 /* First native mutation. Every later return requires owner finalization or
  * held recovery; this component never publishes a lifetime epoch/releases. */
 result.mutation_started=1;result.owner_action=AR_OWNER_RECOVER;_gz_ar_stage=3;
 _gz_ar_native_dispose(d->current.graph.root,0,0);
 if(R32(d->current.graph.root,0x30)||R32(d->current.graph.root,0x48)){z=AR_RECOVERY;goto done;}
#ifdef GZ_JOINT_OBSERVATION_FIXTURE
 _gz_test_aj_disposed(graph_of(undo),current.count);
#endif
 z=rebuild(&d->source,source,&saved);
 if(!z){result.owner_action=AR_OWNER_FINALIZE;result.pending_epoch=d->target_epoch;result.heaps=saved.count;result.objects=R32((const U8*)source+64,28);goto done;}
 if(z==AR_RECOVERY)goto done;
 _gz_ar_stage=8;z=repair_intrinsics();if(z)goto done;
 _gz_ar_native_dispose(d->source.graph.root,0,0);
 if(R32(d->source.graph.root,0x30)||R32(d->source.graph.root,0x48)){z=AR_RECOVERY;goto done;}
#ifdef GZ_JOINT_OBSERVATION_FIXTURE
 _gz_test_aj_disposed(graph_of(journal.image),journal.constructed);
#endif
 z=rebuild(&d->current,undo,&current);
 if(z){z=AR_RECOVERY;goto done;}
 result.owner_action=AR_OWNER_FINALIZE;result.pending_epoch=d->rollback_epoch;result.heaps=current.count;result.objects=R32((const U8*)undo+64,28);z=AR_ROLLED_BACK;
done:result.code=z;result.stage=_gz_ar_stage;
 /* An invalid result pointer cannot be written. All accepted pointers were
  * checked against both caller metadata and the complete owned native arena. */
 if(result_ok&&forest_private((U32)out,sizeof(*out),&d->source))copy(out,&result,sizeof(result));
 release();return z;
}
AgU32 _gz_ar_restore(const ArDomain*d,const void*source,U32 bytes,void*undo,U32 capacity,ArResult*out){
 return restore_core(d,source,bytes,undo,capacity,out,0);
}
AgU32 _gz_ar_restore_prepared(const ArDomain*d,const void*source,U32 bytes,const void*undo,U32 undo_bytes,ArResult*out){
 return restore_core(d,source,bytes,(void*)undo,undo_bytes,out,1);
}

#ifdef GZ_JOINT_RESTORE_DRIVER
/* Resume only a fully constructed, published undo graph whose final payload
 * verification failed. Constructor/observer failures need additional typed
 * repair and remain held. Never capture a new before-image from dirty memory.
 * The retained joint driver authenticates exact original buffers and caller,
 * current metadata stamp, thread, native root identity and this journal.
 * Whole-world exclusion and global reconciliation remain the outer owner job. */
static U32 recover_undo(const ArDomain*d,const void*undo,U32 bytes,ArResult*out,U32*native_started){
 ForestHeader h;ArResult r;U32 z=AR_RECOVERY;
 zero(&r,sizeof(r));r.mutation_started=1;r.owner_action=AR_OWNER_RECOVER;r.undo_bytes=bytes;
 if(!jr_entry_allowed(d)||!metadata((U32)d,sizeof(*d))||!outside((U32)undo,bytes,d)||
  !outside((U32)out,sizeof(*out),d)||!apart((U32)out,sizeof(*out),(U32)undo,bytes)||
  !_gz_image_write_allowed((U32)out,sizeof(*out)))return AR_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&forest_busy,0,1))return AR_BUSY;
 z=forest_validate_locked(&d->current,undo,bytes,&h);if(z)goto done;
 z=closure(&d->current,undo,&h);if(z)goto done;
 if(_gz_ar_stage!=7||journal.domain!=&d->current||journal.image!=undo||journal.constructed!=h.count||
  !equal(&journal.header,&h,sizeof(h))){z=AR_UNSUPPORTED;goto done;}
 /* Prove the actual native graph is the complete known undo graph before
  * calling any destructor. Payload equality is deliberately not required:
  * restoring that payload is the purpose of this recovery operation. */
 z=_gz_ag_matches_live(&d->current.graph,(const U8*)undo+64,h.graph_bytes);if(z)goto done;
 *native_started=1;_gz_ar_stage=9;_gz_ar_native_dispose(d->current.graph.root,0,0);
 if(R32(d->current.graph.root,0x30)||R32(d->current.graph.root,0x48)){z=AR_RECOVERY;goto done;}
#ifdef GZ_JOINT_OBSERVATION_FIXTURE
 _gz_test_aj_disposed(graph_of(undo),h.count);
#endif
 z=rebuild(&d->current,undo,&h);if(z)goto done;
 r.owner_action=AR_OWNER_FINALIZE;r.pending_epoch=d->rollback_epoch;r.heaps=h.count;r.objects=R32((const U8*)undo+64,28);z=AR_ROLLED_BACK;
done:r.code=z;r.stage=_gz_ar_stage;
 if(forest_private((U32)out,sizeof(*out),&d->current))copy(out,&r,sizeof(r));
 release();return z;
}
#endif
