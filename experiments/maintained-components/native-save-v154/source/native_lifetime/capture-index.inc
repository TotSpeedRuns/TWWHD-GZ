/* One immutable allocation interval directory per source capture, under the
 * SAME observer lock throughout build and use. This is not a persistent cache,
 * native allocation permission, world hold or source replay certificate. */
typedef struct {u32 row,generation,pointer,capacity,prefix_end;} SiRow;
typedef struct {u32 slot,heap,generation,epoch,start,bytes,first,count;} SiHeap;
typedef struct {u32 magic,bytes,version,valid,code,heaps,rows,serial,release_serial,frame,active_allocations,queries,candidate_checks,binary_steps,missing,reserved;} SiControl;
SiControl _gz_si_control={.magic=0x53494e31u,.bytes=64,.version=1,.missing=31};
static SiRow si_rows[HL_ALLOCS];
static SiHeap si_heaps[HL_HEAPS];
typedef char SiControlSize[sizeof(SiControl)==64?1:-1];
typedef char SiRowSize[sizeof(SiRow)==20?1:-1];
typedef char SiHeapSize[sizeof(SiHeap)==32?1:-1];
static u32 si_private(u32 p,u32 n){return hr_apart(p,n,(u32)&_gz_si_control,sizeof(_gz_si_control))&&
 hr_apart(p,n,(u32)si_rows,sizeof(si_rows))&&hr_apart(p,n,(u32)si_heaps,sizeof(si_heaps));}
static u32 si_outputs_private(void){u32 i;HlHeap*h;
 if(!hr_private_base((u32)&_gz_si_control,sizeof(_gz_si_control),0)||!hr_private_base((u32)si_rows,sizeof(si_rows),0)||!hr_private_base((u32)si_heaps,sizeof(si_heaps),0))return 0;
 for(i=0;i<HL_HEAPS;i++){h=_gz_hl_heaps+i;
  if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)&&
   (!hr_private_base((u32)&_gz_si_control,sizeof(_gz_si_control),h)||!hr_private_base((u32)si_rows,sizeof(si_rows),h)||!hr_private_base((u32)si_heaps,sizeof(si_heaps),h)))return 0;
 }return 1;
}
static u32 si_less(const SiRow*a,const SiRow*b){return a->pointer<b->pointer||(a->pointer==b->pointer&&a->row<b->row);}
static void si_sift(SiRow*rows,u32 root,u32 n){SiRow value;u32 child;copy(&value,rows+root,sizeof(value));
 while(root<n/2){child=root*2+1;if(child+1<n&&si_less(rows+child,rows+child+1))child++;
  if(!si_less(&value,rows+child))break;copy(rows+root,rows+child,sizeof(value));root=child;
 }copy(rows+root,&value,sizeof(value));
}
static void si_sort(SiRow*rows,u32 n){u32 first=n/2,last=n;SiRow temp;
 while(first){--first;si_sift(rows,first,n);}
 while(last>1){--last;copy(&temp,rows,sizeof(temp));copy(rows,rows+last,sizeof(temp));copy(rows+last,&temp,sizeof(temp));si_sift(rows,0,last);}
}
static u32 si_build_locked(void){SiControl*c=&_gz_si_control;HlHeap*h;HlStoredAllocation*a;HiHeap*index;SiHeap*out;SiRow*r;u32 i,j,node,maximum,z=SC_OBSERVER;
 if(_gz_hl_control.busy!=1||!si_outputs_private())return SC_ARGUMENT;
 zero(c,sizeof(*c));c->magic=0x53494e31u;c->bytes=64;c->version=1;c->missing=31;
 if(!hr_healthy())goto done;c->serial=_gz_hl_control.serial;c->release_serial=_gz_hl_control.release_serial;c->frame=_gz_hl_frame;c->active_allocations=_gz_hl_control.active_allocations;
 for(i=0;i<HL_HEAPS;i++){
  h=_gz_hl_heaps+i;index=_gz_hi_heaps+i;
  if(!h->heap){if(index->head||index->count||index->generation||index->reserved)goto done;continue;}
  if(!hi_ready(h))goto done;
  if(h->state!=HL_LIVE){if(index->count)goto done;continue;}
  if(!hr_native_header(h)||!_gz_hm_scope_live(h))goto done;
  out=si_heaps+c->heaps++;out->slot=i;out->heap=h->heap;out->generation=h->generation;out->epoch=h->epoch;out->start=h->start;out->bytes=h->bytes;out->first=c->rows;out->count=0;
  node=hi_head(h);
  for(j=0;j<index->count;j++){
   a=_gz_hl_allocations+node-1;
   if(a->state==HL_LIVE){
    if(c->rows>=HL_ALLOCS||a->heap_generation!=h->generation||a->heap_epoch!=h->epoch||!a->generation||
     !extent(a->pointer,a->capacity)||!within(h->start,h->bytes,a->pointer,a->capacity))goto done;
    r=si_rows+c->rows++;r->row=node;r->generation=a->generation;r->pointer=a->pointer;r->capacity=a->capacity;r->prefix_end=0;out->count++;
   }node=hi_next(node);
  }
  if(node)goto done;si_sort(si_rows+out->first,out->count);maximum=0;
  for(j=0;j<out->count;j++){r=si_rows+out->first+j;if(r->pointer+r->capacity>maximum)maximum=r->pointer+r->capacity;r->prefix_end=maximum;}
 }
 if(c->heaps!=_gz_hl_control.active_heaps||c->serial!=_gz_hl_control.serial||c->release_serial!=_gz_hl_control.release_serial||c->frame!=_gz_hl_frame||!hr_healthy())goto done;
 z=0;__sync_synchronize();c->valid=1;
done:c->code=z;return z;
}
static u32 si_owner_locked(u32 p,u32 n,HlStoredAllocation**allocation,HlHeap**heap){SiControl*c=&_gz_si_control;SiHeap*s;SiRow*r;HlStoredAllocation*a,*found=0;HlHeap*h,*owner=0;u32 i,lo,hi,mid,k;
 *allocation=0;*heap=0;
 if(!span(p,n)||_gz_hl_control.busy!=1||!c->valid||c->magic!=0x53494e31u||c->bytes!=64||c->version!=1||c->code||c->heaps>HL_HEAPS||c->rows>HL_ALLOCS||c->missing!=31||c->reserved||
  c->serial!=_gz_hl_control.serial||c->release_serial!=_gz_hl_control.release_serial||c->frame!=_gz_hl_frame||c->active_allocations!=_gz_hl_control.active_allocations||!hr_healthy())return 3;
 c->queries++;
 for(i=0;i<c->heaps;i++){
  s=si_heaps+i;if(s->slot>=HL_HEAPS||s->first>c->rows||s->count>c->rows-s->first)return 3;h=_gz_hl_heaps+s->slot;
  if(h->state!=HL_LIVE||h->heap!=s->heap||h->generation!=s->generation||h->epoch!=s->epoch||h->start!=s->start||h->bytes!=s->bytes)return 3;
  if(!within(s->start,s->bytes,p,n))continue;
  lo=0;hi=s->count;
  while(lo<hi){mid=lo+(hi-lo)/2;c->binary_steps++;if(si_rows[s->first+mid].pointer<=p)lo=mid+1;else hi=mid;}
  for(k=lo;k;k--){r=si_rows+s->first+k-1;if(r->prefix_end<p+n)break;
   if(!r->row||r->row>HL_ALLOCS)return 3;a=_gz_hl_allocations+r->row-1;c->candidate_checks++;
   if(a->state!=HL_LIVE||a->generation!=r->generation||a->pointer!=r->pointer||a->capacity!=r->capacity||a->heap!=h->heap||a->heap_generation!=h->generation||a->heap_epoch!=h->epoch)return 3;
   if(!within(a->pointer,a->capacity,p,n))continue;
   if(found){if(h==owner)return 2;if(descends(h,owner,found)){found=a;owner=h;}else if(!descends(owner,h,a))return 2;}
   else{found=a;owner=h;}
  }
 }if(found){*allocation=found;*heap=owner;return 0;}return 1;
}
static void si_end(void){__sync_synchronize();_gz_si_control.valid=0;}
