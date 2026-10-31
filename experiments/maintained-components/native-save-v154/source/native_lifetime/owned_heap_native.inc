/* Real WHL-owned native reader. Reuses GHD's one parent/membership census,
 * GPA's real SDK birth match, GAR3's current backing/scope containment, and
 * arena_snapshot's native header/mutable-control policy. AS v3 is untouched.
 * Every native body read below occurs after real HG/GPA and whole-range
 * exclusions. Missing protection/coherence flags are never cleared. */
#include "owned_heap_native.h"
#include "allocated_run_policy.h"
#include "owned_heap_process.h"
#include "owned_heap_common_domain.h"
#include "opaque_chunks.h"
#include "owned_heap_held_view.h"
#include "held_heap_roundtrip.h"
#define GHC_FN static __attribute__((noinline,section(".text.gz_ghc")))
#define GHC_API __attribute__((noinline,section(".text.gz_ghc")))
GHC_FN u32 ghp_measure(u32*);
extern u32 ghc_hl_healthy(void) __asm__("obt_healthy");
extern u32 ghc_hl_lock(void) __asm__("_gz_hl_meta_try_lock");
extern void ghc_hl_unlock(void) __asm__("_gz_hl_meta_unlock");
extern HlHeap*ghc_hl_heap(u32,ObTryWork*) __asm__("obt_heap");
extern HlStoredAllocation*ghc_hl_generation(u32,u32,ObTryWork*) __asm__("obt_generation");
extern u32 ghc_hl_scope(HlHeap*,u32,u32*,ObTryWork*) __asm__("obt_scope");
extern u32 ghc_hl_member(u32,u32,u32) __asm__("hi_member");
extern u32 _gz_rsa_private_apart(u32,u32),_gz_osa_private_apart(u32,u32);
extern u32 _gz_krc_private_apart(u32,u32),_gz_cgp_private(u32,u32);
extern u32 _gz_gpn_native_private_apart(u32,u32);
extern const unsigned char _gz_rsa_image_begin[],_gz_rsa_image_last[];
typedef struct {u32 active,request,frame,raw,bytes,capacity;GhdLabels labels;
 HlHeap root;HmScope scope;u32 purpose,process_capacity,allocation_capacity;} GhcProposal;
typedef char GhcProposal148[sizeof(GhcProposal)==148?1:-1];
static GhcProposal ghc_proposal __attribute__((section(".bss.gz_ghc")));
volatile u32 _gz_ghc_report[32] __attribute__((section(".data.gz_ghc")))={GHC_MAGIC,128,GHC_VERSION,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,GHC_MISSING_ALL};
/* Purpose dispatch is fixed to the actual WCT producer or the distinct native
 * Save producer. No archive/request field can select an ownership provider. */
GHC_FN u32 ghc_native_scope(u32 req,u32 f,u32 phase){return
 ghc_proposal.purpose==GHC_VERSION?_gz_wct_ghc_scope(req,f,phase):
 ghc_proposal.purpose==GHS_VERSION?_gz_ghs_scope(req,f,phase):0;}
GHC_FN u32 ghc_storage_scope(u32 req,u32 f,u32 phase){return
 ghc_proposal.purpose==GHC_VERSION?_gz_gac_heap_scope(req,f,phase):
 ghc_proposal.purpose==GHS_VERSION?_gz_ghs_scope(req,f,phase):0;}
GHC_FN u32 ghc_actual_hg(u32 req,u32 f){return
 ghc_proposal.purpose==GHC_VERSION?_gz_wct_ghc_hg(req,f):
 ghc_proposal.purpose==GHS_VERSION?_gz_ghs_hg(req,f):0;}
GHC_FN u32 ghc_apart(u32 p,u32 n,u32 q,u32 m){return ghc_policy_apart(p,n,q,m);}
GHC_FN u32 ghc_contains(u32 p,u32 n,u32 q,u32 m){return ghc_policy_contains(p,n,q,m);}
GHC_FN void ghc_copy(void*d,const void*s,u32 n){u32 i;for(i=0;i<n;i++)((unsigned char*)d)[i]=((const unsigned char*)s)[i];}
GHC_FN u32 ghc_equal(const void*a,const void*b,u32 n){u32 i;for(i=0;i<n;i++)if(((const unsigned char*)a)[i]!=((const unsigned char*)b)[i])return 0;return 1;}
GHC_API u32 _gz_ghc_private_apart(u32 p,u32 n){return
 ghc_apart(p,n,(u32)&ghc_proposal,sizeof(ghc_proposal))&&
 ghc_apart(p,n,(u32)_gz_ghc_report,sizeof(_gz_ghc_report));}
GHC_API u32 _gz_ghc_all_private_apart(u32 p,u32 n){return _gz_ghc_private_apart(p,n)&&_gz_ghw_private_apart(p,n)&&
 _gz_wct_ghc_private_apart(p,n)&&_gz_gac_heap_private_apart(p,n)&&_gz_ois_heap_private_apart(p,n)&&
 _gz_ghs_private_apart(p,n)&&_gz_jwa_ghs_private_apart(p,n);}
GHC_FN u32 ghc_table(u32 kind){return kind==HL_EXP?(u32)_gz_hl_exp_table:kind==HL_SOLID?(u32)_gz_hl_solid_table:
 kind==HL_UNIT?(u32)_gz_hl_unit_table:kind==HL_CUSTOM?(u32)_gz_hl_custom_table:0;}
GHC_FN u32 ghc_bounds(const HlHeap*h,u32*p,u32*n){u32 end,head=header_bytes(h->kind),begin;
 if(h->state!=HL_LIVE||!h->generation||h->kind<HL_EXP||h->kind>HL_CUSTOM||
  (h->flags&~HL_REBORN)||h->end_frame||h->dtor_site||h->vtable!=ghc_table(h->kind)||!head||!span(h->heap,head)||
  !span(h->start,h->bytes)||(h->bytes&3))return 0;
 begin=h->heap<h->start?h->heap:h->start;end=h->start+h->bytes;
 if(h->heap+head>end)end=h->heap+head;
 if(end<=begin||!span(begin,end-begin))return 0;*p=begin;*n=end-begin;return 1;
}
GHC_FN u32 ghc_common_propose_locked(void);
GHC_FN HlHeap*ghc_common_live_root(const GhdLabels*,ObTryWork*);
GHC_FN u32 ghc_common_marks(const GhdCut*,u32,u32,u32,ObTryWork*);
GHC_FN u32 ghc_common_allocations(const GhdCut*,u32,u32,u32,GhcAllocationDirectory*,u32,u32,u32*);
GHC_FN u32 ghc_common_format(const GhcHeader*,const GhdCut*,u32);
extern u32 _gz_ois_heap_preallocate_apart(u32,u32,u32,u32);
GHC_FN u32 ghc_same_root(void){ObTryWork w={GHD_WORK,0};HlHeap*h=ghc_proposal.purpose==GHS_VERSION?ghc_common_live_root(&ghc_proposal.labels,&w):ghc_hl_heap(ghc_proposal.root.heap,&w);
 return h&&!w.exhausted&&ghc_hl_healthy()&&
 ghc_equal(h,&ghc_proposal.root,sizeof(*h))&&
 ghc_equal(_gz_hm_scopes+(h-_gz_hl_heaps),&ghc_proposal.scope,sizeof(HmScope));}
GHC_FN u32 ghc_preallocate_apart(u32 req,u32 f,u32 p,u32 n){u32 a=(u32)_gz_rsa_image_begin,b=(u32)_gz_rsa_image_last;
 return a>=0x10000000u&&b>=a&&b<0xbffe0000u&&ghc_apart(p,n,a,b-a+1)&&
 _gz_ghc_all_private_apart(p,n)&&_gz_gpn_native_private_apart(p,n)&&
 _gz_rsa_private_apart(p,n)&&_gz_osa_private_apart(p,n)&&_gz_krc_private_apart(p,n)&&
 _gz_cgp_private(p,n)&&_gz_hr_buffer_private((const void*)p,n)&&_gz_ois_heap_preallocate_apart(req,f,p,n);
}
GHC_FN u32 ghc_propose_kind(u32 req,u32 f,u32 purpose){ObTryWork w={GHD_WORK,0};HlHeap*h;u32 z=0;
 if(ghc_proposal.active||(purpose==GHC_VERSION?!_gz_wct_ghc_scope(req,f,1):
  purpose==GHS_VERSION?!_gz_ghs_scope(req,f,1):1))return 0;
 ghc_proposal.purpose=purpose;_gz_ghc_report[2]=purpose==GHS_VERSION?GHC_PROCESS_VERSION:purpose;
 {u32 i;for(i=3;i<32;i++)_gz_ghc_report[i]=0;_gz_ghc_report[22]=GHC_MISSING_ALL;}
 ghc_proposal.process_capacity=0;ghc_proposal.allocation_capacity=0;
 if(purpose==GHS_VERSION&&!ghp_measure(&ghc_proposal.process_capacity))return 0;
 ghc_proposal.labels=ghd_resident_labels();if(!ghc_proposal.labels.game0||
  !ghc_proposal.labels.game1||!ghc_proposal.labels.zelda||!ghc_hl_lock())return 0;
 if(!ghc_hl_healthy())goto done;
 if(purpose==GHS_VERSION){if(!ghc_common_propose_locked())goto done;goto selected;}
 h=ghc_hl_heap(ghc_proposal.labels.game1,&w);
 if(!h||w.exhausted||!ghc_bounds(h,&ghc_proposal.raw,&ghc_proposal.bytes)||
  !ghc_policy_capacity(ghc_proposal.bytes))goto done;
 ghc_copy(&ghc_proposal.root,h,sizeof(*h));
 ghc_copy(&ghc_proposal.scope,_gz_hm_scopes+(h-_gz_hl_heaps),sizeof(HmScope));
 if(ghc_proposal.scope.flags||ghc_proposal.scope.active_token||ghc_proposal.scope.reserved)goto done;
selected:
 ghc_proposal.capacity=purpose==GHS_VERSION?ghc_common_capacity(ghc_proposal.bytes,ghc_proposal.process_capacity,ghc_proposal.allocation_capacity):
  ghc_policy_capacity(ghc_proposal.bytes);if(!ghc_proposal.capacity)goto done;ghc_proposal.request=req;
 ghc_proposal.frame=f;ghc_proposal.active=1;z=ghc_proposal.capacity;
done:ghc_hl_unlock();
 if(z&&purpose==GHS_VERSION&&!ghc_preallocate_apart(req,f,ghc_proposal.raw,ghc_proposal.bytes)){ghc_proposal.active=0;z=0;}
 _gz_ghc_report[3]=1;_gz_ghc_report[4]=req;
 _gz_ghc_report[5]=f;_gz_ghc_report[6]=ghc_proposal.raw;_gz_ghc_report[7]=ghc_proposal.bytes;
 _gz_ghc_report[10]=z;return z;
}
GHC_API u32 _gz_ghc_propose(u32 req,u32 f){return ghc_propose_kind(req,f,GHC_VERSION);}
GHC_API u32 _gz_ghs_heap_propose(u32 req,u32 f){return ghc_propose_kind(req,f,GHS_VERSION);}
GHC_API u32 _gz_ghc_refresh(u32 req,u32 f){GhdLabels labels;u32 same;
 if(!ghc_proposal.active||ghc_proposal.request!=req||ghc_proposal.frame!=f||
  !ghc_native_scope(req,f,1))return GHC_CONTEXT;
 labels=ghd_resident_labels();if(!ghd_same_labels(&labels,&ghc_proposal.labels)||!ghc_hl_lock())return GHC_CHANGED;
 same=ghc_same_root();ghc_hl_unlock();return same?0:GHC_CHANGED;
}
GHC_FN u32 ghc_known_apart(u32 req,u32 f,u32 p,u32 n){u32 a=(u32)_gz_rsa_image_begin,b=(u32)_gz_rsa_image_last;
 return a>=0x10000000u&&b>=a&&b<0xbffe0000u&&
 ghc_apart(p,n,a,b-a+1)&&_gz_ghc_all_private_apart(p,n)&&_gz_gpn_native_private_apart(p,n)&&
 _gz_rsa_private_apart(p,n)&&_gz_osa_private_apart(p,n)&&
 _gz_krc_private_apart(p,n)&&_gz_cgp_private(p,n)&&
 _gz_hr_buffer_private((const void*)p,n)&&_gz_ois_heap_apart(req,f,p,n);
}
/* GAR3's backing join, with this consumer's genuine WHL HG and its closed
 * sameGate. No JOL token substitution, GPR stub or caller flag is used. */
GHC_FN u32 ghc_rows(const GhdCut*cut,GhcHeapRow*rows,u32*count,u32 raw,u32 bytes){u32 i,n=0,depth,common=ghc_proposal.purpose==GHS_VERSION,selected;ObTryWork w={GHD_WORK,0};GarRootSpan sdk={0};
 for(i=0;i<cut->nodes;i++)if(!cut->node[i].parent_node){sdk.heap=cut->node[i].heap.heap;
  sdk.generation=cut->node[i].heap.generation;sdk.start=cut->node[i].heap.start;sdk.bytes=cut->node[i].heap.bytes;}
 selected=common?ghc_common_node(cut,&ghc_proposal.labels):0;
 if(common&&(!selected||!ghc_common_marks(cut,selected,raw,bytes,&w)))return 0;
 for(i=0;i<HL_HEAPS;i++)if(ghd_scratch.mask[i]&(common?GHC_ENROLLED_HEAP:GHC_MASK)){
  HlHeap*h=_gz_hl_heaps+i,*parent,*header_owner;HmScope*s=_gz_hm_scopes+i;
  HlStoredAllocation*a,*b;GhcHeapRow*r=rows+n;u32 head=header_bytes(h->kind);
  if(n==HL_HEAPS||!gz_gar_heap_policy(h,&sdk,ghc_table(h->kind),head)||
   !ghc_contains(raw,bytes,h->heap,head)||!ghc_contains(raw,bytes,h->start,h->bytes)||
   s->flags||s->active_token||s->reserved)return 0;
  depth=128;if(!ghc_hl_scope(h,0,&depth,&w)||w.exhausted)return 0;
  if(h->parent){a=ghc_hl_generation(s->managed_pointer,s->managed_generation,&w);
   b=ghc_hl_generation(s->header_pointer,s->header_generation,&w);
   parent=ghc_hl_heap(h->parent,&w);header_owner=b?ghc_hl_heap(b->heap,&w):0;
   if(!a||!b||!parent||!header_owner||w.exhausted||
    !gz_gar_heap_policy(parent,&sdk,ghc_table(parent->kind),header_bytes(parent->kind))||
    !gz_gar_heap_policy(header_owner,&sdk,ghc_table(header_owner->kind),header_bytes(header_owner->kind))||
    !gz_gar_allocation_policy(a,parent)||!gz_gar_allocation_policy(b,header_owner)||
    parent->state!=HL_LIVE||parent->generation!=h->parent_generation||parent->epoch!=s->parent_epoch||
    a->state!=HL_LIVE||b->state!=HL_LIVE||a->release_token||b->release_token||
    a->reserved||b->reserved||a->heap!=parent->heap||a->heap_generation!=parent->generation||
    b->heap_generation!=header_owner->generation||a->heap_epoch!=parent->epoch||b->heap_epoch!=header_owner->epoch||
    !ghc_hl_member((u32)(a-_gz_hl_allocations)+1,(u32)(parent-_gz_hl_heaps)+1,parent->generation)||
    !ghc_hl_member((u32)(b-_gz_hl_allocations)+1,(u32)(header_owner-_gz_hl_heaps)+1,header_owner->generation)||
    !ghc_contains(a->pointer,a->capacity,h->start,h->bytes)||
    !ghc_contains(b->pointer,b->capacity,h->heap,head))return 0;
   depth=128;if(!ghc_hl_scope(header_owner,0,&depth,&w)||w.exhausted)return 0;
  }
  /* Complete chosen domain includes out-of-line custom allocator controls
   * only when they lie in the same physical root. No silent omission. */
  if(h->kind==HL_CUSTOM){HlCustomMeta*c=_gz_hl_custom_meta+i;
   if(c->heap_generation!=h->generation||!c->slots||c->slots>0xffffffffu/16||
    !ghc_contains(raw,bytes,c->table,c->slots*16))return 0;}
  ghc_copy(r->heap,h,64);ghc_copy(r->scope,s,32);r->raw_offset=h->heap-raw;
  r->raw_bytes=head;r->mutable_offset=0x94;
  r->mutable_count=h->kind==HL_EXP?9:h->kind==HL_SOLID?2:h->kind==HL_UNIT?6:0;
  r->mask=(ghd_scratch.mask[i]&7u)|(common?GHC_ENROLLED_HEAP:0u);r->slot=i+1;r->header_bytes=head;r->reserved=common&&h->heap!=ghc_proposal.root.heap?ghd_scratch.parent[i]:0;++n;
 }
 *count=n;return n&&!w.exhausted;
}
GHC_FN u32 ghc_gpa(u32 hg,const GhdCut*c){u32 i,matched=0;
 if(c->flags&~2u||c->unknown_chain_heaps||c->unknown_chain_indexed||!c->nodes||c->nodes>GHD_NODES)return 0;
 for(i=0;i<c->nodes;i++)if(!c->node[i].parent_node){const HlHeap*h=&c->node[i].heap;
  if(h->parent||_gz_gpa_try_match(hg,h->heap,h->generation,h->start,h->bytes)!=GPA_MATCH)return 0;++matched;}
 return matched==1; /* Current GPA owns one genuine original SDK arena. */
}
GHC_FN u32 ghc_hash(const void*p,u32 n){const unsigned char*b=p;u32 i,h=2166136261u;
 for(i=0;i<n;i++)h=(h^((i>=12&&i<16)||(i>=108&&i<112)?0:b[i]))*16777619u;return h;}
GHC_FN u32 ghc_plan_hash(const void*p,u32 n){const unsigned char*b=p;u32 i,h=2166136261u;
 for(i=0;i<n;i++)h=(h^b[i])*16777619u;return h;}
#include "owned_heap_process.inc"
#include "owned_heap_common_domain.inc"
GHC_FN u32 ghc_packed_header(const GhcHeader*h,const GhcHeapRow*r){
 u32 body[37];
 if(r->raw_bytes<sizeof(body)||oc_read((const unsigned char*)h+h->data,h->data_bytes,
  h->domain.span,r->raw_offset,body,sizeof(body))!=OC_OK)return 0;
 return body[3]==r->heap[8]&&body[7]==r->heap[4]&&body[8]==r->heap[5]&&
  body[9]==r->heap[6]&&body[20]==r->heap[9]&&!(body[36]&2u);
}
GHC_API u32 _gz_ghc_capture(u32 req,u32 f,u32 p,u32 capacity,u32*written){
 GhcHeader*h=(GhcHeader*)p;GhcSource*source=(GhcSource*)(p+128);GhdLabels*labels=(GhdLabels*)(p+192);
 GhdCut*c=(GhdCut*)(p+208);GhcHeapRow*rows=(GhcHeapRow*)(p+3536);
 GhcProcessDirectory*process=0;GhcAllocationDirectory*allocations=0;
 u32 hg,n=0,i,z=GHC_OBSERVER,at,process_at=0,limit=capacity,scratch_at=0,scratch_bytes=0,allocation_count=0,allocation_at=0,selected=0,checked=0,packed=0,raw=ghc_proposal.raw,bytes=ghc_proposal.bytes;
 if(!ghc_proposal.active||ghc_proposal.request!=req||ghc_proposal.frame!=f||
  !ghc_storage_scope(req,f,2)||!_gz_ois_heap_stage_match(req,f,p,capacity)||
  capacity<(ghc_proposal.purpose==GHS_VERSION?ghc_proposal.capacity-ghc_proposal.bytes+64u+((ghc_proposal.bytes-1u)/OC_ENCODE_PAGE+1u)*32u:ghc_proposal.capacity)||!_gz_image_write_allowed((u32)written,4))return GHC_CONTEXT;
 _gz_ghc_report[23]=1;
 *written=0;hg=ghc_actual_hg(req,f);if(!hg)return GHC_CONTEXT;
 _gz_ghc_report[23]=2;
 if(!ghc_hl_lock())return GHC_OBSERVER;i=ghc_same_root();ghc_hl_unlock();if(!i)return GHC_CHANGED;
 /* The WHOLE destination capacity + all existing private SDK ranges are
  * excluded from the entire raw union. Overlap rejects; no hidden holes. */
 if(!ghc_known_apart(req,f,raw,bytes))return GHC_OVERLAP;
 h->valid=0;ghc_copy(labels,&ghc_proposal.labels,16);
 _gz_ghc_report[23]=3;
 i=_gz_ob_try_actor_heap_cut(labels,c);_gz_ghc_report[24]=i;if(i!=OB_OK)return GHC_OBSERVER;
 _gz_ghc_report[23]=4;i=ghc_gpa(hg,c);_gz_ghc_report[25]=i;if(!i)return GHC_OBSERVER;
 selected=ghc_proposal.purpose==GHS_VERSION?ghc_common_node(c,labels):c->root[1].node;
 if(!selected||selected>c->nodes||!ghc_equal(&c->node[selected-1].heap,&ghc_proposal.root,64))return GHC_CHANGED;
 if(!ghc_hl_lock())return GHC_OBSERVER;
 if(!ghc_hl_healthy()||_gz_hl_control.serial!=c->serial||_gz_hl_control.release_serial!=c->release_serial||
  !ghc_equal(c,&ghd_scratch.cut,sizeof(*c))||!ghc_same_root())goto unlock;
 _gz_ghc_report[23]=6;
 if(!ghc_rows(c,rows,&n,raw,bytes))goto unlock;
 if(ghc_proposal.purpose==GHS_VERSION){_gz_ghc_report[23]=7;z=ghc_common_allocations(c,selected,raw,bytes,0,ghc_proposal.allocation_capacity,0,&allocation_count);_gz_ghc_report[26]=z;if(z)goto unlock;}
 z=0;
unlock:ghc_hl_unlock();if(z)return z;
 at=ghc_policy_data(n);
 if(ghc_proposal.purpose==GHS_VERSION){
  scratch_bytes=ghc_batch_bytes(ghc_proposal.process_capacity);
  if(!scratch_bytes||(capacity&31u)||scratch_bytes>capacity)return GHC_SPACE;
  scratch_at=capacity-scratch_bytes;limit=scratch_at;
  process_at=ghc_process_offset(n);
  if(!process_at||process_at>limit||GHC_PROCESS_HEADER>limit-process_at)return GHC_SPACE;
  process=(GhcProcessDirectory*)(p+process_at);
  if(ghc_proposal.process_capacity>(limit-process_at-GHC_PROCESS_HEADER)/GHC_PROCESS_ROW)return GHC_SPACE;
  z=ghp_begin(process,ghc_proposal.process_capacity);if(z)return z;
  allocation_at=ghc_allocation_offset(n,process->count);at=ghc_common_data(n,process->count,allocation_count);
  if(!allocation_at||allocation_at>limit||sizeof(*allocations)>limit-allocation_at)return GHC_SPACE;
  allocations=(GhcAllocationDirectory*)(p+allocation_at);
 }
 if(!at||at>limit||!bytes||(!process&&bytes>limit-at)||(process&&limit-at<64u+((bytes-1u)/OC_ENCODE_PAGE+1u)*32u))return GHC_SPACE;
 for(i=0;i<128;i++)((unsigned char*)h)[i]=0;
 h->magic=GHC_MAGIC;h->version=ghc_proposal.purpose==GHS_VERSION?GHC_PROCESS_VERSION:ghc_proposal.purpose;h->bytes=at+bytes;h->domain.heap=ghc_proposal.root.heap;
 h->domain.start=raw;h->domain.span=bytes;h->domain.heap_generation=ghc_proposal.root.generation;
 h->domain.token=hg; /* Actual HG, with obligations/session deliberately0. */
 h->diagnostic=1;h->missing=GHC_MISSING_ALL;h->request=req;h->frame=f;h->hg=hg;
 h->cut=208;h->table=3536;h->count=n;h->data=at;h->data_bytes=bytes;h->root_mask=process?GHC_COMMON_ROOT_MASK:GHC_MASK;
 if(process){h->reserved[0]=process_at;h->reserved[1]=process->bytes;}
 if(ghc_proposal.purpose==GHC_VERSION){
  z=_gz_wct_ghc_source(req,f,source);if(z)return z;
  h->lease=source->lease;h->parent=source->parent;h->thread=source->thread;
 }else{GhcSaveProducer*producer=(GhcSaveProducer*)source;
  z=_gz_ghs_source(req,f,producer);if(z)return z;
  h->lease=producer->lease;h->parent=producer->original_return;h->thread=producer->thread;
  if(process){z=_gz_ghs_process_scene(req,f,&process->scene);if(z)return z;}
 }
 /* Verify fresh authority immediately before the first opaque read. */
 if(!ghc_native_scope(req,f,2)||ghc_actual_hg(req,f)!=hg||!ghc_gpa(hg,c)||
  !ghc_known_apart(req,f,raw,bytes))return GHC_CHANGED;
 if(process){z=ghp_join(process,c,rows,n,raw,bytes,(void*)(p+scratch_at),scratch_bytes,ghc_proposal.process_capacity);if(z)return z;
  if(!ghc_native_scope(req,f,2)||ghc_actual_hg(req,f)!=hg||!ghc_gpa(hg,c)||
   !ghc_known_apart(req,f,raw,bytes))return GHC_CHANGED;}
 if(allocations){
  if(!ghc_hl_lock())return GHC_OBSERVER;
  z=ghc_same_root()?ghc_common_allocations(c,selected,raw,bytes,allocations,allocation_count,1,&checked):GHC_CHANGED;
  ghc_hl_unlock();if(z||checked!=allocation_count)return z?z:GHC_CHANGED;
  if(!ghc_native_scope(req,f,2)||ghc_actual_hg(req,f)!=hg||!ghc_gpa(hg,c)||!ghc_known_apart(req,f,raw,bytes))return GHC_CHANGED;
 }
 if(process){
  allocations->bank_offset=at-WGC_BYTES;allocations->bank_bytes=WGC_BYTES;
  z=_gz_wgc_capture(req,f,h);if(z)return GHC_CHANGED;
  if(!ghc_native_scope(req,f,2)||ghc_actual_hg(req,f)!=hg||!ghc_gpa(hg,c)||!ghc_known_apart(req,f,raw,bytes))return GHC_CHANGED;
  z=oc_encode((const void*)raw,bytes,(void*)(p+at),limit-at,&packed);
  if(z!=OC_OK)return z==OC_SPACE?GHC_SPACE:GHC_CHANGED;
  h->data_bytes=packed;h->bytes=at+packed;
  if(oc_matches((const void*)(p+at),packed,bytes,(const void*)raw)!=OC_OK)return GHC_CHANGED;
 }else{
  for(i=0;i<bytes;i++)((unsigned char*)(p+at))[i]=((volatile const unsigned char*)raw)[i];
  /* Equal second observation remains diagnostic, never coherence. */
  for(i=0;i<bytes;i++)if(((unsigned char*)(p+at))[i]!=((volatile const unsigned char*)raw)[i])return GHC_CHANGED;
 }
 if(!ghc_native_scope(req,f,2)||ghc_actual_hg(req,f)!=hg||!ghc_gpa(hg,c)||
  !ghc_known_apart(req,f,raw,bytes))return GHC_CHANGED;
 if(!ghc_hl_lock())return GHC_OBSERVER;
 i=ghc_same_root()&&ghc_hl_healthy()&&_gz_hl_control.serial==c->serial&&_gz_hl_control.release_serial==c->release_serial;
 ghc_hl_unlock();if(!i)return GHC_CHANGED;
 if(process){z=ghp_end(process,c,(void*)(p+scratch_at),scratch_bytes);if(z)return z;
  if(!ghc_native_scope(req,f,2)||ghc_actual_hg(req,f)!=hg||!ghc_gpa(hg,c)||
   !ghc_known_apart(req,f,raw,bytes))return GHC_CHANGED;}
 if(allocations){
  if(!ghc_hl_lock())return GHC_OBSERVER;
  z=ghc_same_root()?ghc_common_allocations(c,selected,raw,bytes,allocations,allocation_count,2,&checked):GHC_CHANGED;
  ghc_hl_unlock();if(z||checked!=allocation_count)return z?z:GHC_CHANGED;
  if(!ghc_native_scope(req,f,2)||ghc_actual_hg(req,f)!=hg||!ghc_gpa(hg,c)||!ghc_known_apart(req,f,raw,bytes))return GHC_CHANGED;
 }
 if(process){
  if(allocations->bank_offset!=at-WGC_BYTES||allocations->bank_bytes!=WGC_BYTES||
   _gz_wgc_verify_held(req,f,h))return GHC_CHANGED;
  if(!ghc_native_scope(req,f,2)||ghc_actual_hg(req,f)!=hg||!ghc_gpa(hg,c)||!ghc_known_apart(req,f,raw,bytes))return GHC_CHANGED;
 }
 /* Scratch lies beyond the complete wire image and is never published as
  * directory/raw payload. Clear all private indices/cache before sealing. */
 if(process)zero((void*)(p+scratch_at),scratch_bytes);
 /* Check copied native headers from PRIVATE bytes, no pointer follows. */
 for(i=0;i<n;i++){const GhcHeapRow*r=rows+i;const u32*body=(const u32*)(p+at+r->raw_offset);
  if(process){if(!ghc_packed_header(h,r))return GHC_CHANGED;}
  else if(body[3]!=r->heap[8]||body[7]!=r->heap[4]||body[8]!=r->heap[5]||
   body[9]!=r->heap[6]||body[20]!=r->heap[9]||(body[36]&2))return GHC_CHANGED;}
 /* Zero directory alignment padding deterministically. */
 for(i=3536+n*sizeof(GhcHeapRow);i<(process?process_at:at);i++)*(unsigned char*)(p+i)=0;
 if(process)for(i=process_at+process->bytes;i<allocation_at;i++)*(unsigned char*)(p+i)=0;
 if(allocations)for(i=allocation_at+allocations->bytes;i<allocations->bank_offset;i++)*(unsigned char*)(p+i)=0;
 h->plan_checksum=ghc_plan_hash((const void*)(p+128),at-128);h->checksum=ghc_hash(h,h->bytes);
 z=_gz_ghc_validate(h,h->bytes,0);if(z)return z;*written=h->bytes;
 _gz_ghc_report[3]=2;_gz_ghc_report[11]=hg;_gz_ghc_report[12]=h->lease;
 _gz_ghc_report[13]=n;_gz_ghc_report[14]=bytes;_gz_ghc_report[15]=h->bytes;
 _gz_ghc_report[16]=c->serial;_gz_ghc_report[17]=c->release_serial;
 _gz_ghc_report[18]=c->flags;_gz_ghc_report[19]=c->selected_heaps;
 _gz_ghc_report[20]=c->selected_allocations;_gz_ghc_report[21]=h->checksum;return 0;
}
GHC_API u32 _gz_ghc_validate(const void*image,u32 bytes,u32 valid){const GhcHeader*h=image;
 const GhcSource*s;const GhdCut*c;const GhcHeapRow*r;u32 i,end,raw,span_bytes,root_rows=0;const GhdNode*root_node;GhdLabels image_labels;u32 common,process_image,selected,packed;
 if(!image||bytes<GHC_FIXED||!_gz_abi_metadata_span((u32)image,bytes))return GHC_FORMAT;
 packed=h->version==GHC_PROCESS_VERSION||h->version==GHC_LEGACY_PACKED_VERSION;common=packed||h->version==GHC_LEGACY_COMMON_VERSION;process_image=common||h->version==GHC_LEGACY_PROCESS_VERSION;
 if(h->magic!=GHC_MAGIC||
  (h->version!=GHC_VERSION&&h->version!=GHS_VERSION&&h->version!=GHC_LEGACY_PROCESS_VERSION&&h->version!=GHC_LEGACY_COMMON_VERSION&&h->version!=GHC_LEGACY_PACKED_VERSION&&h->version!=GHC_PROCESS_VERSION)||h->bytes!=bytes||h->valid!=valid||valid>1||h->diagnostic!=1||
  h->missing!=GHC_MISSING_ALL||h->root_mask!=(common?GHC_COMMON_ROOT_MASK:GHC_MASK)||
  (!process_image&&(h->reserved[0]||h->reserved[1]))||
  (process_image&&(!h->reserved[0]||h->reserved[1]<GHC_PROCESS_HEADER))||
  !h->request||!h->frame||!h->hg||h->domain.token!=h->hg||h->domain.session||h->domain.obligations||
  h->domain.reserved[0]||h->domain.reserved[1]||h->domain.reserved[2]||!h->domain.heap_generation||
  h->cut!=208||h->table!=3536||!h->count||h->count>HL_HEAPS||
  (!process_image&&h->data!=ghc_policy_data(h->count))||
  (process_image&&(h->data<ghc_process_data(h->count,0)||(h->data&31u)))||h->data>bytes||
  h->data_bytes!=bytes-h->data||(!packed&&h->domain.span!=h->data_bytes)||!span(h->domain.start,h->domain.span))return GHC_FORMAT;
 s=(const GhcSource*)((const unsigned char*)image+128);c=(const GhdCut*)((const unsigned char*)image+h->cut);
 if((h->version==GHC_VERSION&&(s->request!=h->request||s->frame!=h->frame||s->hg!=h->hg||s->lease!=h->lease||
  s->parent!=h->parent||s->thread!=h->thread||!s->roh_token||!s->source||!s->used||!s->pin||
  !s->identity||!s->generation||!s->lease||!s->parent||!s->thread||s->slot>9||s->reserved))||
  ((h->version==GHS_VERSION||process_image)&&!ghs_provenance_format((const GhcSaveProducer*)s,h->request,h->frame,
   h->hg,h->lease,h->parent,h->thread))||
  c->magic!=0x47484431u||c->bytes!=sizeof(GhdCut)||c->version!=1||!c->nodes||c->nodes>GHD_NODES||
  c->flags&~2u||c->missing!=31||c->session||c->unknown_chain_heaps||c->unknown_chain_indexed||
  !c->root[1].node||c->root[1].node>c->nodes)return GHC_FORMAT;
 ghc_copy(&image_labels,(const unsigned char*)image+192,16);
 selected=common?ghc_common_node(c,&image_labels):c->root[1].node;
 if(!selected||selected>c->nodes)return GHC_FORMAT;
 root_node=c->node+selected-1;
 if(h->domain.heap!=root_node->heap.heap||h->domain.heap_generation!=root_node->heap.generation||
  !ghc_bounds(&root_node->heap,&raw,&span_bytes)||h->domain.start!=raw||h->domain.span!=span_bytes||
  (!common&&h->count!=c->root[1].heaps)||c->selected_heaps>HL_HEAPS||c->selected_allocations>HL_ALLOCS||
  !c->serial||c->flags!=2)return GHC_FORMAT;
 ghc_copy(&image_labels,(const unsigned char*)image+192,16);
 if(image_labels.reserved)return GHC_FORMAT;
 for(i=0;i<3;i++){u32 label=((const u32*)&image_labels)[i];
  if(!label||c->root[i].label!=label||!c->root[i].node||c->root[i].node>c->nodes||
   c->node[c->root[i].node-1].heap.heap!=label||c->root[i].world||c->root[i].read)return GHC_FORMAT;}
 for(i=0;i<c->nodes;i++){const GhdNode*node=c->node+i;u32 j,depth=i+1,seen=0;
  if(!node->slot||node->slot>HL_HEAPS||node->parent_node>c->nodes||node->parent_node==i+1||
   node->heap.state!=HL_LIVE||node->heap.kind!=HL_EXP||!node->heap.generation||
   !node->root_mask||node->root_mask&~7u)return GHC_FORMAT;
  for(j=0;j<i;j++)if(c->node[j].slot==node->slot)return GHC_FORMAT;
  while(depth){if(depth>c->nodes||(seen&(1u<<(depth-1))))return GHC_FORMAT;seen|=1u<<(depth-1);
   depth=c->node[depth-1].parent_node;}
  if(node->parent_node){const HlHeap*parent=&c->node[node->parent_node-1].heap;
   if(node->heap.parent!=parent->heap||node->heap.parent_generation!=parent->generation)return GHC_FORMAT;
  }else if(node->heap.parent||node->heap.parent_generation)return GHC_FORMAT;
 }
 r=(const GhcHeapRow*)((const unsigned char*)image+h->table);end=0;
 for(i=0;i<h->count;i++){
  if(!r[i].slot||r[i].slot>HL_HEAPS||r[i].slot<=end||!(r[i].mask&(common?GHC_ENROLLED_HEAP:GHC_MASK))||r[i].mask&~(common?15u:7u)||(!common&&r[i].reserved)||
   r[i].heap[2]!=HL_LIVE||!r[i].heap[1]||r[i].heap[3]<HL_EXP||r[i].heap[3]>HL_CUSTOM||
   (r[i].heap[14]&~HL_REBORN)||r[i].heap[11]||r[i].heap[13]||r[i].heap[8]!=ghc_table(r[i].heap[3])||
   r[i].header_bytes!=header_bytes(r[i].heap[3])||r[i].raw_bytes!=r[i].header_bytes||
   !r[i].header_bytes||r[i].raw_offset>h->domain.span||r[i].raw_bytes>h->domain.span-r[i].raw_offset||
   r[i].heap[0]<h->domain.start||r[i].raw_offset!=r[i].heap[0]-h->domain.start||
   r[i].mutable_offset!=0x94||r[i].mutable_count!=(r[i].heap[3]==HL_EXP?9:r[i].heap[3]==HL_SOLID?2:r[i].heap[3]==HL_UNIT?6:0)||
   r[i].scope[5]||r[i].scope[6]||r[i].scope[7]||
   !ghc_contains(h->domain.start,h->domain.span,r[i].heap[4],r[i].heap[5]))return GHC_FORMAT;
  if(r[i].slot==root_node->slot){if(!ghc_equal(r[i].heap,&root_node->heap,64)||
   !ghc_equal(r[i].scope,&root_node->scope,32))return GHC_FORMAT;++root_rows;}
  end=r[i].slot;
 }
 if(root_rows!=1)return GHC_FORMAT;
 if(process_image){if(!ghp_format(h)||(common&&!ghc_common_format(h,c,selected)))return GHC_FORMAT;}
 else for(i=h->table+h->count*GHC_ROW;i<h->data;i++)if(((const unsigned char*)image)[i])return GHC_FORMAT;
 if(packed){
  if(oc_validate((const unsigned char*)image+h->data,h->data_bytes,h->domain.span)!=OC_OK)return GHC_FORMAT;
  for(i=0;i<h->count;i++)if(!ghc_packed_header(h,r+i))return GHC_FORMAT;
 }
 if(h->plan_checksum!=ghc_plan_hash((const unsigned char*)image+128,h->data-128)||
  h->checksum!=ghc_hash(image,bytes))return GHC_FORMAT;return 0;
}
GHC_API u32 _gz_ghc_release(u32 req,u32 f){GhdLabels labels;u32 z=0;
 if(!ghc_proposal.active)return _gz_wct_ghc_scope(req,f,3)||_gz_ghs_scope(req,f,3)?0:GHC_RETAINED;
 if(!ghc_native_scope(req,f,3))return GHC_RETAINED;
 if(ghc_proposal.request!=req||ghc_proposal.frame!=f)return GHC_RETAINED;
 /* Three opaque resident root labels, sampled outside physical holds.
  * No heap body, method, child or allocation pointer is followed. */
 labels=ghd_resident_labels();if(!ghd_same_labels(&labels,&ghc_proposal.labels))z=GHC_CHANGED;
 ghc_proposal.active=0;_gz_ghc_report[3]=3;_gz_ghc_report[30]=z;return z;
}
#include "owned_heap_held_domain.inc"
#undef GHC_FN
#undef GHC_API
