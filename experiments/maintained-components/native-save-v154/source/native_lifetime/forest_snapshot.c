/* OoT-inspired allocator/live-payload rewind for a stable TWWHD hierarchy.
 * Child heaps are replayed separately. Their complete address ranges and
 * header prefixes are excluded from the parent's payload replay, so child
 * OS mutexes and intrusive registries never get overwritten by a parent copy.
 * Reverse layouts are supported; changed registry topology and external
 * resource ownership still require the wider lifecycle transaction.
 */
#include "forest_snapshot.h"
#include "arena_snapshot.h"
static const AfDomain*forest_scope;
static const AgHeap*forest_heaps;
static unsigned int forest_count,forest_current,forest_busy,forest_validating;
#include "arena_part.c"
typedef struct {U32 magic,version,bytes,graph_bytes,count,index,data,checksum;
 U32 valid,session,root,parent,generation,reserved[3];} ForestHeader;
typedef struct {U32 offset,bytes;} ForestIndex;
typedef char ForestHeaderSize[sizeof(ForestHeader)==64?1:-1];
enum {FOREST_MAGIC=0x41465333u};

static U32 nested(U32 parent,U32 child){U32 i;
 while(child>parent){
  for(i=0;i<child;i++)if(child>=forest_heaps[i].child_first&&child-forest_heaps[i].child_first<forest_heaps[i].child_count)break;
  if(i==child)return 0;
  child=i;
 }
 return child==parent;
}
unsigned int forest_domain(const AsDomain*d){const AgHeap*h=forest_heaps+forest_current;
 return forest_scope&&d->heap==h->heap&&d->start==h->start&&d->span==h->span&&d->heap_generation==forest_scope->generation&&
  d->session==forest_scope->graph.session&&forest_meta(d,0x24)==h->parent&&forest_meta(d,0xc)==h->vtable&&
  forest_meta(d,0x30)==h->child_count&&forest_meta(d,0x48)==h->object_count;
}
/* Archive validation reads its immutable graph, never the old native heap.
 * Live capture/apply checks retain real reads and the separate graph gate. */
unsigned int forest_meta(const AsDomain*d,U32 off){const AgHeap*h=forest_heaps+forest_current;
 if(!forest_validating)return R32(d->heap,off);
 switch(off){case 12:return h->vtable;case 0x1c:return h->start;case 0x20:return h->span;
  case 0x24:return h->parent;case 0x30:return h->child_count;case 0x48:return h->object_count;
  case 0x50:return h->direction;case 0x90:return h->flags;default:return 0;}
}
unsigned int forest_begin(const AsDomain*d,U32 header){return d->start+(forest_meta(d,0x50)==1?header:0);}
unsigned int forest_end(const AsDomain*d,U32 header){return d->start+d->span-(forest_meta(d,0x50)==1?0:header);}
/* Return the next unprotected run, or a protected hole. No byte in a
 * descendant heap/header may be replayed by this heap's block archive. */
static U32 run_end(U32 at,U32 end,U32*hole){U32 i,k,a,n,result=end;*hole=0;
 for(i=forest_current+1;i<forest_count;i++)if(nested(forest_current,i)){
  for(k=0;k<2;k++){
   a=k?forest_heaps[i].heap:forest_heaps[i].start;n=k?heap_header(forest_heaps[i].kind):forest_heaps[i].span;
   if(at>=a&&at-a<n){if(!*hole||a+n>result)result=a+n;*hole=1;}
   else if(!*hole&&at<a&&a<result)result=a;
  }
 }
 return result<end?result:end;
}
void forest_read(void*out,U32 address,U32 n){U32 at=address,end=address+n,stop,hole;
 while(at<end){stop=run_end(at,end,&hole);if(hole)zero((U8*)out+at-address,stop-at);
  else copy((U8*)out+at-address,(const void*)at,stop-at);
  at=stop;}
}
void forest_write(U32 address,const void*in,U32 n){U32 at=address,end=address+n,stop,hole;
 while(at<end){stop=run_end(at,end,&hole);if(!hole)copy((void*)at,(const U8*)in+at-address,stop-at);at=stop;}
}
unsigned int forest_equal(U32 address,const void*in,U32 n){U32 at=address,end=address+n,stop,hole;
 while(at<end){stop=run_end(at,end,&hole);if(!hole&&!equal((const void*)at,(const U8*)in+at-address,stop-at))return 0;at=stop;}
 return 1;
}
static U32 forest_hash(const void*p,U32 n){U32 i,h=2166136261u;const U8*b=p;
 for(i=0;i<n;i++)h=(h^((i>=28&&i<32)?0:b[i]))*16777619u;
 return h;
}
static U32 forest_descriptor(const AfDomain*d){U32 i,z;
 if(!d||!_gz_abi_metadata_span((U32)d,sizeof(*d))||!d->generation)return AS_ARGUMENT;
 for(i=0;i<3;i++)if(d->reserved[i])return AS_ARGUMENT;
 z=0;return z;
}
static U32 forest_scratch(U32 p,U32 n,const AfDomain*d){
 if(!_gz_abi_metadata_span(p,n)||!apart(p,n,(U32)d,sizeof(*d))||!apart(p,n,(U32)rows,sizeof(rows))||
  !apart(p,n,(U32)&forest_scope,4)||!apart(p,n,(U32)&forest_heaps,4)||
  !apart(p,n,(U32)&forest_count,4)||!apart(p,n,(U32)&forest_current,4)||!apart(p,n,(U32)&forest_busy,4)||!apart(p,n,(U32)&forest_validating,4))return 0;
 return 1;
}
static U32 forest_private(U32 p,U32 n,const AfDomain*d){U32 i;
 if(!forest_scratch(p,n,d))return 0;
 for(i=0;i<forest_count;i++)if(!apart(p,n,forest_heaps[i].heap,heap_header(forest_heaps[i].kind))||!apart(p,n,forest_heaps[i].start,forest_heaps[i].span))return 0;
 return 1;
}
static U32 establish(const AfDomain*d,const void*graph,U32 bytes){U32 z,i,j;const AgHeap*h;
 z=_gz_ag_validate(&d->graph,graph,bytes);if(z)return z;
 forest_scope=d;forest_count=R32(graph,24);forest_heaps=(const AgHeap*)((const U8*)graph+64);
 for(i=0;i<forest_count;i++){
  h=forest_heaps+i;
  if(!heap_header(h->kind)||h->span<0xc8||(h->direction!=1&&h->direction!=0xffffffffu)||(h->kind==3&&h->direction!=1)||
   (h->direction==1?h->heap!=h->start:h->heap!=h->start+h->span-heap_header(h->kind)))return AS_LAYOUT;
  if(!apart((U32)d,sizeof(*d),h->heap,0xb8)||!apart((U32)d,sizeof(*d),h->start,h->span))return AS_ARGUMENT;
  for(j=0;j<i;j++){
   const AgHeap*p=forest_heaps+j;
   if(nested(j,i)){
    U32 begin=p->start+(p->direction==1?heap_header(p->kind):0);
    U32 end=p->start+p->span-(p->direction==1?0:heap_header(p->kind));
    if(h->start<begin||h->span>end-begin||h->start-begin>end-begin-h->span)return AS_LAYOUT;
   }else if(!apart(h->start,h->span,p->start,p->span))return AS_LAYOUT;
  }
 }
 return 0;
}
static void part_domain(const AfDomain*d,U32 index,AsDomain*out){const AgHeap*h=forest_heaps+index;
 zero(out,sizeof(*out));out->heap=h->heap;out->start=h->start;out->span=h->span;
 out->heap_generation=d->generation;out->session=d->graph.session;out->token=d->graph.token;out->obligations=d->graph.obligations;forest_current=index;
}
static U32 forest_capture_locked(const AfDomain*d,void*out,U32 capacity,U32*written){ForestHeader h;AsDomain part;U32 z,g,n,i,at;ForestIndex*ix;
 if(!_gz_image_write_allowed((U32)out,capacity)||!_gz_image_write_allowed((U32)written,4))return AS_ARGUMENT;
 z=forest_descriptor(d);if(z)return z;
 if(!forest_scratch((U32)out,capacity,d)||!forest_scratch((U32)written,4,d))return AS_ARGUMENT;
 if(capacity<64)return AS_SPACE;
 z=_gz_ag_capture(&d->graph,(U8*)out+64,capacity-64,&g);if(z)return z;
 z=establish(d,(U8*)out+64,g);if(z)return z;
 if(!forest_private((U32)out,capacity,d)||!forest_private((U32)written,4,d)||!apart((U32)out,capacity,(U32)written,4))return AS_ARGUMENT;
 *written=0;at=64+g+forest_count*sizeof(ForestIndex);if(at>capacity)return AS_SPACE;
 ix=(ForestIndex*)((U8*)out+64+g);
 for(i=0;i<forest_count;i++){
  part_domain(d,i,&part);z=capture_locked(&part,(U8*)out+at,capacity-at,&n);if(z)return z;
  z=validate_locked(&part,(U8*)out+at,n,0);if(z)return z;
  ix[i].offset=at;ix[i].bytes=n;at+=n;
 }
 z=_gz_ag_matches_live(&d->graph,(U8*)out+64,g);if(z)return z;
 zero(&h,sizeof(h));h.magic=FOREST_MAGIC;h.version=3;h.bytes=at;h.graph_bytes=g;h.count=forest_count;h.index=64+g;h.data=64+g+h.count*8;
 h.valid=1;h.session=d->graph.session;h.root=d->graph.root;h.parent=d->graph.parent;h.generation=d->generation;
 copy(out,&h,64);((ForestHeader*)out)->checksum=forest_hash(out,at);*written=at;return 0;
}
static U32 forest_validate_locked(const AfDomain*d,const void*image,U32 bytes,ForestHeader*out){ForestHeader h;AsDomain part;U32 z,i,at;const ForestIndex*ix;
 z=forest_descriptor(d);if(z)return z;
 if(bytes<64||!forest_scratch((U32)image,bytes,d))return AS_ARGUMENT;
 copy(&h,image,64);
 if(h.magic!=FOREST_MAGIC||h.version!=3||h.bytes!=bytes||h.valid!=1||h.session!=d->graph.session||h.root!=d->graph.root||h.parent!=d->graph.parent||h.generation!=d->generation||
  h.graph_bytes>bytes-64||h.index!=64+h.graph_bytes||h.count>AG_MAX_HEAPS||h.count>(bytes-h.index)/8||h.data!=h.index+h.count*8||h.checksum!=forest_hash(image,bytes))return AS_CORRUPT;
 for(i=0;i<3;i++)if(h.reserved[i])return AS_CORRUPT;
 z=establish(d,(const U8*)image+64,h.graph_bytes);if(z)return z;
 if(h.count!=forest_count||!forest_private((U32)image,bytes,d))return AS_CORRUPT;
 ix=(const ForestIndex*)((const U8*)image+h.index);at=h.data;
 for(i=0;i<h.count;i++){
  if(ix[i].offset!=at||ix[i].bytes>bytes-at)return AS_CORRUPT;
  part_domain(d,i,&part);forest_validating=1;z=validate_locked(&part,(const U8*)image+at,ix[i].bytes,0);forest_validating=0;
  if(z)return z;
  at+=ix[i].bytes;
 }
 if(at!=bytes)return AS_CORRUPT;
 if(out)copy(out,&h,64);
 return 0;
}
static U32 forest_apply(const AfDomain*d,const void*image,const ForestHeader*h){U32 i,z;AsDomain part;AsHeader ph;const ForestIndex*ix=(const ForestIndex*)((const U8*)image+h->index);
 z=establish(d,(const U8*)image+64,h->graph_bytes);if(z)return z;
 for(i=0;i<h->count;i++){
  part_domain(d,i,&part);copy(&ph,(const U8*)image+ix[i].offset,sizeof(ph));apply(&part,(const U8*)image+ix[i].offset,&ph);
 }
 for(i=0;i<h->count;i++){
  part_domain(d,i,&part);copy(&ph,(const U8*)image+ix[i].offset,sizeof(ph));if(!matches(&part,(const U8*)image+ix[i].offset,&ph))return AS_CHANGED;
 }
 return _gz_ag_matches_live(&d->graph,(const U8*)image+64,h->graph_bytes);
}
static void release(void){forest_scope=0;forest_heaps=0;forest_count=0;forest_current=0;forest_validating=0;__sync_synchronize();forest_busy=0;}
AgU32 _gz_af_capture(const AfDomain*d,void*out,U32 cap,U32*written){U32 z;
 if(!__sync_bool_compare_and_swap(&forest_busy,0,1))return AS_BUSY;
 z=forest_capture_locked(d,out,cap,written);release();return z;
}
AgU32 _gz_af_validate(const AfDomain*d,const void*p,U32 bytes){U32 z;
 if(!__sync_bool_compare_and_swap(&forest_busy,0,1))return AS_BUSY;
 z=forest_validate_locked(d,p,bytes,0);release();return z;
}
AgU32 _gz_af_restore(const AfDomain*d,const void*p,U32 bytes,void*undo,U32 cap,U32*written){ForestHeader h,u;U32 z,n;
 if(!_gz_image_write_allowed((U32)undo,cap)||!_gz_image_write_allowed((U32)written,4))return AS_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&forest_busy,0,1))return AS_BUSY;
 z=forest_validate_locked(d,p,bytes,&h);if(z)goto done;
 z=_gz_ag_matches_live(&d->graph,(const U8*)p+64,h.graph_bytes);if(z)goto done;
 if(!forest_private((U32)undo,cap,d)||!forest_private((U32)written,4,d)||!apart((U32)undo,cap,(U32)p,bytes)||
  !apart((U32)written,4,(U32)p,bytes)||!apart((U32)written,4,(U32)undo,cap)){z=AS_ARGUMENT;goto done;}
 z=forest_capture_locked(d,undo,cap,&n);if(z)goto done;
 z=forest_validate_locked(d,p,bytes,&h);if(z)goto done;
 copy(&u,undo,64);*written=n;z=forest_apply(d,p,&h);if(!z)goto done;
 z=forest_apply(d,undo,&u)?AS_ROLLBACK_FAILED:AS_APPLY_FAILED;
done:release();return z;
}
