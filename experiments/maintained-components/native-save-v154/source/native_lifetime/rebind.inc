/* Included after the CURRENT observer and mutator implementation. This path
 * repairs observation identities after validated bulk heap replay, without
 * replaying old generations, OS state or native allocation callbacks. */
#include "rebind.h"
#include "source_directory.h"
#include "native_abi.h"
typedef struct {u32 magic,version,bytes,heap,kind,generation,epoch,count;
 u32 checksum,valid,serial,release_serial,start,span,parent,direction;} HrHeader;
typedef struct {u32 start,bytes,used;} HrRange;
typedef char HrHeaderSize[sizeof(HrHeader)==64?1:-1];
typedef char HrMapSize[sizeof(HrMap)==32?1:-1];
typedef char HrReceiptSize[sizeof(HrReceipt)==48?1:-1];
static HlAllocation hr_rows[HR_MAX_ALLOCS];
static HrRange hr_ranges[HR_MAX_BLOCKS];
static u32 hr_slots[HR_MAX_ALLOCS],hr_reserved;
static u32 hr_marks[(HL_ALLOCS+31)/32];
#define HR_MAGIC 0x48525031u
static u32 hr_apart(u32 a,u32 n,u32 b,u32 m){return a>=b+m||b>=a+n;}
extern u32 _gz_jp_private(u32,u32);
extern u32 _gz_ghc_all_private_apart(u32,u32);
static u32 hr_private_base_without_jp(u32 p,u32 n,const HlHeap*h){
 if(!_gz_ghc_all_private_apart(p,n)||!_gz_abi_metadata_span(p,n)||!hr_apart(p,n,(u32)&_gz_hl_control,sizeof(_gz_hl_control))||
  !hr_apart(p,n,(u32)_gz_hl_heaps,sizeof(_gz_hl_heaps))||!hr_apart(p,n,(u32)_gz_hl_allocations,sizeof(_gz_hl_allocations))||
  !hr_apart(p,n,(u32)&_gz_hl_observation,sizeof(_gz_hl_observation))||!hr_apart(p,n,(u32)_gz_hl_projected,sizeof(_gz_hl_projected))||!hr_apart(p,n,(u32)_gz_hl_custom_meta,sizeof(_gz_hl_custom_meta))||
  !hr_apart(p,n,(u32)_gz_hm_scopes,sizeof(_gz_hm_scopes))||!hr_apart(p,n,(u32)_gz_hm_records,sizeof(_gz_hm_records))||
  !hr_apart(p,n,(u32)&_gz_hm_control,sizeof(_gz_hm_control))||!hr_apart(p,n,(u32)hr_rows,sizeof(hr_rows))||
  !hr_apart(p,n,(u32)_gz_hi_heaps,sizeof(_gz_hi_heaps))||!hr_apart(p,n,(u32)_gz_hi_links,sizeof(_gz_hi_links))||
  !hr_apart(p,n,(u32)&_gz_hi_control,sizeof(_gz_hi_control))||
  !hr_apart(p,n,(u32)hr_ranges,sizeof(hr_ranges))||!hr_apart(p,n,(u32)hr_slots,sizeof(hr_slots))||
  !hr_apart(p,n,(u32)&hr_reserved,4)||!hr_apart(p,n,(u32)hr_marks,sizeof(hr_marks)))return 0;
 return !h||(hr_apart(p,n,h->start,h->bytes)&&hr_apart(p,n,h->heap,header_bytes(h->kind)));
}
static u32 hr_private_base(u32 p,u32 n,const HlHeap*h){return _gz_jp_private(p,n)&&hr_private_base_without_jp(p,n,h);}
static u32 hr_private(u32 p,u32 n,const HlHeap*h){return hr_private_base(p,n,h)&&hr_apart(p,n,(u32)&_gz_rd_report,sizeof(_gz_rd_report))&&_gz_sc_private(p,n);}
static u32 hr_healthy(void);
u32 _gz_hr_storage_private(const void*p,u32 n){u32 i,ok=0,hrp_active;HlHeap*h;HrpInvocation hrp_invocation;HrpTicks hrp_started=0;
 if(!hr_private((u32)p,n,0)||!lock())return 0;
 hrp_active=hrp_window(&hrp_invocation);if(hrp_active)hrp_started=hrp_clock();
 if(!hr_healthy())goto done;
 for(i=0;i<HL_HEAPS;i++){h=_gz_hl_heaps+i;
  if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)&&!hr_private((u32)p,n,h))goto done;
 }ok=1;
done:if(hrp_active)hrp_note(2,&hrp_invocation,hrp_started);unlock();return ok;
}
/* Fixed allocator query, not an actor/buffer/restore permission and not a
 * caller-selected skip flag. The exact prospective SDK span must be held by
 * JP's private admission scope. Every heap/metadata exclusion still runs. */
extern u32 _gz_jp_admission_active(u32,u32);
static u32 hr_private_for_jp(u32 p,u32 n,const HlHeap*h){return hr_private_base_without_jp(p,n,h)&&hr_apart(p,n,(u32)&_gz_rd_report,sizeof(_gz_rd_report))&&_gz_sc_private(p,n);}
u32 _gz_hr_storage_private_for_jp(const void*p,u32 n){u32 i,ok=0,hrp_active;HlHeap*h;HrpInvocation hrp_invocation;HrpTicks hrp_started=0;
 if(!_gz_jp_admission_active((u32)p,n)||!hr_private_for_jp((u32)p,n,0)||!lock())return 0;
 hrp_active=hrp_window(&hrp_invocation);if(hrp_active)hrp_started=hrp_clock();
 if(!hr_healthy())goto done;
 for(i=0;i<HL_HEAPS;i++){h=_gz_hl_heaps+i;
  if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)&&!hr_private_for_jp((u32)p,n,h))goto done;
 }ok=1;
done:if(hrp_active)hrp_note(2,&hrp_invocation,hrp_started);unlock();return ok&&_gz_jp_admission_active((u32)p,n);
}
static u32 hr_hash(const void*p,u32 n){u32 i,h=2166136261u;const unsigned char*b=p;
 for(i=0;i<n;i++)h=(h^((i>=32&&i<36)?0:(i>=36&&i<40)?(i==39):b[i]))*16777619u;
 return h;
}
static u32 hr_healthy(void){return !_gz_hl_control.flags&&hi_header()&&_gz_hi_control.nodes==_gz_hl_control.active_allocations&&
 hm_header()&&!_gz_hm_control.active&&!_gz_hm_control.failed;}
HrU32 _gz_hr_buffer_private(const void*p,u32 n){return hr_private((u32)p,n,0);}
HrU32 _gz_hr_stamp(u32 out[4]){u32 z=HR_OBSERVER;
 if(!_gz_image_write_allowed((u32)out,16))return HR_ARGUMENT;
 if(!lock())return HR_OBSERVER;
 if(hr_healthy()&&hr_private((u32)out,16,0)){
  out[0]=_gz_hl_control.serial;out[1]=_gz_hl_control.release_serial;
  out[2]=_gz_hl_control.active_heaps;out[3]=_gz_hl_control.active_allocations;z=0;
 }unlock();return z;
}
HrU32 _gz_hr_heap_identity(u32 heap,u32 out[2]){u32 z=HR_OBSERVER;HlHeap*h;
 if(!_gz_image_write_allowed((u32)out,8))return HR_ARGUMENT;
 if(!lock())return HR_OBSERVER;h=heap_find(heap,0);
 if(hr_healthy()&&h&&h->state==HL_LIVE&&hr_private((u32)out,8,0)){
  out[0]=h->generation;out[1]=h->epoch;z=0;
 }unlock();return z;
}
static u32 hr_native_header(const HlHeap*h){return span(h->heap,header_bytes(h->kind))&&
 R32(h->heap,12)==h->vtable&&R32(h->heap,0x1c)==h->start&&R32(h->heap,0x20)==h->bytes&&
 R32(h->heap,0x24)==h->parent&&R32(h->heap,0x50)==h->direction&&!(R32(h->heap,0x90)&2);}
/* A parent's bulk replay deliberately invalidates the cached child anchors.
 * Re-prove the current parent/backing without editing the cache first. This
 * also supports retained child headers; no generation is invented for them. */
static u32 hr_parent_ready(HlHeap*h){HlHeap*p;HlStoredAllocation*a;
 if(!h->parent)return 1;
 p=heap_find(h->parent,0);
 if(!p||p->state!=HL_LIVE||p->generation!=h->parent_generation||!_gz_hm_scope_live(p))return 0;
 a=hm_anchor(h->start,h->bytes,h->parent);
 return a&&within(a->pointer,a->capacity,h->heap,header_bytes(h->kind));
}
static u32 hr_row(const HlHeap*h,const HlAllocation*a){u32 norm;
 if(a->state!=HL_LIVE||a->kind!=h->kind||a->heap!=h->heap||a->heap_generation!=h->generation||a->heap_epoch!=h->epoch||
  !a->generation||a->requested>0xfffffffcu||a->missing!=HL_MISSING_ALL||a->reserved||a->release_token||
  /* Reject unrepresentable source flags during hr_image, before the first
   * rebind mutation. All current native allocation provenance fits9bits. */
  a->flags>511u)return 0;
 norm=((a->requested<4?4:a->requested)+3)&~3u;
 if(!norm||a->capacity<norm||!within(h->start,h->bytes,a->pointer,a->capacity))return 0;
 if(h->kind==HL_EXP)return within(h->start,h->bytes,a->block,16)&&a->block<=a->pointer&&a->pointer-a->block>=16&&a->padding==a->pointer-a->block-16&&
  within(h->start,h->bytes,a->block,16+a->padding+a->capacity);
 return (h->kind==HL_UNIT||h->kind==HL_SOLID)&&a->block==a->pointer&&!a->padding;
}
static u32 hr_find(u32 p,u32 n){u32 lo=0,hi=n,mid;
 while(lo<hi){mid=lo+(hi-lo)/2;if(hr_rows[mid].pointer==p)return mid;
  if(hr_rows[mid].pointer<p)lo=mid+1;else hi=mid;}
 return n;
}
static void hr_row_sift(u32 root,u32 n){HlAllocation value;u32 child;copy(&value,hr_rows+root,sizeof(value));
 while(root<n/2){child=root*2+1;if(child+1<n&&hr_rows[child].pointer<hr_rows[child+1].pointer)child++;
  if(value.pointer>=hr_rows[child].pointer)break;copy(hr_rows+root,hr_rows+child,sizeof(value));root=child;
 }copy(hr_rows+root,&value,sizeof(value));
}
static void hr_row_sort(u32 n){u32 start=n/2,end=n;HlAllocation temp;
 while(start){--start;hr_row_sift(start,n);}
 while(end>1){--end;copy(&temp,hr_rows,sizeof(temp));copy(hr_rows,hr_rows+end,sizeof(temp));copy(hr_rows+end,&temp,sizeof(temp));hr_row_sift(0,end);}
}
static void hr_range_sift(u32 root,u32 n){HrRange value=hr_ranges[root];u32 child;
 while(root<n/2){child=root*2+1;if(child+1<n&&hr_ranges[child].start<hr_ranges[child+1].start)child++;
  if(value.start>=hr_ranges[child].start)break;hr_ranges[root]=hr_ranges[child];root=child;
 }hr_ranges[root]=value;
}
static void hr_range_sort(u32 n){u32 start=n/2,end=n;HrRange temp;
 while(start){--start;hr_range_sift(start,n);}
 while(end>1){--end;temp=hr_ranges[0];hr_ranges[0]=hr_ranges[end];hr_ranges[end]=temp;hr_range_sift(0,end);}
}
static u32 hr_allocator(const HlHeap*h,u32 n){u32 i,j,k,count,head,node,prev,size,pad,p,at,total=0,z=0,begin,end,first,stride,raw,free,slots;HrRange temp;
 begin=h->start+(h->direction==1?header_bytes(h->kind):0);end=h->start+h->bytes-(h->direction==1?0:header_bytes(h->kind));
 if(h->kind==HL_SOLID){
  first=R32(h->heap,0x94);at=R32(h->heap,0x98);if(first<begin||first>at||at>end)return HR_LAYOUT;
  for(i=0;i<n;i++)if(!((hr_rows[i].pointer>=begin&&hr_rows[i].pointer<=first&&hr_rows[i].capacity<=first-hr_rows[i].pointer)||
   (hr_rows[i].pointer>=at&&hr_rows[i].pointer<=end&&hr_rows[i].capacity<=end-hr_rows[i].pointer)))return HR_LAYOUT;
  return 0;
 }
 if(h->kind==HL_UNIT){
  stride=R32(h->heap,0x94);first=R32(h->heap,0x98);raw=R32(h->heap,0x9c);free=R32(h->heap,0xa0);
  if(h->direction!=1||stride<4||(stride&3)||(first&3)||first<begin||first>end||raw!=end-first||!raw||stride>raw||
   R32(h->heap,0xa8)!=first||free>raw||free%stride)return HR_LAYOUT;
  slots=raw/stride;if(slots>HR_MAX_BLOCKS||free/stride>slots||slots-free/stride!=n)return HR_LIMIT;
  for(i=0;i<slots;i++)hr_ranges[i].used=0;
  for(i=0;i<n;i++){
   p=hr_rows[i].pointer;if(p<first||p-first>=slots*stride||(p-first)%stride||hr_rows[i].capacity!=stride)return HR_LAYOUT;
   k=(p-first)/stride;if(hr_ranges[k].used)return HR_LAYOUT;hr_ranges[k].used=1;
  }
  node=R32(h->heap,0xa4);
  for(i=0;i<free/stride;i++){
   if(node<first||node-first>=slots*stride||(node-first)%stride)return HR_LAYOUT;
   k=(node-first)/stride;if(hr_ranges[k].used)return HR_LAYOUT;hr_ranges[k].used=2;node=R32(node,0);
  }
  return node?HR_LAYOUT:0;
 }
 if(h->kind!=HL_EXP)return HR_LAYOUT;
 for(k=0;k<2;k++){
  head=h->heap+(k?0xa8:0x98);count=R32(head,8);node=R32(head,4);prev=head;
  if(count>HR_MAX_BLOCKS-total||R32(head,12)||(k&&count!=n))return HR_LIMIT;
  for(i=0;i<count;i++){
   if(!within(begin,end-begin,node,16)||R32(node,0)!=prev)return HR_LAYOUT;
   size=R32(node,8);pad=R32(node,12);
   if((size&3)||(pad&3)||(!k&&pad)||size>end-node-16||pad>end-node-16-size)return HR_LAYOUT;
   hr_ranges[total].start=node;hr_ranges[total].bytes=16+pad+size;hr_ranges[total++].used=k;
   if(k){
    p=node+16+pad;j=hr_find(p,n);if(j==n||hr_rows[j].block!=node||hr_rows[j].padding!=pad||hr_rows[j].capacity!=size)return HR_LAYOUT;
    if(pad&&R32(p,-4)!=(node|1u))return HR_LAYOUT;z++;
   }
   prev=node;node=R32(node,4);
  }
  if(node!=head||R32(head,0)!=prev)return HR_LAYOUT;
 }
 hr_range_sort(total);
 at=begin;for(i=0;i<total;i++){if(hr_ranges[i].start!=at)return HR_LAYOUT;at+=hr_ranges[i].bytes;}
 return at==end&&z==n?0:HR_LAYOUT;
}
static u32 hr_image(const void*p,u32 bytes,HrHeader*out,HlHeap*h){u32 i;HrHeader x;
 if(bytes<128||!hr_private((u32)p,bytes,0))return HR_ARGUMENT;
 copy(&x,p,64);copy(h,(const unsigned char*)p+64,64);
 if(x.magic!=HR_MAGIC||x.version!=1||x.valid!=1||x.bytes!=bytes||x.count>HR_MAX_ALLOCS||bytes!=128+x.count*sizeof(HlAllocation)||
  x.heap!=h->heap||x.kind!=h->kind||x.generation!=h->generation||x.epoch!=h->epoch||x.start!=h->start||x.span!=h->bytes||
  x.parent!=h->parent||x.direction!=h->direction||!h->generation||x.serial<h->generation||x.release_serial<h->epoch||
  (h->parent?(!h->parent_generation||h->parent_generation>=h->generation):h->parent_generation!=0)||
  h->state!=HL_LIVE||h->kind<HL_EXP||h->kind>HL_UNIT||
  h->vtable!=(h->kind==HL_EXP?(u32)_gz_hl_exp_table:h->kind==HL_SOLID?(u32)_gz_hl_solid_table:(u32)_gz_hl_unit_table)||
  !span(h->heap,header_bytes(h->kind))||!span(h->start,h->bytes)||h->bytes<header_bytes(h->kind)+16||
  (h->direction!=1&&h->direction!=0xffffffffu)||(h->kind==HL_UNIT&&h->direction!=1)||!hr_private((u32)p,bytes,h)||
  x.checksum!=hr_hash(p,bytes))return HR_CORRUPT;
 copy(hr_rows,(const unsigned char*)p+128,x.count*sizeof(HlAllocation));
 for(i=0;i<x.count;i++)if(!hr_row(h,hr_rows+i)||hr_rows[i].generation>x.serial||(i&&(hr_rows[i-1].pointer>=hr_rows[i].pointer||
  hr_rows[i-1].capacity>hr_rows[i].pointer-hr_rows[i-1].pointer)))return HR_CORRUPT;
 if(out)copy(out,&x,64);return 0;
}
HrU32 _gz_hr_validate(const void*p,u32 bytes){u32 z;HlHeap h;
 if(!lock())return HR_OBSERVER;z=hr_image(p,bytes,0,&h);unlock();return z;
}
HrU32 _gz_hr_capture(u32 heap,void*p,u32 capacity,u32*written){HlHeap*h;HrHeader out;HlAllocation t;u32 i,j,n=0,z=HR_OBSERVER;
 if(!_gz_image_write_allowed((u32)p,capacity)||!_gz_image_write_allowed((u32)written,4))return HR_ARGUMENT;
 if(!lock())return HR_OBSERVER;
 h=heap_find(heap,0);
 if(!hr_healthy()||!h||h->state!=HL_LIVE||h->kind<HL_EXP||h->kind>HL_UNIT||!_gz_hm_scope_live(h))goto done;
 if(!hr_private((u32)p,capacity,h)||!hr_private((u32)written,4,h)||!hr_apart((u32)p,capacity,(u32)written,4)){z=HR_ARGUMENT;goto done;}
 *written=0;if(!hr_native_header(h)){z=HR_CHANGED;goto done;}
 if(!hi_ready(h)){z=HR_OBSERVER;goto done;}
 for(i=hi_head(h);i;i=hi_next(i)){
  HlStoredAllocation*a=_gz_hl_allocations+i-1;
  if(a->state==HL_ENDING){z=HR_OBSERVER;goto done;}
  if(a->state!=HL_LIVE)continue;
  hl_export(&t,a);if(!hr_row(h,&t)){z=HR_CHANGED;goto done;}if(n==HR_MAX_ALLOCS){z=HR_LIMIT;goto done;}
  copy(hr_rows+n++,&t,sizeof(t));
 }
 hr_row_sort(n);
 for(i=1;i<n;i++)if(hr_rows[i-1].pointer>=hr_rows[i].pointer||hr_rows[i-1].capacity>hr_rows[i].pointer-hr_rows[i-1].pointer){z=HR_LAYOUT;goto done;}
 z=hr_allocator(h,n);if(z)goto done;
 if(capacity<128+n*sizeof(HlAllocation)){z=HR_SPACE;goto done;}
 zero(&out,sizeof(out));out.magic=HR_MAGIC;out.version=1;out.bytes=128+n*sizeof(HlAllocation);out.heap=heap;
 out.kind=h->kind;out.generation=h->generation;out.epoch=h->epoch;out.count=n;
 out.serial=_gz_hl_control.serial;out.release_serial=_gz_hl_control.release_serial;out.start=h->start;out.span=h->bytes;out.parent=h->parent;out.direction=h->direction;
 copy(p,&out,64);copy((unsigned char*)p+64,h,64);copy((unsigned char*)p+128,hr_rows,n*sizeof(HlAllocation));
 if(!hr_native_header(h)||hr_allocator(h,n)){z=HR_CHANGED;goto done;}
 ((HrHeader*)p)->checksum=hr_hash(p,out.bytes);__sync_synchronize();((HrHeader*)p)->valid=1;*written=out.bytes;z=0;
done:unlock();return z;
}
static u32 hr_taken(u32 index){return (hr_marks[index/32]>>(index%32))&1u;}
static void hr_take(u32 index){hr_marks[index/32]|=1u<<(index%32);}
/* Reserve all existing exact keys first. Then reserve insert positions, so
 * collision/tombstone reuse cannot steal an entry required by a later row. */
static u32 hr_plan(u32 heap,u32 n){u32 i,j,k,start,found;HlStoredAllocation*a;
 zero(hr_marks,sizeof(hr_marks));
 for(i=0;i<n;i++)hr_slots[i]=HL_ALLOCS;
 for(i=0;i<n;i++){
  start=((hr_rows[i].pointer>>2)*0x9e3779b1u)>>14;found=HL_ALLOCS;
  for(j=0;j<HL_ALLOCS;j++){
   k=(start+j)&(HL_ALLOCS-1);a=_gz_hl_allocations+k;if(!a->pointer)break;
   if(a->pointer==hr_rows[i].pointer&&a->heap==heap){if(found!=HL_ALLOCS)return HR_OBSERVER;found=k;}
  }
  if(found!=HL_ALLOCS){if(hr_taken(found))return HR_OBSERVER;hr_slots[i]=found;hr_take(found);}
 }
 for(i=0;i<n;i++)if(hr_slots[i]==HL_ALLOCS){
  start=((hr_rows[i].pointer>>2)*0x9e3779b1u)>>14;
  for(j=0;j<HL_ALLOCS;j++){
   k=(start+j)&(HL_ALLOCS-1);a=_gz_hl_allocations+k;if(hr_taken(k))continue;
   if(!a->pointer||a->state==HL_DEAD||(a->heap==heap&&a->state==HL_LIVE)){hr_slots[i]=k;hr_take(k);break;}
  }
  if(j==HL_ALLOCS)return HR_SPACE;
 }
 return 0;
}
HrU32 _gz_hr_rebind(const void*p,u32 bytes,u32 generation,u32 epoch,HrMap*map,u32 capacity,HrReceipt*receipt){
 HrHeader image;HlHeap source,*h=0;HrReceipt out;HlAllocation a;HlStoredAllocation stored;u32 z,i,next,retiring=0,new_epoch,serial_before,result_ok=0;
 if(!_gz_image_write_allowed((u32)map,capacity)||!_gz_image_write_allowed((u32)receipt,sizeof(*receipt)))return HR_ARGUMENT;
 if(!lock())return HR_OBSERVER;
 z=hr_image(p,bytes,&image,&source);if(z)goto done;
 if(image.count>capacity/sizeof(HrMap)||!hr_private((u32)map,capacity,&source)||!hr_private((u32)receipt,sizeof(*receipt),&source)||
  !hr_apart((u32)map,capacity,(u32)p,bytes)||!hr_apart((u32)receipt,sizeof(*receipt),(u32)p,bytes)||
  !hr_apart((u32)map,capacity,(u32)receipt,sizeof(*receipt))){z=HR_ARGUMENT;goto done;}
 zero(&out,sizeof(out));out.missing=HL_MISSING_ALL;result_ok=1;h=heap_find(source.heap,0);
 if(!hr_healthy()||!h||h->state!=HL_LIVE||h->generation!=generation||h->epoch!=epoch||!hr_parent_ready(h)){z=HR_OBSERVER;goto done;}
 if(h->kind!=source.kind||h->vtable!=source.vtable||h->start!=source.start||h->bytes!=source.bytes||h->parent!=source.parent||
  h->direction!=source.direction||!hr_native_header(h)){z=HR_CHANGED;goto done;}
 z=hr_allocator(h,image.count);if(z)goto done;
 if(!hi_ready(h)){z=HR_OBSERVER;goto done;}
 for(i=hi_head(h);i;i=hi_next(i)){
  HlStoredAllocation*r=_gz_hl_allocations+i-1;
  if(r->state==HL_ENDING){z=HR_OBSERVER;goto done;}
  if(r->state==HL_LIVE)retiring++;
 }
 if(retiring>_gz_hl_control.active_allocations||image.count>HL_ALLOCS-(_gz_hl_control.active_allocations-retiring)){z=HR_SPACE;goto done;}
 if(image.count>0xffffffffu-_gz_hl_control.serial||_gz_hl_control.release_serial==0xffffffffu){z=HR_EXHAUSTED;goto done;}
 z=hr_plan(h->heap,image.count);if(z)goto done;
 if(!hr_native_header(h)||hr_allocator(h,image.count)){z=HR_CHANGED;goto done;}
 out.source_heap_generation=source.generation;out.heap_generation=h->generation;out.old_epoch=h->epoch;
 serial_before=_gz_hl_control.serial;out.serial_before=serial_before;new_epoch=_gz_hl_control.release_serial+1;
 /* First metadata mutation. No native writes/callbacks/counter rewind. A
  * dirty failure below retains the outer hold; it cannot resume gameplay. */
 out.mutated=1;
 for(i=hi_head(h);i;i=next){
  HlStoredAllocation*r=_gz_hl_allocations+i-1;next=hi_next(i);if(r->heap==h->heap&&r->state==HL_LIVE){
   if(!hi_detach(r)){z=HR_DIRTY;goto done;}
   r->state=HL_DEAD;r->end_frame=_gz_hl_frame;r->end_site=HR_MAGIC;r->flags|=HL_HEAP_RETIRED;
  }
 }
 _gz_hl_control.active_allocations-=retiring;_gz_hl_control.release_serial=new_epoch;h->epoch=new_epoch;
 for(i=0;i<image.count;i++){
  copy(&a,hr_rows+i,sizeof(a));a.generation=++_gz_hl_control.serial;a.heap_generation=h->generation;a.heap_epoch=new_epoch;
  a.state=HL_LIVE;a.flags|=256u;a.born_frame=_gz_hl_frame;a.birth_site=HR_MAGIC;a.end_frame=a.end_site=a.release_token=0;
  a.missing=HL_MISSING_ALL;a.reserved=0;if(!hl_import(&stored,&a)||!hi_store(_gz_hl_allocations+hr_slots[i],&stored)){z=HR_DIRTY;goto done;}_gz_hl_control.active_allocations++;
  map[i].heap=h->heap;map[i].pointer=a.pointer;map[i].source_generation=hr_rows[i].generation;
  map[i].source_heap_generation=hr_rows[i].heap_generation;map[i].source_epoch=hr_rows[i].heap_epoch;
  map[i].generation=a.generation;map[i].heap_generation=h->generation;map[i].epoch=new_epoch;
 }
 _gz_hm_bind_heap(h);out.mapped=image.count;out.epoch=new_epoch;out.serial_after=_gz_hl_control.serial;
 if(!_gz_hm_scope_live(h)){z=HR_DIRTY;goto done;}
 out.valid=1;z=0;
done:if(result_ok){out.code=z;__sync_synchronize();copy(receipt,&out,sizeof(out));}unlock();return z;
}
