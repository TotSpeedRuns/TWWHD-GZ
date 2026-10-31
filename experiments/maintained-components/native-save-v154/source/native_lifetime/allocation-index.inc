/* Observer-owned metadata, never part of a replayed heap image. All access is
 * under the common lifetime lock. Indexes are row+1 / heap-slot+1; zero is nil.
 * Keep public HlHeap/HlAllocation projections unchanged. A fresh boot is required;
 * an already populated observer cannot silently adopt an empty side index. */
typedef struct {u32 head,count,generation,reserved;} HiHeap;
/* Inclusive row+1 range0..262144 requires19bits; inclusive heap
 * slot+1 range0..4096 requires13bits. Generation always retains32bits. */
typedef struct {u32 previous:19,reserved:13;u32 next:19,owner:13;u32 generation;} HiLink;
typedef struct {u32 magic,bytes,version,flags,nodes,attached,detached,visits;
 u32 heap_walks,full_audits,reserved[6];} HiControl;
HiHeap _gz_hi_heaps[HL_HEAPS];
HiLink _gz_hi_links[HL_ALLOCS];
HiControl _gz_hi_control={.magic=0x48495831u,.bytes=64,.version=1};
typedef char HiControlSize[sizeof(HiControl)==64?1:-1];
typedef char HiHeapSize[sizeof(HiHeap)==16?1:-1];
typedef char HiLinkSize[sizeof(HiLink)==12?1:-1];
static u32 hi_bad(void){_gz_hi_control.flags|=1;flag(HL_BAD_HEADER);return 0;}
static u32 hi_header(void){return _gz_hi_control.magic==0x48495831u&&_gz_hi_control.bytes==64&&
 _gz_hi_control.version==1&&!_gz_hi_control.flags&&_gz_hi_control.nodes<=HL_ALLOCS;}
static u32 hi_id(HlStoredAllocation*a){return (u32)(a-_gz_hl_allocations)+1;}
static u32 hi_active(const HlStoredAllocation*a){return a->state==HL_LIVE||a->state==HL_ENDING;}
static u32 hi_member(u32 id,u32 owner,u32 generation){HlStoredAllocation*a;HiLink*l;
 if(!id||id>HL_ALLOCS||!owner||owner>HL_HEAPS)return 0;
 a=_gz_hl_allocations+id-1;l=_gz_hi_links+id-1;
 return hi_active(a)&&a->heap==_gz_hl_heaps[owner-1].heap&&a->heap_generation==generation&&
  !l->reserved&&l->owner==owner&&l->generation==a->generation&&a->generation;
}
static u32 hrp_hi_ready_original(HlHeap*h){u32 owner=(u32)(h-_gz_hl_heaps)+1,node,previous=0,i;HiHeap*s;
 if(!hi_header()||!owner||owner>HL_HEAPS)return hi_bad();s=_gz_hi_heaps+owner-1;
 if(s->generation!=h->generation||s->reserved||s->count>HL_ALLOCS||(!s->head)!=(!s->count))return hi_bad();
 node=s->head;bump(&_gz_hi_control.heap_walks);
 for(i=0;i<s->count;i++){
  if(!hi_member(node,owner,h->generation)||_gz_hi_links[node-1].previous!=previous)return hi_bad();
  bump(&_gz_hi_control.visits);previous=node;node=_gz_hi_links[node-1].next;
 }return node?hi_bad():1;
}
static u32 hi_ready(HlHeap*h){HrpTicks started=0;HrpInvocation invocation;u32 active=hrp_window(&invocation),result;
 if(active)started=hrp_clock();result=hrp_hi_ready_original(h);
 if(active)hrp_note(0,&invocation,started);return result;
}
static u32 hi_heap_born(HlHeap*h){HiHeap*s=&_gz_hi_heaps[h-_gz_hl_heaps];
 if(!hi_header()||s->head||s->count||s->reserved)return hi_bad();s->generation=h->generation;return 1;
}
static u32 hi_detach(HlStoredAllocation*a){u32 id=hi_id(a),owner;HiLink*l,*p=0,*n=0;HiHeap*s;
 if(!hi_header()||!id||id>HL_ALLOCS)return hi_bad();l=_gz_hi_links+id-1;
 if(!hi_active(a))return (l->reserved||l->owner||l->previous||l->next||l->generation)?hi_bad():1;
 owner=l->owner;if(!owner||owner>HL_HEAPS)return hi_bad();s=_gz_hi_heaps+owner-1;
 if(s->generation!=a->heap_generation||!hi_member(id,owner,s->generation)||!s->count||!_gz_hi_control.nodes)return hi_bad();
 if(l->previous){if(!hi_member(l->previous,owner,s->generation))return hi_bad();p=_gz_hi_links+l->previous-1;if(p->next!=id)return hi_bad();}
 else if(s->head!=id)return hi_bad();
 if(l->next){if(!hi_member(l->next,owner,s->generation))return hi_bad();n=_gz_hi_links+l->next-1;if(n->previous!=id)return hi_bad();}
 /* All neighbors checked before the first write. */
 if(p)p->next=l->next;else s->head=l->next;if(n)n->previous=l->previous;
 s->count--;_gz_hi_control.nodes--;zero(l,sizeof(*l));bump(&_gz_hi_control.detached);return 1;
}
static u32 hi_attach(HlStoredAllocation*a){HlHeap*h;HiHeap*s;HiLink*l;u32 id=hi_id(a),owner;
 if(!hi_header()||!id||id>HL_ALLOCS||!hi_active(a))return hi_bad();l=_gz_hi_links+id-1;
 if(l->reserved||l->owner||l->previous||l->next||l->generation)return hi_bad();h=heap_find(a->heap,0);
 if(!h||h->generation!=a->heap_generation)return hi_bad();owner=(u32)(h-_gz_hl_heaps)+1;s=_gz_hi_heaps+owner-1;
 if(s->generation!=h->generation||s->reserved||s->count==HL_ALLOCS||_gz_hi_control.nodes==HL_ALLOCS||(!s->head)!=(!s->count))return hi_bad();
 if(s->head&&(!hi_member(s->head,owner,h->generation)||_gz_hi_links[s->head-1].previous))return hi_bad();
 l->owner=owner;l->generation=a->generation;l->next=s->head;
 if(s->head)_gz_hi_links[s->head-1].previous=id;s->head=id;s->count++;_gz_hi_control.nodes++;
 bump(&_gz_hi_control.attached);return 1;
}
static u32 hi_store(HlStoredAllocation*a,const HlStoredAllocation*r){
 if(!hi_detach(a))return 0;copy(a,r,sizeof(*a));return hi_attach(a);
}
static u32 hi_head(HlHeap*h){return _gz_hi_heaps[h-_gz_hl_heaps].head;}
static u32 hi_next(u32 node){return _gz_hi_links[node-1].next;}
u32 _gz_hi_check_heap(u32 heap){HlHeap*h;u32 z=1;
 if(!lock())return 1;h=heap_find(heap,0);if(h&&hi_ready(h))z=0;unlock();return z;
}
/* Explicit diagnostic audit; ordinary heap capture/retirement does not call
 * this full table scan. No native access or actor authority is granted. */
u32 _gz_hi_audit(void){u32 i,total=0,z=1;HlHeap*h;HiHeap*s;HiLink*l;HlStoredAllocation*a;
 if(!lock())return 1;if(!hi_header())goto done;bump(&_gz_hi_control.full_audits);
 for(i=0;i<HL_HEAPS;i++){
  h=_gz_hl_heaps+i;s=_gz_hi_heaps+i;
  if(!h->heap){if(s->head||s->count||s->generation||s->reserved)goto done;continue;}
  if(!hi_ready(h)||s->count>HL_ALLOCS-total)goto done;total+=s->count;
 }
 if(total!=_gz_hi_control.nodes||total!=_gz_hl_control.active_allocations)goto done;
 for(i=0;i<HL_ALLOCS;i++){
  a=_gz_hl_allocations+i;l=_gz_hi_links+i;
  if(hi_active(a)){if(!hi_member(i+1,l->owner,a->heap_generation))goto done;}
  else if(l->reserved||l->owner||l->previous||l->next||l->generation)goto done;
 }z=0;
done:if(z)hi_bad();unlock();return z;
}
