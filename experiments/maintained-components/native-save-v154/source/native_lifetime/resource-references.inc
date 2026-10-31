/* Original EUR member methods027BF5AC..027BF988:
 * +00 table {data,u16 capacity,u16 count}; six-byte descriptor rows.
 * +04 active view, +08 backing, +0c per-buffer stride, +10 used bytes,
 * +14 ownership bits, +15 buffer count, +18 fixed member vtable10169D90.
 * Setter027BF794 initializes active/backing together. Factory027BF7A4
 * allocates stride*count, with256-byte alignment. These are base members;
 * no claim is made about a derived object's remaining fields or GPU state.
 */
#include "resource_references.h"
typedef char SpReferenceReportSize[sizeof(SpReferenceReport)==64?1:-1];
static u32 spr_output(u32 p,u32 bytes){u32 i;HlHeap*h;
 if(!sp_buffer(p,bytes))return 0;
 for(i=0;i<HL_HEAPS;i++){h=_gz_hl_heaps+i;
  if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)&&!hr_private(p,bytes,h))return 0;
 }return 1;
}
static const SpEntry*spr_edge(const ScView*v,const SpEntry*entries,const ScObject*o,u32 slot,u32 role){u32 i;const ScEdge*e;
 for(i=0;i<o->edge_count;i++){e=v->edges+o->edge_begin+i;if(e->slot==slot&&e->role==role)return entries+e->allocation;}
 return 0;
}
/* An unowned reference can be classified only when this immutable archive
 * already retains its whole extent. Missing extents stay unresolved; they
 * are never chased in old or current native memory. */
static u32 spr_retained(const ScView*v,u32 pointer,u32 bytes){u32 i,n=0;const HlAllocation*a;
 if(!bytes||pointer>0xffffffffu-bytes)return 0;
 for(i=0;i<v->header->allocation_count;i++){a=&v->allocations[i].identity;
  if(pointer>=a->pointer&&bytes<=a->capacity&&pointer-a->pointer<=a->capacity-bytes)n++;
 }return n==1;
}
static u32 spr_member(const unsigned char*b,const ScView*v,const SpEntry*entries,const ScObject*o,u32 slot,SpReferenceReport*r){
 const unsigned char*m=o->raw+16+slot*28,*t;const SpEntry*e;const HlAllocation*a;u32 flags=m[20],count=m[21],base=sp_read_u32(m+8),active=sp_read_u32(m+4),stride=sp_read_u32(m+12),used=sp_read_u32(m+16),table=sp_read_u32(m),extent,offset,cap,n,data;
 r->members++;
 if((flags&~3u)||m[22]||m[23]||sp_read_u32(m+24)!=0x10169d90u)return SC_FIELD;
 if(!base){if(active||stride||used||count||(flags&2))return SC_FIELD;}
 else{
  if(!count||!stride||used>stride||stride>0xffffffffu/count)return SC_FIELD;
  extent=stride*count;if(active<base)return SC_FIELD;offset=active-base;
  if(offset>=extent||offset%stride)return SC_FIELD;
  if(flags&2){e=spr_edge(v,entries,o,slot,RD_OWNED_ARRAY);if(!e)return SC_FIELD;
   a=&v->allocations[e->allocation].identity;if(a->pointer!=base||extent>a->capacity)return SC_FIELD;r->owned_views++;
  }else{r->borrowed_views++;if(!spr_retained(v,base,extent))r->unresolved_references++;}
 }
 if(flags&1){
  e=spr_edge(v,entries,o,slot,RD_OWNED_TABLE);if(!e||e->bytes<8)return SC_FIELD;
  a=&v->allocations[e->allocation].identity;if(a->pointer!=table)return SC_FIELD;
  t=b+e->offset;data=sp_read_u32(t);cap=((u32)t[4]<<8)|t[5];n=((u32)t[6]<<8)|t[7];
  if(n>cap)return SC_FIELD;
  if(data){e=spr_edge(v,entries,o,slot,RD_OWNED_TABLE_DATA);if(!e||e->bytes<cap*6)return SC_FIELD;}
  else if(cap)return SC_FIELD;
  r->table_headers++;
 }else if(table){if(!spr_retained(v,table,8))r->unresolved_references++;}
 return 0;
}
u32 _gz_sp_references(const void*image,u32 bytes,SpReferenceReport*out){
 SpReferenceReport r;ScView v;const SpEntry*entries;const unsigned char*payload;u32 z,i,j,checksum;
 if(!_gz_image_write_allowed((u32)out,sizeof(*out)))return SC_ARGUMENT;
 if(bytes<192||!sp_buffer((u32)image,bytes)||!spr_output((u32)out,sizeof(*out))||!hr_apart((u32)image,bytes,(u32)out,sizeof(*out)))return SC_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&sc_busy,0,1))return SC_BUSY;
 zero(&r,sizeof(r));r.magic=0x53505231u;r.bytes=sizeof(r);r.version=1;r.missing=31;
 z=sp_validate_image(image,bytes);if(z)goto done;
 checksum=((const SpHeader*)image)->checksum;r.source_checksum=checksum;
 z=sp_view(image,bytes,&v,&entries,&payload);if(z)goto done;
 r.objects=v.header->object_count;r.pending_derived=r.objects;
 for(i=0;i<r.objects;i++)if(v.objects[i].provider){
  r.known_objects++;for(j=0;j<2;j++){z=spr_member(image,&v,entries,v.objects+i,j,&r);if(z)goto done;}
 }
 if(sp_hash(image,bytes)!=checksum){z=SC_CHANGED;goto done;}
 r.valid=1;
done:r.code=z;copy(out,&r,sizeof(r));__sync_synchronize();sc_busy=0;return z;
}
