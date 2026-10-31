#include "source_directory.h"
ScDirectory *_gz_sc_storage;
u32 _gz_scs_report[16]={0x53435331u,64,1};
static u32 scs_busy,scs_seal;
static u32 scs_hash(void){return 0x53435331u^_gz_scs_report[5]^_gz_scs_report[6]^_gz_scs_report[7];}
u32 _gz_sc_storage_ready(void){return _gz_scs_report[0]==0x53435331u&&_gz_scs_report[1]==64&&
 _gz_scs_report[2]==1&&_gz_scs_report[3]==3&&!_gz_scs_report[4]&&
 _gz_scs_report[5]==sizeof(ScDirectory)&&_gz_scs_report[6]==(u32)_gz_sc_storage&&
 _gz_scs_report[7]&&_gz_scs_report[8]==1&&scs_seal==scs_hash()&&
 span((u32)_gz_sc_storage,sizeof(ScDirectory))&&!((u32)_gz_sc_storage&255);}
typedef char ScHeaderSize[sizeof(ScHeader)==128?1:-1];
typedef char ScHeapSize[sizeof(ScHeap)==384?1:-1];
typedef char ScAllocationSize[sizeof(ScAllocation)==96?1:-1];
typedef char ScObjectSize[sizeof(ScObject)==128?1:-1];
typedef char ScEdgeSize[sizeof(ScEdge)==24?1:-1];
typedef char ScRegistrySize[sizeof(ScRegistry)==48?1:-1];
typedef struct {const ScHeader*header;const ScHeap*heaps;const ScAllocation*allocations;
 const ScObject*objects;const ScEdge*edges;const ScRegistry*registry;} ScView;
static void sc_fixed_view(ScView*v,const ScDirectory*d){v->header=&d->header;v->heaps=d->heaps;v->allocations=d->allocations;v->objects=d->objects;v->edges=d->edges;v->registry=d->registry;}
enum {SC_HASH=8192,SC_CAT_HASH=65536};
static u32 sc_busy,sc_alloc_index[SC_HASH],sc_cat_heads[SC_CAT_HASH],sc_cat_next[SC_REGISTRY];
#include "capture-index.inc"
static u32 sc_hash_key(u32 heap,u32 pointer,u32 generation,u32 mask){return (((pointer>>2)*0x9e3779b1u)^heap^(generation*0x85ebca6bu))&mask;}
static u32 sc_static_private(u32 p,u32 n){return
 hr_apart(p,n,(u32)&_gz_sc_storage,sizeof(_gz_sc_storage))&&hr_apart(p,n,(u32)_gz_scs_report,sizeof(_gz_scs_report))&&
 hr_apart(p,n,(u32)&scs_busy,4)&&hr_apart(p,n,(u32)&scs_seal,4)&&
 hr_apart(p,n,(u32)&sc_busy,4)&&hr_apart(p,n,(u32)sc_alloc_index,sizeof(sc_alloc_index))&&
 hr_apart(p,n,(u32)sc_cat_heads,sizeof(sc_cat_heads))&&hr_apart(p,n,(u32)sc_cat_next,sizeof(sc_cat_next))&&si_private(p,n);}
u32 _gz_sc_private(u32 p,u32 n){u32 candidate=_gz_scs_report[6];
 if(_gz_sc_storage&&!_gz_sc_storage_ready())return 0;
 if(candidate&&(scs_seal!=scs_hash()||!span(candidate,sizeof(ScDirectory))||
  !hr_apart(p,n,candidate,sizeof(ScDirectory))))return 0;
 return sc_static_private(p,n);
}
static u32 sc_outputs_private(void){u32 i;HlHeap*h;
 if(!_gz_sc_storage_ready())return 0;
 if(!si_outputs_private()||!hr_private_base((u32)&_gz_sc_directory,sizeof(_gz_sc_directory),0)||
  !hr_apart((u32)&_gz_sc_directory,sizeof(_gz_sc_directory),(u32)&_gz_rd_report,sizeof(_gz_rd_report)))return 0;
 for(i=0;i<HL_HEAPS;i++){h=_gz_hl_heaps+i;if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)){
  if(!hr_private_base((u32)&_gz_sc_directory,sizeof(_gz_sc_directory),h)||
   !hr_private_base((u32)&sc_busy,4,h)||!hr_private_base((u32)sc_alloc_index,sizeof(sc_alloc_index),h)||
   !hr_private_base((u32)sc_cat_heads,sizeof(sc_cat_heads),h)||!hr_private_base((u32)sc_cat_next,sizeof(sc_cat_next),h))return 0;
 }}return rd_output_private();
}
static ScHeap*sc_heap(const ScView*d,u32 heap){u32 i;for(i=0;i<d->header->heap_count;i++)if(d->heaps[i].identity.heap==heap)return (ScHeap*)(d->heaps+i);return 0;}
static u32 sc_allocation_find(const ScAllocation*records,u32 heap,u32 pointer,u32 generation){u32 key=sc_hash_key(heap,pointer,generation,SC_HASH-1),i,n;const HlAllocation*a;
 for(i=0;i<SC_HASH;i++){n=sc_alloc_index[(key+i)&(SC_HASH-1)];if(!n)return SC_ALLOCS;a=&records[n-1].identity;
  if(a->heap==heap&&a->pointer==pointer&&a->generation==generation)return n-1;
 }return SC_ALLOCS;
}
static u32 sc_allocation_index(const ScAllocation*records,u32 n){u32 key,i;const HlAllocation*a=&records[n].identity;
 key=sc_hash_key(a->heap,a->pointer,a->generation,SC_HASH-1);
 for(i=0;i<SC_HASH;i++){if(!sc_alloc_index[(key+i)&(SC_HASH-1)]){sc_alloc_index[(key+i)&(SC_HASH-1)]=n+1;return 1;}}return 0;
}
static u32 sc_add_allocation(const HlAllocation*a,u32*out){ScDirectory*d=&_gz_sc_directory;u32 i=sc_allocation_find(d->allocations,a->heap,a->pointer,a->generation);ScAllocation*r;
 if(i<SC_ALLOCS){if(!rd_equal(&d->allocations[i].identity,a,sizeof(*a)))return SC_CHANGED;*out=i;return 0;}
 i=d->header.allocation_count;if(i>=SC_ALLOCS)return SC_CAPACITY;r=d->allocations+i;zero(r,sizeof(*r));copy(&r->identity,a,sizeof(*a));
 d->header.allocation_count++;if(!sc_allocation_index(d->allocations,i))return SC_CAPACITY;*out=i;return 0;
}
static u32 sc_allocation_valid(const ScView*d,const HlAllocation*a){const ScHeap*h=sc_heap(d,a->heap);u32 norm;
 if(!h||!a->generation||a->state!=HL_LIVE||a->kind!=h->identity.kind||a->heap_generation!=h->identity.generation||
  a->heap_epoch!=h->identity.epoch||a->requested>0xfffffffcu||a->missing!=31||a->reserved||a->release_token||a->end_frame||a->end_site)return 0;
 norm=((a->requested<4?4:a->requested)+3)&~3u;
 return norm&&a->capacity>=norm&&within(h->identity.start,h->identity.bytes,a->pointer,a->capacity);
}
static u32 sc_ancestry(const ScView*d,const ScHeap*h,u32 depth,u32*budget){const ScHeap*p,*o;const HlHeap*a;
 for(;depth<32;depth++){
  if(!*budget)return 0;--*budget;a=&h->identity;
  if(a->kind==HL_CUSTOM){
   if(h->custom.heap_generation!=a->generation||!h->custom.slots||h->custom.slots>0x0fffffffu||
    h->custom.anchor_pointer!=h->custom_anchor.pointer||h->custom.anchor_generation!=h->custom_anchor.generation||
    !sc_allocation_valid(d,&h->custom_anchor)||!within(h->custom_anchor.pointer,h->custom_anchor.capacity,h->custom.table,h->custom.slots*16))return 0;
   o=sc_heap(d,h->custom_anchor.heap);if(!o||o==h||!sc_ancestry(d,o,depth+1,budget))return 0;
  }
  if(!a->parent)return a->parent_generation==0;
  p=sc_heap(d,a->parent);
  if(!p||p==h||a->parent_generation!=p->identity.generation||h->scope.parent_epoch!=p->identity.epoch||
   h->scope.flags||h->scope.active_token||h->scope.reserved||
   h->scope.managed_pointer!=h->managed.pointer||h->scope.managed_generation!=h->managed.generation||
   h->scope.header_pointer!=h->header.pointer||h->scope.header_generation!=h->header.generation||
   !sc_allocation_valid(d,&h->managed)||!sc_allocation_valid(d,&h->header)||h->managed.heap!=a->parent||
   !within(h->managed.pointer,h->managed.capacity,a->start,a->bytes)||!within(h->header.pointer,h->header.capacity,a->heap,header_bytes(a->kind)))return 0;
  o=sc_heap(d,h->header.heap);if(!o)return 0;if(o!=p&&!sc_ancestry(d,o,depth+1,budget))return 0;h=p;
 }return 0;
}
static u32 sc_heap_capture(void){ScDirectory*d=&_gz_sc_directory;HlHeap*h;HmScope*s;HlStoredAllocation*a;ScHeap*out;u32 i;
 for(i=0;i<HL_HEAPS;i++){
  h=_gz_hl_heaps+i;if(!h->heap||h->state!=HL_LIVE)continue;
  if(!hr_native_header(h)||!hi_ready(h)||!_gz_hm_scope_live(h))return SC_ANCESTRY;
  out=d->heaps+d->header.heap_count++;zero(out,sizeof(*out));copy(&out->identity,h,sizeof(*h));
  s=_gz_hm_scopes+i;copy(&out->scope,s,sizeof(*s));
  if(h->parent){a=allocation_generation(s->managed_pointer,s->managed_generation);if(!a)return SC_ANCESTRY;hl_export(&out->managed,a);
   a=allocation_generation(s->header_pointer,s->header_generation);if(!a)return SC_ANCESTRY;hl_export(&out->header,a);}
  if(h->kind==HL_CUSTOM){copy(&out->custom,_gz_hl_custom_meta+i,sizeof(out->custom));
   a=allocation_generation(out->custom.anchor_pointer,out->custom.anchor_generation);if(!a)return SC_ANCESTRY;hl_export(&out->custom_anchor,a);}
  out->native_flags=R32(h->heap,0x90);out->first=R32(h->heap,0x44);out->last=R32(h->heap,0x40);out->count=R32(h->heap,0x48);out->stride=R32(h->heap,0x4c);
 }
 return d->header.heap_count==_gz_hl_control.active_heaps?0:SC_CHANGED;
}
static u32 sc_registry_capture(void){ScDirectory*d=&_gz_sc_directory;ScHeap*h;ScRegistry*r;HlStoredAllocation*a;HlHeap*owner;u32 i,j,head,node,previous,object,z,key;
 for(i=0;i<d->header.heap_count;i++){
  h=d->heaps+i;head=h->identity.heap+0x40;node=h->first;previous=head;h->registry_begin=d->header.registry_count;
  if(h->stride!=4||h->count>SC_REGISTRY-d->header.registry_count)return SC_CAPACITY;
  for(j=0;j<h->count;j++){
   if(node==head||node<4)return SC_REGISTRY_ERROR;object=node-4;
   z=si_owner_locked(object,16,&a,&owner);if(z)return z==3?SC_OBSERVER:SC_UNTRACKED;
   if(R32(object,0)!=h->identity.heap||R32(node,0)!=previous)return SC_REGISTRY_ERROR;
   r=d->registry+d->header.registry_count;zero(r,sizeof(*r));
   r->object=object;r->owner=h->identity.heap;r->storage_heap=a->heap;r->storage_pointer=a->pointer;r->storage_generation=a->generation;r->storage_capacity=a->capacity;
   r->vtable=R32(object,12);r->previous=previous;r->next=R32(node,4);
   key=sc_hash_key(a->heap,a->pointer,a->generation,SC_CAT_HASH-1);
   sc_cat_next[d->header.registry_count]=sc_cat_heads[key];sc_cat_heads[key]=++d->header.registry_count;
   previous=node;node=r->next;
  }
  if(node!=head||previous!=h->last)return SC_REGISTRY_ERROR;
 }return 0;
}
static u32 sc_expand(void){ScDirectory*d=&_gz_sc_directory;ScAllocation*a;ScRegistry*c;ScObject*o;ScEdge*e;RdReport*r=&_gz_rd_report;u32 i,j,node,z,k,root,unknown;
 for(i=0;i<d->header.allocation_count;i++){
  a=d->allocations+i;a->object_begin=d->header.object_count;
  node=sc_cat_heads[sc_hash_key(a->identity.heap,a->identity.pointer,a->identity.generation,SC_CAT_HASH-1)];
  for(;node;node=sc_cat_next[node-1]){
   c=d->registry+node-1;
   if(c->storage_heap!=a->identity.heap||c->storage_pointer!=a->identity.pointer||c->storage_generation!=a->identity.generation)continue;
   if(d->header.object_count>=SC_OBJECTS)return SC_CAPACITY;o=d->objects+d->header.object_count;zero(o,sizeof(*o));
   o->object=c->object;o->owner=c->owner;o->allocation=i;o->offset=c->object-a->identity.pointer;o->vtable=c->vtable;o->edge_begin=d->header.edge_count;
   unknown=!rd_type(c->vtable);o->provider=unknown?0:1;
   if(unknown){copy(o->raw,(const void*)c->object,16);d->header.unknown_count++;}
   else{
    z=rd_inspect_locked(c->object,c->owner);if(z)return z;
    if(!r->valid||!rd_equal(&r->root.allocation,&a->identity,sizeof(HlAllocation)))return SC_CHANGED;
    copy(o->raw,r->object_bytes,72);copy(o->table,r->table_value,8);
    if(r->edge_count>SC_EDGES-d->header.edge_count)return SC_CAPACITY;
    for(j=0;j<r->edge_count;j++){
     z=sc_add_allocation(&r->edges[j].identity.allocation,&k);if(z)return z;
     e=d->edges+d->header.edge_count++;e->object=d->header.object_count;e->slot=r->edges[j].slot;e->role=r->edges[j].role;
     e->field=r->edges[j].field;e->value=r->edges[j].value;e->allocation=k;o->edge_count++;
    }
   }d->header.object_count++;a->object_count++;
  }
 }return 0;
}
static u32 sc_recheck(void){ScDirectory*d=&_gz_sc_directory;ScHeap*h;HlHeap*live;ScRegistry*c;ScObject*o;ScEdge*e;u32 i,j;
 if(!hr_healthy()||d->header.serial!=_gz_hl_control.serial||d->header.release_serial!=_gz_hl_control.release_serial||
  d->header.active_allocations!=_gz_hl_control.active_allocations||d->header.frame!=_gz_hl_frame)return SC_CHANGED;
 for(i=0;i<d->header.heap_count;i++){h=d->heaps+i;live=heap_find(h->identity.heap,0);
  if(!live||!rd_equal(live,&h->identity,sizeof(HlHeap))||!hr_native_header(live)||
   R32(live->heap,0x90)!=h->native_flags||R32(live->heap,0x40)!=h->last||R32(live->heap,0x44)!=h->first||
   R32(live->heap,0x48)!=h->count||R32(live->heap,0x4c)!=h->stride)return SC_CHANGED;
 }
 for(i=0;i<d->header.registry_count;i++){c=d->registry+i;
  if(R32(c->object,0)!=c->owner||R32(c->object,4)!=c->previous||R32(c->object,8)!=c->next||R32(c->object,12)!=c->vtable)return SC_CHANGED;
 }
 for(i=0;i<d->header.object_count;i++){o=d->objects+i;
  if(!rd_equal(o->raw,(const void*)o->object,o->provider?72:16))return SC_CHANGED;
  for(j=0;j<o->edge_count;j++){e=d->edges+o->edge_begin+j;if(e->role==RD_OWNED_TABLE&&R32(e->value,0)!=o->table[e->slot])return SC_CHANGED;}
 }return 0;
}
static u32 sc_hash_part(u32 hash,const void*p,u32 n){u32 i;const unsigned char*b=p;for(i=0;i<n;i++)hash=(hash^b[i])*16777619u;return hash;}
static u32 sc_view_checksum(const ScView*d){u32 hash=2166136261u,i;const unsigned char*b=(const unsigned char*)d->header;
 for(i=0;i<sizeof(ScHeader);i++)hash=(hash^((i>=96&&i<100)?0:b[i]))*16777619u;
 hash=sc_hash_part(hash,d->heaps,d->header->heap_count*sizeof(ScHeap));hash=sc_hash_part(hash,d->allocations,d->header->allocation_count*sizeof(ScAllocation));
 hash=sc_hash_part(hash,d->objects,d->header->object_count*sizeof(ScObject));hash=sc_hash_part(hash,d->edges,d->header->edge_count*sizeof(ScEdge));
 return sc_hash_part(hash,d->registry,d->header->registry_count*sizeof(ScRegistry));
}
static u32 sc_used(const ScHeader*h){return sizeof(*h)+h->heap_count*sizeof(ScHeap)+h->allocation_count*sizeof(ScAllocation)+
 h->object_count*sizeof(ScObject)+h->edge_count*sizeof(ScEdge)+h->registry_count*sizeof(ScRegistry);}
static u32 sc_zero(const void*p,u32 n){u32 i;for(i=0;i<n/4;i++)if(((const u32*)p)[i])return 0;return 1;}
static u32 sc_offsets(const ScHeader*h){return h->heaps_offset==__builtin_offsetof(ScDirectory,heaps)&&
 h->allocations_offset==__builtin_offsetof(ScDirectory,allocations)&&h->objects_offset==__builtin_offsetof(ScDirectory,objects)&&
 h->edges_offset==__builtin_offsetof(ScDirectory,edges)&&h->registry_offset==__builtin_offsetof(ScDirectory,registry);}
static u32 sc_validate_view(const ScView*d,u32 bytes){const ScHeader*h=d->header;const ScHeap*p,*heap;const ScAllocation*a;const ScRegistry*c;const ScObject*o;const ScEdge*e;
 u32 i,j,k,node,previous,count=0,objects=0,edges=0,unknown=0,budget,flags,base,table,value,expected,slot,role,found;
 if(bytes!=sizeof(ScDirectory)||h->magic!=0x53434431u||h->bytes!=sizeof(ScDirectory)||h->version!=1||h->valid!=1||h->code||h->missing!=31||h->session||h->reserved||
  !h->serial||!h->heap_count||h->heap_count>HL_HEAPS||!h->allocation_count||h->allocation_count>SC_ALLOCS||h->object_count>SC_OBJECTS||
  h->edge_count>SC_EDGES||h->registry_count>SC_REGISTRY||h->unknown_count>h->object_count||h->seed_allocation>=h->allocation_count||
  h->borrowed_complete||h->payloads_complete||h->writer_exclusion||h->owned_complete!=(h->unknown_count==0)||!sc_offsets(h)||h->used_bytes!=sc_used(h))return SC_IMAGE;
 if(h->checksum!=sc_view_checksum(d))return SC_IMAGE;
 zero(sc_alloc_index,sizeof(sc_alloc_index));zero(sc_cat_heads,sizeof(sc_cat_heads));zero(sc_cat_next,sizeof(sc_cat_next));
 for(i=0;i<h->heap_count;i++){
  p=d->heaps+i;if(!p->identity.heap||!p->identity.generation||p->identity.state!=HL_LIVE||p->identity.kind<HL_EXP||p->identity.kind>HL_CUSTOM||
   !span(p->identity.heap,header_bytes(p->identity.kind))||!managed(p->identity.kind,p->identity.start,p->identity.bytes)||
   (p->identity.direction!=1&&p->identity.direction!=0xffffffffu)||(p->native_flags&2)||p->reserved||p->stride!=4||
   p->count>h->registry_count-count||p->registry_begin!=count)return SC_IMAGE;
  for(j=0;j<i;j++)if(d->heaps[j].identity.heap==p->identity.heap)return SC_IMAGE;
  if(!p->identity.parent&&(!sc_zero(&p->scope,sizeof(p->scope))||!sc_zero(&p->managed,sizeof(p->managed))||!sc_zero(&p->header,sizeof(p->header))))return SC_IMAGE;
  if(p->identity.kind!=HL_CUSTOM&&(!sc_zero(&p->custom,sizeof(p->custom))||!sc_zero(&p->custom_anchor,sizeof(p->custom_anchor))))return SC_IMAGE;
  budget=64;if(!sc_ancestry(d,p,0,&budget))return SC_ANCESTRY;
  node=p->first;previous=p->identity.heap+0x40;
  for(j=0;j<p->count;j++){
   c=d->registry+count+j;heap=sc_heap(d,c->storage_heap);
   if(!span(c->object,16)||node!=c->object+4||c->owner!=p->identity.heap||c->previous!=previous||!c->storage_generation||!heap||
    !within(c->storage_pointer,c->storage_capacity,c->object,16)||!within(heap->identity.start,heap->identity.bytes,c->storage_pointer,c->storage_capacity)||
    !sc_zero(c->reserved,sizeof(c->reserved)))return SC_IMAGE;
   k=sc_hash_key(0,c->object,0,SC_CAT_HASH-1);
   for(found=sc_cat_heads[k];found;found=sc_cat_next[found-1])if(d->registry[found-1].object==c->object)return SC_IMAGE;
   sc_cat_next[count+j]=sc_cat_heads[k];sc_cat_heads[k]=count+j+1;previous=node;node=c->next;
  }count+=p->count;if(node!=p->identity.heap+0x40||previous!=p->last)return SC_IMAGE;
 }
 if(count!=h->registry_count)return SC_IMAGE;
 zero(sc_cat_heads,sizeof(sc_cat_heads));zero(sc_cat_next,sizeof(sc_cat_next));
 for(i=0;i<h->registry_count;i++){c=d->registry+i;k=sc_hash_key(c->storage_heap,c->storage_pointer,c->storage_generation,SC_CAT_HASH-1);
  sc_cat_next[i]=sc_cat_heads[k];sc_cat_heads[k]=i+1;
 }
 for(i=0;i<h->allocation_count;i++){
  a=d->allocations+i;if(!sc_allocation_valid(d,&a->identity)||!sc_zero(a->reserved,sizeof(a->reserved))||
   sc_allocation_find(d->allocations,a->identity.heap,a->identity.pointer,a->identity.generation)!=SC_ALLOCS||!sc_allocation_index(d->allocations,i))return SC_IMAGE;
 }
 for(i=0;i<h->allocation_count;i++){
  a=d->allocations+i;if(a->object_begin!=objects||a->object_count>h->object_count-objects)return SC_IMAGE;
  node=sc_cat_heads[sc_hash_key(a->identity.heap,a->identity.pointer,a->identity.generation,SC_CAT_HASH-1)];expected=0;
  for(;node;node=sc_cat_next[node-1]){c=d->registry+node-1;
   if(c->storage_heap==a->identity.heap&&c->storage_pointer==a->identity.pointer&&c->storage_generation==a->identity.generation){
    if(c->storage_capacity!=a->identity.capacity||expected>=a->object_count)return SC_IMAGE;
    /* Expansion preserves the deterministic bucket-chain order. */
    o=d->objects+objects+expected++;if(o->object!=c->object||o->owner!=c->owner||o->vtable!=c->vtable)return SC_IMAGE;
   }
  }if(expected!=a->object_count)return SC_IMAGE;
  for(j=0;j<a->object_count;j++){
   o=d->objects+objects+j;if(o->allocation!=i||o->offset!=o->object-a->identity.pointer||o->edge_begin!=edges||o->edge_count>h->edge_count-edges||
    !within(a->identity.pointer,a->identity.capacity,o->object,o->provider?72:16)||!sc_zero(o->reserved,sizeof(o->reserved))||
    R32(o->raw,0)!=o->owner||R32(o->raw,12)!=o->vtable||o->provider!=(rd_type(o->vtable)?1:0))return SC_IMAGE;
   if(!o->provider){unknown++;if(o->edge_count||!sc_zero(o->raw+16,56)||!sc_zero(o->table,8))return SC_IMAGE;continue;}
   k=0;
   for(slot=0;slot<2;slot++){
    base=16+slot*28;flags=o->raw[base+20];table=R32(o->raw,base);
    if(!(flags&1)&&o->table[slot])return SC_IMAGE;
    for(role=1;role<=3;role++){
     if(role==1){if(!(flags&2))continue;value=R32(o->raw,base+8);if(!value)continue;found=o->object+base+8;}
     else if(role==2){if(!(flags&1))continue;value=table;if(!value)return SC_IMAGE;found=o->object+base;}
     else {if(!(flags&1))continue;value=o->table[slot];if(!value)continue;found=table;}
     if(k>=o->edge_count)return SC_IMAGE;e=d->edges+edges+k++;
     if(e->object!=objects+j||e->slot!=slot||e->role!=role||e->field!=found||e->value!=value||e->allocation>=h->allocation_count||
      e->allocation==i||d->allocations[e->allocation].identity.pointer!=value)return SC_IMAGE;
     for(expected=0;expected+1<k;expected++)if(d->edges[edges+expected].allocation==e->allocation)return SC_IMAGE;
    }
   }if(k!=o->edge_count)return SC_IMAGE;edges+=k;
  }objects+=a->object_count;
 }
 if(objects!=h->object_count||edges!=h->edge_count||unknown!=h->unknown_count)return SC_IMAGE;
 found=0;for(i=0;i<h->object_count;i++)if(d->objects[i].object==h->seed_object&&d->objects[i].owner==h->seed_owner&&d->objects[i].allocation==h->seed_allocation)found++;
 if(found!=1)return SC_IMAGE;
 /* A known native delete target cannot have two exclusive owners. Shared
  * object slices are supported; double ownership of a deletion target is not. */
 zero(sc_cat_next,sizeof(sc_cat_next));
 for(i=0;i<h->edge_count;i++){e=d->edges+i;if(e->allocation==h->seed_allocation||sc_cat_next[e->allocation]++)return SC_IMAGE;}
 /* Every selected allocation must be reachable from the requested seed. */
 zero(sc_cat_next,sizeof(sc_cat_next));sc_cat_next[h->seed_allocation]=1;
 for(i=0;i<h->allocation_count;i++){found=0;for(j=0;j<h->edge_count;j++){e=d->edges+j;
  if(sc_cat_next[d->objects[e->object].allocation]&&!sc_cat_next[e->allocation]){sc_cat_next[e->allocation]=1;found=1;}
 }if(!found)break;}
 for(i=0;i<h->allocation_count;i++)if(!sc_cat_next[i])return SC_IMAGE;
 return 0;
}
static u32 sc_checksum(const ScDirectory*d){ScView v;sc_fixed_view(&v,d);return sc_view_checksum(&v);}
static u32 sc_validate_image(const ScDirectory*d,u32 bytes){ScView v;sc_fixed_view(&v,d);return sc_validate_view(&v,bytes);}
u32 _gz_sc_validate(const void*image,u32 bytes){u32 z;
 if(bytes!=sizeof(ScDirectory)||!si_private((u32)image,bytes)||!hr_private_base((u32)image,bytes,0)||!hr_apart((u32)image,bytes,(u32)&_gz_rd_report,sizeof(_gz_rd_report))||
  !hr_apart((u32)image,bytes,(u32)&sc_busy,4)||!hr_apart((u32)image,bytes,(u32)sc_alloc_index,sizeof(sc_alloc_index))||
  !hr_apart((u32)image,bytes,(u32)sc_cat_heads,sizeof(sc_cat_heads))||!hr_apart((u32)image,bytes,(u32)sc_cat_next,sizeof(sc_cat_next))||
  ((u32)image!=(u32)&_gz_sc_directory&&!hr_apart((u32)image,bytes,(u32)&_gz_sc_directory,sizeof(_gz_sc_directory))))return SC_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&sc_busy,0,1))return SC_BUSY;z=sc_validate_image(image,bytes);__sync_synchronize();sc_busy=0;return z;
}
/* Caller owns both sc_busy and the common lifetime lock, and has proved
 * every fixed output private. Allows one source interval to include payloads. */
static u32 sc_capture_locked(u32 object,u32 owner_heap){ScDirectory*d=&_gz_sc_directory;ScHeader*h=&d->header;HlStoredAllocation*a;HlAllocation projected;HlHeap*heap;u32 z=SC_ARGUMENT,i,seed;
 zero(h,sizeof(*h));h->magic=0x53434431u;h->bytes=sizeof(*d);h->version=1;h->missing=31;h->seed_object=object;h->seed_owner=owner_heap;
 h->heaps_offset=(u32)((unsigned char*)d->heaps-(unsigned char*)d);h->allocations_offset=(u32)((unsigned char*)d->allocations-(unsigned char*)d);
 h->objects_offset=(u32)((unsigned char*)d->objects-(unsigned char*)d);h->edges_offset=(u32)((unsigned char*)d->edges-(unsigned char*)d);h->registry_offset=(u32)((unsigned char*)d->registry-(unsigned char*)d);
 if(!hr_healthy()){z=SC_OBSERVER;goto done;}h->serial=_gz_hl_control.serial;h->release_serial=_gz_hl_control.release_serial;h->frame=_gz_hl_frame;h->active_allocations=_gz_hl_control.active_allocations;
 zero(sc_alloc_index,sizeof(sc_alloc_index));zero(sc_cat_heads,sizeof(sc_cat_heads));zero(sc_cat_next,sizeof(sc_cat_next));
 z=sc_heap_capture();if(z)goto done;z=si_build_locked();if(z)goto done;z=sc_registry_capture();if(z)goto done;
 z=owner_index_locked(object,16,&a,&heap);if(z){z=SC_UNTRACKED;goto done;}
 seed=0;for(i=0;i<h->registry_count;i++)if(d->registry[i].object==object&&d->registry[i].owner==owner_heap)seed++;
 if(seed!=1){z=SC_REGISTRY_ERROR;goto done;}hl_export(&projected,a);z=sc_add_allocation(&projected,&h->seed_allocation);if(z)goto done;
 z=sc_expand();if(z)goto done;z=sc_recheck();if(z)goto done;
 h->owned_complete=!h->unknown_count;h->used_bytes=sc_used(h);h->valid=1;h->checksum=sc_checksum(d);
 z=sc_validate_image(d,sizeof(*d));
done:if(z){h->valid=0;h->checksum=0;}h->code=z;
 return z;
}
u32 _gz_sc_capture(u32 object,u32 owner_heap){u32 z=SC_ARGUMENT;
 if(!_gz_sc_storage_ready())return SC_BUSY;
 if(!span(object,16)||!_gz_sc_private(object,16)||!hr_apart(object,16,(u32)&_gz_rd_report,sizeof(_gz_rd_report)))return SC_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&sc_busy,0,1))return SC_BUSY;
 if(!lock()){sc_busy=0;return SC_OBSERVER;}
 if(sc_outputs_private())z=sc_capture_locked(object,owner_heap);
 si_end();unlock();__sync_synchronize();sc_busy=0;return z;
}
