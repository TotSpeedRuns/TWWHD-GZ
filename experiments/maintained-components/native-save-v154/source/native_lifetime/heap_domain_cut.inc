/* APPEND-ONLY SOURCE PROPOSAL over the sealed f8f5 lifetime TU. No existing
 * observer function/capacity/flag is changed. No native memory is followed.
 * Only selected heap lists are walked; unrelated application rows are not
 * materialized or revalidated as a pretend selected-domain certificate.
 */
#include "heap_domain_cut.h"
#define GHD_FN static __attribute__((noinline,section(".text.gz_ghd")))
#define GHD_API __attribute__((noinline,section(".text.gz_ghd")))
/* Distinct C identifiers keep new calls out of the old static IPA graph.
 * Each assembler label is the existing same-TU helper, with its exact ABI.
 * objcopy applies the unchanged local-symbol rename after assembly. No new
 * helper definition, native callback, answer substitution or policy exists. */
extern u32 ghd_original_span(u32,u32) __asm__("span");
extern u32 ghd_original_within(u32,u32,u32,u32) __asm__("within");
extern u32 ghd_original_hi_member(u32,u32,u32) __asm__("hi_member");
extern u32 ghd_original_hr_private(u32,u32,const HlHeap*) __asm__("hr_private");
extern u32 ghd_original_obt_take(ObTryWork*) __asm__("obt_take");
extern HlHeap* ghd_original_obt_heap(u32,ObTryWork*) __asm__("obt_heap");
extern HlStoredAllocation* ghd_original_obt_generation(u32,u32,ObTryWork*) __asm__("obt_generation");
extern u32 ghd_original_obt_healthy(void) __asm__("obt_healthy");
extern u32 ghd_original_obd_reserve(ObTryWork*,u32) __asm__("obd_reserve");

typedef struct { GhdCut cut; unsigned short parent[HL_HEAPS]; unsigned char mask[HL_HEAPS]; } GhdScratch;
static GhdScratch ghd_scratch __attribute__((section(".bss.gz_ghd")));
typedef char GhdScratch15616[sizeof(GhdScratch)==15616?1:-1];
enum { GHD_KNOWN=128,GHD_UNKNOWN=64,GHD_MASK=7,GHD_BAD_PARENT=65535 };

GHD_FN u32 ghd_span(u32 p,u32 n) {
 return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;
}
GHD_FN u32 ghd_private(u32 input,u32 output,ObTryWork*w) {
 u32 i;HlHeap*h;
 if(!ghd_original_hr_private(input,16,0)||!ghd_original_hr_private(output,sizeof(GhdCut),0)||
    !hr_apart(input,16,(u32)&ghd_scratch,sizeof(ghd_scratch))||
    !hr_apart(output,sizeof(GhdCut),(u32)&ghd_scratch,sizeof(ghd_scratch)))return 0;
 for(i=0;i<HL_HEAPS;i++) {
  if(!ghd_original_obt_take(w))return 0;h=_gz_hl_heaps+i;
  if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)) {
   if(!hr_apart(input,16,h->start,h->bytes)||!hr_apart(input,16,h->heap,header_bytes(h->kind))||
      !hr_apart(output,sizeof(GhdCut),h->start,h->bytes)||!hr_apart(output,sizeof(GhdCut),h->heap,header_bytes(h->kind)))return 0;
  }
 }
 return 1;
}
GHD_FN void ghd_words(void*d,const void*s,u32 n) {
 u32 i;for(i=0;i<n/4;i++)((u32*)d)[i]=((const u32*)s)[i];
}
GHD_FN void ghd_add(u32*lo,u32*hi,u32 value) {
 u32 next=*lo+value;*hi+=next<*lo;*lo=next;
}

/* Build parent slot identities once for the4096-slot private directory.
 * Unknown unrelated chains remain explicitly unknown, never silently outside
 * the selected domains. Root/ancestor chains are independently required below.
 */
GHD_FN u32 ghd_parents(ObTryWork*w) {
 u32 i;HlHeap*h,*p;HmScope*s;
 for(i=0;i<HL_HEAPS;i++) {
  if(!ghd_original_obt_take(w))return 0;h=_gz_hl_heaps+i;
  if(!h->heap||h->state==HL_DEAD)continue;
  if(h->state!=HL_LIVE) {ghd_scratch.parent[i]=GHD_BAD_PARENT;continue;}
  if(!h->parent)continue;
  s=_gz_hm_scopes+i;p=ghd_original_obt_heap(h->parent,w);
  if(!p||p->state!=HL_LIVE||!h->parent_generation||p->generation!=h->parent_generation||
     s->parent_epoch!=p->epoch||s->flags||s->active_token||s->reserved)ghd_scratch.parent[i]=GHD_BAD_PARENT;
  else ghd_scratch.parent[i]=(unsigned short)((p-_gz_hl_heaps)+1);
 }
 return 1;
}
GHD_FN u32 ghd_membership(u32 slot,ObTryWork*w) {
 u32 path[GHD_DEPTH],n=0,i=slot,mask=0,j;
 while(1) {
  if(!ghd_original_obt_take(w))return 0;
  if(ghd_scratch.mask[i]&GHD_KNOWN) {mask=ghd_scratch.mask[i];break;}
  if(n==GHD_DEPTH) {mask=GHD_UNKNOWN;break;}
  for(j=0;j<n;j++) {if(!ghd_original_obt_take(w))return 0;if(path[j]==i) {mask=GHD_UNKNOWN;goto unwind;}}
  path[n++]=i;
  if(ghd_scratch.parent[i]==GHD_BAD_PARENT) {mask=GHD_UNKNOWN;break;}
  if(!ghd_scratch.parent[i])break;
  i=ghd_scratch.parent[i]-1;
 }
unwind:
 while(n) {
  if(!ghd_original_obt_take(w))return 0;
  i=path[--n];mask|=ghd_scratch.mask[i]&GHD_MASK;
  ghd_scratch.mask[i]=(unsigned char)(GHD_KNOWN|mask);
 }
 return 1;
}

/* Only three root chains, deduplicated, max16 nodes. This is a conservative
 * common Exp-heap proposal: separate header backing or deeper ancestry refuses.
 * Hash lookups occur per root/ancestor, never per allocation in the census.
 */
GHD_FN u32 ghd_node(u32 slot,ObTryWork*w) {
 u32 i;GhdNode*n;HlHeap*h=_gz_hl_heaps+slot;HmScope*s=_gz_hm_scopes+slot;
 for(i=0;i<ghd_scratch.cut.nodes;i++) {
  if(!ghd_original_obt_take(w))return 0;
  if(ghd_scratch.cut.node[i].slot==slot+1)return i+1;
 }
 if(ghd_scratch.cut.nodes==GHD_NODES) {w->exhausted=1;return 0;}
 if(h->state!=HL_LIVE||h->kind!=HL_EXP||!h->generation||!ghd_original_span(h->heap,0xb8)||
    !ghd_original_span(h->start,h->bytes)||h->vtable!=(u32)_gz_hl_exp_table||
    (h->direction!=1&&h->direction!=0xffffffffu)||s->active_token||s->flags||s->reserved)return 0;
 if(!ghd_original_obd_reserve(w,24))return 0;
 i=ghd_scratch.cut.nodes++;n=ghd_scratch.cut.node+i;
 ghd_words(&n->heap,h,64);ghd_words(&n->scope,s,32);n->slot=slot+1;
 return i+1;
}
GHD_FN u32 ghd_roots(const GhdLabels*labels,ObTryWork*w) {
 u32 r,k,slot,node,previous,seen;const u32*values=(const u32*)labels;HlHeap*h;
 for(r=0;r<GHD_ROOTS;r++) {
  h=ghd_original_obt_heap(values[r],w);if(!h)return 0;
  slot=(u32)(h-_gz_hl_heaps);ghd_scratch.mask[slot]|=(unsigned char)(1u<<r);
  ghd_scratch.cut.root[r].label=values[r];previous=0;seen=0;
  for(k=0;k<GHD_DEPTH;k++) {
   if(!ghd_original_obt_take(w))return 0;
   node=ghd_node(slot,w);if(!node)return 0;
   if(seen&(1u<<(node-1)))return 0;seen|=1u<<(node-1);
   ghd_scratch.cut.node[node-1].root_mask|=1u<<r;
   if(!previous)ghd_scratch.cut.root[r].node=node;
   else ghd_scratch.cut.node[previous-1].parent_node=node;
   previous=node;
   if(ghd_scratch.parent[slot]==GHD_BAD_PARENT)return 0;
   if(!ghd_scratch.parent[slot])break;
   slot=ghd_scratch.parent[slot]-1;
  }
  if(k==GHD_DEPTH)return 0;
 }
 return 1;
}
GHD_FN void ghd_project(HlAllocation*d,const HlStoredAllocation*s) {
 d->pointer=s->pointer;d->generation=s->generation;d->state=s->state;d->kind=s->kind;
 d->heap=s->heap;d->heap_generation=s->heap_generation;d->requested=s->requested;d->alignment=s->alignment;
 d->block=s->block;d->capacity=s->capacity;d->padding=s->kind==HL_EXP&&s->block?s->pointer-s->block-16u:0;
 d->born_frame=s->born_frame;d->end_frame=s->end_frame;d->birth_site=s->birth_site;d->end_site=s->end_site;
 d->release_token=s->release_token;d->flags=s->flags;d->heap_epoch=s->heap_epoch;d->missing=s->missing;d->reserved=s->reserved;
}
/* A completed Exp resize is a fresh current generation, not an error bit.
 * Keep the old allowed provenance unchanged. The extra case is EXACT flags16
 * with the fresh LIVE publication's zero terminal fields; mixed/retired/error
 * flags stay refused. All state/epoch/HI/scope/extent tests remain below. */
GHD_FN __attribute__((noipa)) u32 ghd_backing_provenance(u32 flags,u32 end_frame,u32 end_site) {
 return !(flags&~OB_PROVENANCE)||(flags==HM_RESIZED&&!end_frame&&!end_site);
}
GHD_FN u32 ghd_backing(ObTryWork*w) {
 u32 i,parent;GhdNode*n,*p;HlStoredAllocation*a;
 for(i=0;i<ghd_scratch.cut.nodes;i++) {
  if(!ghd_original_obt_take(w))return 0;n=ghd_scratch.cut.node+i;parent=n->parent_node;
  if(!parent) {ghd_scratch.cut.flags|=2;continue;} /* Observed terminal, NOT SDK certificate. */
  p=ghd_scratch.cut.node+parent-1;
  if(!n->scope.managed_pointer||!n->scope.managed_generation||
     n->scope.header_pointer!=n->scope.managed_pointer||n->scope.header_generation!=n->scope.managed_generation)return 0;
  a=ghd_original_obt_generation(n->scope.managed_pointer,n->scope.managed_generation,w);
  if(!a||a->state!=HL_LIVE||a->heap!=p->heap.heap||a->heap_generation!=p->heap.generation||
     a->heap_epoch!=p->heap.epoch||a->kind!=p->heap.kind||a->release_token||a->reserved||
     a->missing!=31||!ghd_backing_provenance(a->flags,a->end_frame,a->end_site)||
     !ghd_original_hi_member((u32)(a-_gz_hl_allocations)+1,p->slot,p->heap.generation)||
     !ghd_original_within(p->heap.start,p->heap.bytes,a->pointer,a->capacity)||
     !ghd_original_within(p->heap.start,p->heap.bytes,a->block,16)||
     !ghd_original_within(a->pointer,a->capacity,n->heap.start,n->heap.bytes)||
     !ghd_original_within(a->pointer,a->capacity,n->heap.heap,0xb8)||!a->block||a->block>a->pointer||a->pointer-a->block<16)return 0;
  if(!ghd_original_obd_reserve(w,20))return 0;
  ghd_project(&n->backing,a);n->backing_row=(u32)(a-_gz_hl_allocations)+1;
 }
 return 1;
}

/* One counted pass: all4096 index headers, only selected descendant lists.
 * Membership is a direct slot byte per node; no allocation invokes owner,
 * ancestor, pointer-generation hash, native header or class-specific mapping.
 */
GHD_FN u32 ghd_census(ObTryWork*w) {
 u32 i,j,r,node,previous,all=0,hc=0,mask;HlHeap*h;HiHeap*s;HiLink*l;HlStoredAllocation*a;GhdRoot*t;
 for(i=0;i<HL_HEAPS;i++) {
  if(!ghd_original_obt_take(w))return 0;h=_gz_hl_heaps+i;s=_gz_hi_heaps+i;
  if(!h->heap) {if(s->head||s->count||s->generation||s->reserved)return 0;continue;}
  if(s->generation!=h->generation||s->reserved||s->count>HL_ALLOCS-all||(!s->head)!=(!s->count))return 0;
  all+=s->count;if(h->state==HL_LIVE||h->state==HL_ENDING||h->state==HL_MUTATING)hc++;
  if(ghd_scratch.mask[i]&GHD_UNKNOWN) {ghd_scratch.cut.unknown_chain_heaps++;ghd_scratch.cut.unknown_chain_indexed+=s->count;ghd_scratch.cut.flags|=1;continue;}
  mask=ghd_scratch.mask[i]&GHD_MASK;if(!mask)continue;
  if(h->state!=HL_LIVE||!h->generation||h->kind<HL_EXP||h->kind>HL_CUSTOM||
     !ghd_original_span(h->heap,header_bytes(h->kind))||!managed(h->kind,h->start,h->bytes))return 0;
  ghd_scratch.cut.selected_heaps++;
  for(r=0;r<GHD_ROOTS;r++) {if(!ghd_original_obt_take(w))return 0;if(mask&(1u<<r)) {t=ghd_scratch.cut.root+r;t->heaps++;ghd_add(&t->managed_lo,&t->managed_hi,h->bytes);}}
  node=s->head;previous=0;
  for(j=0;j<s->count;j++) {
   if(!ghd_original_obt_take(w)||!ghd_original_hi_member(node,i+1,h->generation))return 0;a=_gz_hl_allocations+node-1;l=_gz_hi_links+node-1;
   if(l->previous!=previous||a->heap_epoch!=h->epoch||a->kind!=h->kind||a->reserved||a->missing!=31||
      !ghd_original_within(h->start,h->bytes,a->pointer,a->capacity))return 0;
   /* Same raw selected-directory Exp projection constraints as obd_census.
    * No native block read; malformed private block arithmetic refuses. */
   if(a->kind==HL_EXP&&a->block&&(a->block>a->pointer||a->pointer-a->block<16||
      !ghd_original_within(h->start,h->bytes,a->block,16)))return 0;
   previous=node;node=l->next;ghd_scratch.cut.selected_allocations++;
   for(r=0;r<GHD_ROOTS;r++) {if(!ghd_original_obt_take(w))return 0;if(mask&(1u<<r)) {
    t=ghd_scratch.cut.root+r;if(a->state==HL_LIVE)t->live++;else t->ending++;
    if(!ghd_original_span(a->pointer,a->capacity))t->non_cpu++;
    ghd_add(&t->capacity_lo,&t->capacity_hi,a->capacity);ghd_add(&t->requested_lo,&t->requested_hi,a->requested);
   }}
  }
  if(node)return 0;
 }
 return hc==_gz_hl_control.active_heaps&&all==_gz_hl_control.active_allocations&&all==_gz_hi_control.nodes;
}
GHD_API ObU32 _gz_ob_try_actor_heap_cut(const GhdLabels*input,GhdCut*output) {
 ObTryWork work={GHD_WORK,0};GhdLabels labels;u32 i,z=OB_OBSERVER;
 if(!ghd_span((u32)input,16)||!ghd_span((u32)output,sizeof(GhdCut))||
    !hr_apart((u32)input,16,(u32)output,sizeof(GhdCut))||
    !_gz_image_write_allowed((u32)input,16)||!_gz_image_write_allowed((u32)output,sizeof(GhdCut)))return OB_ARGUMENT;
 if(!obt_real_try_lock())return OB_BUSY;
 if(!ghd_original_obt_healthy())goto done; /* All existing HL/HI/HM flags retained: no cap/flag workaround. */
 if(!ghd_private((u32)input,(u32)output,&work)) {z=OB_ARGUMENT;goto done;}
 ghd_words(&labels,input,16);
 if(labels.reserved) {z=OB_ARGUMENT;goto done;}
 if(!labels.game0||!labels.game1||!labels.zelda) {z=OB_ABSENT;goto done;}
 if(!ghd_original_span(labels.game0,0xb8)||!ghd_original_span(labels.game1,0xb8)||!ghd_original_span(labels.zelda,0xb8)) {z=OB_ARGUMENT;goto done;}
 if(!ghd_original_obd_reserve(&work,sizeof(ghd_scratch)/4))goto done;
 for(i=0;i<sizeof(ghd_scratch)/4;i++)((u32*)&ghd_scratch)[i]=0;
 if(labels.game0==labels.game1||labels.game0==labels.zelda||labels.game1==labels.zelda)ghd_scratch.cut.flags|=4;
 if(!ghd_parents(&work)||!ghd_roots(&labels,&work))goto done;
 for(i=0;i<HL_HEAPS;i++)if(!ghd_membership(i,&work))goto done;
 if(!ghd_backing(&work)||!ghd_census(&work))goto done;
 if(!ghd_original_obd_reserve(&work,sizeof(GhdCut)/4))goto done;
 ghd_scratch.cut.magic=0x47484431u;ghd_scratch.cut.bytes=sizeof(GhdCut);ghd_scratch.cut.version=1;
 ghd_scratch.cut.global_heaps=_gz_hl_control.active_heaps;ghd_scratch.cut.global_allocations=_gz_hl_control.active_allocations;
 ghd_scratch.cut.serial=_gz_hl_control.serial;ghd_scratch.cut.release_serial=_gz_hl_control.release_serial;
 ghd_scratch.cut.frame=_gz_hl_frame;ghd_scratch.cut.missing=31;ghd_scratch.cut.session=0;
 ghd_words(output,&ghd_scratch.cut,sizeof(GhdCut));z=OB_OK;
done:
 if(work.exhausted)z=OB_BUDGET;
 obt_real_unlock();return z;
}
#undef GHD_FN
#undef GHD_API
