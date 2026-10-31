#include "resource_payload.h"
typedef char SpHeaderSize[sizeof(SpHeader)==64?1:-1];
typedef char SpEntrySize[sizeof(SpEntry)==16?1:-1];
static u32 sp_hash(const void*p,u32 n){u32 i,hash=2166136261u;const unsigned char*b=p;
 for(i=0;i<n;i++)hash=(hash^((i>=16&&i<20)?0:(i>=12&&i<16)?(i==15):b[i]))*16777619u;return hash;
}
static u32 sp_plain_hash(const void*p,u32 n){return sc_hash_part(2166136261u,p,n);}
static void sp_copy_bytes(void*d,const void*s,u32 n){u32 i;for(i=0;i<n;i++)((unsigned char*)d)[i]=((const unsigned char*)s)[i];}
static u32 sp_read_u32(const unsigned char*p){return ((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3];}
static u32 sp_buffer(u32 p,u32 n){return hr_private(p,n,0);}
static u32 sp_outputs_private(u32 out,u32 n,u32 written){u32 i;HlHeap*h;
 if(!sp_buffer(out,n)||!sp_buffer(written,4)||!hr_apart(out,n,written,4))return 0;
 for(i=0;i<HL_HEAPS;i++){h=_gz_hl_heaps+i;if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)&&
  (!hr_private(out,n,h)||!hr_private(written,4,h)))return 0;}
 return sc_outputs_private();
}
static void sp_pack(unsigned char*target,const ScDirectory*d){u32 offset=128;copy(target,&d->header,128);
 copy(target+offset,d->heaps,d->header.heap_count*sizeof(ScHeap));offset+=d->header.heap_count*sizeof(ScHeap);
 copy(target+offset,d->allocations,d->header.allocation_count*sizeof(ScAllocation));offset+=d->header.allocation_count*sizeof(ScAllocation);
 copy(target+offset,d->objects,d->header.object_count*sizeof(ScObject));offset+=d->header.object_count*sizeof(ScObject);
 copy(target+offset,d->edges,d->header.edge_count*sizeof(ScEdge));offset+=d->header.edge_count*sizeof(ScEdge);
 copy(target+offset,d->registry,d->header.registry_count*sizeof(ScRegistry));
}
static u32 sp_view(const void*image,u32 bytes,ScView*out,const SpEntry**entries,const unsigned char**payload){const SpHeader*s=image;const unsigned char*b=image;const ScHeader*h;u32 offset,metadata;
 if(bytes<64+128||s->magic!=0x53504231u||s->bytes!=bytes||s->version!=1||s->valid!=1||s->metadata_offset!=64||
  s->missing!=31||s->session||s->writer_exclusion||s->reserved[0]||s->reserved[1]||s->metadata_bytes<128||s->metadata_bytes>bytes-64)return SC_IMAGE;
 h=(const ScHeader*)(b+64);
 if(!h->heap_count||h->heap_count>HL_HEAPS||!h->allocation_count||h->allocation_count>SC_ALLOCS||h->object_count>SC_OBJECTS||
  h->edge_count>SC_EDGES||h->registry_count>SC_REGISTRY)return SC_IMAGE;
 metadata=sc_used(h);if(s->metadata_bytes!=metadata||metadata!=h->used_bytes||s->index_offset!=64+metadata||
  s->count!=h->allocation_count||s->count>(bytes-s->index_offset)/16||s->payload_offset!=s->index_offset+s->count*16||
  s->payload_bytes!=bytes-s->payload_offset)return SC_IMAGE;
 out->header=h;offset=64+128;out->heaps=(const ScHeap*)(b+offset);offset+=h->heap_count*sizeof(ScHeap);
 out->allocations=(const ScAllocation*)(b+offset);offset+=h->allocation_count*sizeof(ScAllocation);
 out->objects=(const ScObject*)(b+offset);offset+=h->object_count*sizeof(ScObject);
 out->edges=(const ScEdge*)(b+offset);offset+=h->edge_count*sizeof(ScEdge);out->registry=(const ScRegistry*)(b+offset);
 *entries=(const SpEntry*)(b+s->index_offset);*payload=b+s->payload_offset;return 0;
}
static u32 sp_validate_image(const void*image,u32 bytes){ScView view;const SpEntry*entries;const unsigned char*payload,*b=image;const SpHeader*h=image;const HlAllocation*a;const ScObject*o;const ScEdge*edge;const SpEntry*e;u32 z,i,j,offset=0;
 z=sp_view(image,bytes,&view,&entries,&payload);if(z)return z;
 if(sp_hash(image,bytes)!=h->checksum)return SC_IMAGE;
 z=sc_validate_view(&view,sizeof(ScDirectory));if(z)return z;
 for(i=0;i<h->count;i++){
  e=entries+i;a=&view.allocations[i].identity;
  if(e->allocation!=i||e->offset!=h->payload_offset+offset||e->bytes!=a->capacity||e->bytes>h->payload_bytes-offset||
   sp_plain_hash(payload+offset,e->bytes)!=e->checksum)return SC_IMAGE;
  offset+=e->bytes;
 }if(offset!=h->payload_bytes)return SC_IMAGE;
 for(i=0;i<view.header->object_count;i++){
  o=view.objects+i;e=entries+o->allocation;
  if(!rd_equal(b+e->offset+o->offset,o->raw,o->provider?72:16))return SC_IMAGE;
  for(j=0;j<o->edge_count;j++){edge=view.edges+o->edge_begin+j;
   if(edge->role==RD_OWNED_TABLE){e=entries+edge->allocation;if(e->bytes<4||sp_read_u32(b+e->offset)!=o->table[edge->slot])return SC_IMAGE;}
  }
 }return 0;
}
u32 _gz_sp_validate(const void*image,u32 bytes){u32 z;
 if(bytes<64+128||!sp_buffer((u32)image,bytes))return SC_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&sc_busy,0,1))return SC_BUSY;z=sp_validate_image(image,bytes);__sync_synchronize();sc_busy=0;return z;
}
u32 _gz_sp_capture(u32 object,u32 owner,void*image,u32 capacity,u32*written){ScDirectory*d;ScHeader*source;SpHeader*h=image;SpEntry*index;
 unsigned char*b=image;u32 z=SC_ARGUMENT,i,need,offset,metadata;HlAllocation*a;
 if(!_gz_sc_storage_ready())return SC_BUSY;
 d=&_gz_sc_directory;source=&d->header;
 if(!_gz_image_write_allowed((u32)image,capacity)||!_gz_image_write_allowed((u32)written,4))return SC_ARGUMENT;
 if(capacity<64||!span(object,16)||!sp_buffer((u32)image,capacity)||!sp_buffer((u32)written,4)||!hr_apart((u32)image,capacity,(u32)written,4))return SC_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&sc_busy,0,1))return SC_BUSY;
 if(!lock()){sc_busy=0;return SC_OBSERVER;}
 if(!sp_outputs_private((u32)image,capacity,(u32)written))goto unlock_only;
 /* The caller must detach/replace a published slot using a new leased buffer. */
 if(h->magic==0x53504231u&&h->valid==1){z=SC_BUSY;goto unlock_only;}
 z=sc_capture_locked(object,owner);if(z)goto unlock_only;
 metadata=sc_used(source);need=64+metadata+source->allocation_count*sizeof(SpEntry);
 if(need>capacity){z=SC_CAPACITY;goto unlock_only;}
 for(i=0;i<source->allocation_count;i++){a=&d->allocations[i].identity;
  if(!span(a->pointer,a->capacity)){z=SC_UNTRACKED;goto unlock_only;}
  if(a->capacity>capacity-need){z=SC_CAPACITY;goto unlock_only;}need+=a->capacity;
 }
 zero(h,sizeof(*h));h->magic=0x53504231u;h->bytes=need;h->version=1;h->metadata_offset=64;h->metadata_bytes=metadata;
 h->index_offset=64+metadata;h->count=source->allocation_count;h->payload_offset=h->index_offset+h->count*16;h->payload_bytes=need-h->payload_offset;h->missing=31;
 sp_pack(b+64,d);index=(SpEntry*)(b+h->index_offset);offset=h->payload_offset;
 for(i=0;i<h->count;i++){a=&d->allocations[i].identity;index[i].allocation=i;index[i].offset=offset;index[i].bytes=a->capacity;
  sp_copy_bytes(b+offset,(const void*)a->pointer,a->capacity);index[i].checksum=sp_plain_hash(b+offset,a->capacity);offset+=a->capacity;
 }
 z=sc_recheck();if(z)goto invalidate;
 for(i=0;i<h->count;i++){a=&d->allocations[i].identity;if(!rd_equal((const void*)a->pointer,b+index[i].offset,a->capacity)){z=SC_CHANGED;goto invalidate;}}
 /* Validate all metadata/payload relationships before final publication. The
  * transient valid bit is private staging under sc_busy, not a slot commit. */
 h->checksum=sp_hash(image,need);h->valid=1;z=sp_validate_image(image,need);h->valid=0;if(z)goto invalidate;
 z=sc_recheck();if(z)goto invalidate;
 for(i=0;i<h->count;i++){a=&d->allocations[i].identity;if(!rd_equal((const void*)a->pointer,b+index[i].offset,a->capacity)){z=SC_CHANGED;goto invalidate;}}
 __sync_synchronize();h->valid=1;*written=need;z=0;goto unlock_only;
invalidate:h->valid=0;h->checksum=0;
unlock_only:si_end();unlock();__sync_synchronize();sc_busy=0;return z;
}
