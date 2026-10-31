/* Same held-interval pairing of allocator/payload images and allocation
 * generations. No executable callbacks/OS identities are read from images. */
#define GZ_JOINT_RESTORE_DRIVER 1
#ifdef GZ_JOINT_OBSERVATION_FIXTURE
#include "heap_graph.h"
extern void _gz_test_aj_disposed(const AgHeap*,AgU32);
#endif
#include "rebuild.c"
#include "joint.h"
#include "rebind.h"
#include "lifetime.h"
#include "joint_restore.h"
typedef struct {U32 magic,version,bytes,valid,checksum,count,afs,afs_bytes;
 U32 index,data,root,session,generation,serial,release_serial,reserved;} AjHeader;
typedef struct {U32 heap,offset,bytes,reserved;} AjIndex;
typedef char AjHeaderSize[sizeof(AjHeader)==64?1:-1];
typedef char AjIndexSize[sizeof(AjIndex)==16?1:-1];
typedef struct {U32 busy,active,thread,phase,next,mapped,birth_floor,root_generation,hr_error;
 U32 recovery,input,result_address,recovery_stage,recovery_stamp[4];Journal recovery_journal;
 const AjRestoreRequest*request;const void*source_af;const void*ar_undo;U32 ar_undo_bytes,stamp[4];
 AjRestoreRequest prepared;
 AjHeader source,undo;
} JrContext;
static JrContext jr;
extern void*OSGetCurrentThread(void);
static U32 aj_busy;
volatile U32 _gz_aj_stage;
static U32 aj_graph[(64+AG_MAX_HEAPS*48+AG_MAX_OBJECTS*16)/4];
#define AJ_MAGIC 0x414a4231u
static U32 aj_meta(U32 p,U32 n){return metadata(p,n)&&_gz_hr_buffer_private((void*)p,n)&&
 apart(p,n,(U32)&aj_busy,4)&&apart(p,n,(U32)&_gz_aj_stage,4)&&apart(p,n,(U32)aj_graph,sizeof(aj_graph))&&
 apart(p,n,(U32)&jr,sizeof(jr));}
static U32 aj_private(U32 p,U32 n,const AfDomain*d){return aj_meta(p,n)&&apart(p,n,(U32)d,sizeof(*d));}
static U32 jr_foreign_thread(void){return jr.busy&&jr.thread!=(U32)OSGetCurrentThread();}
static U32 aj_graph_private(U32 p,U32 n,const AgHeap*hs,U32 count){U32 i;
 for(i=0;i<count;i++)if(!apart(p,n,hs[i].start,hs[i].span)||!apart(p,n,hs[i].heap,heap_header(hs[i].kind)))return 0;
 return 1;
}
static U32 aj_hash(const void*p,U32 n){const U8*b=p;U32 i,h=2166136261u;
 for(i=0;i<n;i++)h=(h^((i>=16&&i<20)?0:(i>=12&&i<16)?(i==15):b[i]))*16777619u;
 return h;
}
/* A validated source directory must describe this SAME allocator archive,
 * including exact Exp padding/capacity and live Unit slots. No old native
 * heap is dereferenced: validation remains usable after heap destruction. */
static U32 aj_pair(const U8*af,const ForestHeader*fh,U32 i,const U8*dir,U32 bytes,U32 serial,U32 release_serial){
 const AgHeap*g=graph_of(af)+i;const ForestIndex*ix=index_of(af,fh);const AsHeader*a=(const AsHeader*)(af+ix[i].offset);
 const AsRow*as=(const AsRow*)((const U8*)a+AS_HEADER);const HlHeap*h=(const HlHeap*)(dir+64);const HlAllocation*rs=(const HlAllocation*)(dir+128);
 U32 j,k,lo,hi,mid,offset,count,z;
 if(bytes<128)return AS_CORRUPT;
 z=_gz_hr_validate(dir,bytes);if(z)return AS_CORRUPT;
 count=R32(dir,28);
 if(h->heap!=g->heap||h->kind!=g->kind||h->start!=g->start||h->bytes!=g->span||h->parent!=g->parent||h->vtable!=g->vtable||
  h->direction!=g->direction||R32(dir,40)!=serial||R32(dir,44)!=release_serial)return AS_CORRUPT;
 if((g->kind==1||g->kind==3)&&count!=a->used)return AS_CORRUPT;
 for(j=0;j<count;j++){
  const HlAllocation*r=rs+j;
  if(g->kind==1){
   offset=r->block-g->start;lo=0;hi=a->blocks;
   while(lo<hi){mid=lo+(hi-lo)/2;if(as[mid].offset<offset)lo=mid+1;else hi=mid;}
   if(lo==a->blocks||as[lo].offset!=offset||as[lo].kind!=1||as[lo].extent!=16+r->padding+r->capacity||
    R32((const U8*)a+as[lo].data,8)!=r->capacity||R32((const U8*)a+as[lo].data,12)!=r->padding)return AS_CORRUPT;
  }else if(g->kind==3){
   if(r->pointer<a->mutable_words[1]||!a->mutable_words[0]||(r->pointer-a->mutable_words[1])%a->mutable_words[0])return AS_CORRUPT;
   k=(r->pointer-a->mutable_words[1])/a->mutable_words[0];
   if(k>=a->blocks||as[k].kind!=3||r->capacity!=as[k].extent||r->pointer!=g->start+as[k].offset)return AS_CORRUPT;
  }else if(g->kind==2){
   for(k=0;k<a->blocks;k++)if(has(g->start+as[k].offset,as[k].extent,r->pointer,r->capacity))break;
   if(k==a->blocks)return AS_CORRUPT;
  }else return AS_CORRUPT;
 }return 0;
}
static U32 aj_validate_locked(const AfDomain*d,const void*p,U32 bytes,AjHeader*out){
 AjHeader h;ForestHeader af;const U8*b=p;const AjIndex*ix;const AgHeap*hs;U32 i,at,z,total=0;
 if(!metadata((U32)d,sizeof(*d))||!aj_private((U32)p,bytes,d)||bytes<64)return AS_ARGUMENT;
 copy(&h,p,64);
 if(h.magic!=AJ_MAGIC||h.version!=1||h.bytes!=bytes||h.valid!=1||h.reserved||h.afs!=64||h.afs_bytes>bytes-64||
  h.root!=d->graph.root||h.session!=d->graph.session||h.generation!=d->generation||h.count>AG_MAX_HEAPS||
  h.index!=64+h.afs_bytes||h.count>(bytes-h.index)/16||h.data!=h.index+h.count*16||h.checksum!=aj_hash(p,bytes))return AS_CORRUPT;
 z=forest_validate_locked(d,b+64,h.afs_bytes,&af);if(z)return z;hs=graph_of(b+64);
 if(af.count!=h.count||!aj_graph_private((U32)p,bytes,hs,h.count))return AS_CORRUPT;
 ix=(const AjIndex*)(b+h.index);at=h.data;
 for(i=0;i<h.count;i++){
  if(ix[i].reserved||ix[i].heap!=hs[i].heap||ix[i].offset!=at||ix[i].bytes>bytes-at)return AS_CORRUPT;
  z=aj_pair(b+64,&af,i,b+at,ix[i].bytes,h.serial,h.release_serial);if(z)return z;
  if(R32(b+at,28)>HL_ALLOCS-total)return AS_CORRUPT;total+=R32(b+at,28);at+=ix[i].bytes;
 }if(at!=bytes)return AS_CORRUPT;if(out)copy(out,&h,64);return 0;
}
AgU32 _gz_aj_validate(const AfDomain*d,const void*p,U32 n){U32 z;
 if(jr.active||jr_foreign_thread())return AS_BUSY;
 if(!__sync_bool_compare_and_swap(&aj_busy,0,1))return AS_BUSY;
 if(!__sync_bool_compare_and_swap(&forest_busy,0,1)){aj_busy=0;return AS_BUSY;}
 z=aj_validate_locked(d,p,n,0);release();__sync_synchronize();aj_busy=0;return z;
}
AgU32 _gz_aj_capture(const AfDomain*d,void*p,U32 cap,U32*written){
 if(!_gz_image_write_allowed((U32)p,cap)||!_gz_image_write_allowed((U32)written,4))return AS_ARGUMENT;
 U32 z,n=0,gn=0,i,at,nb=0,stamp[4],after[4],result_ok=0;AjHeader h;ForestHeader af;AjIndex*ix;const AgHeap*hs;
 if(jr.active||jr_foreign_thread()||!__sync_bool_compare_and_swap(&aj_busy,0,1))return AS_BUSY;
 if(!__sync_bool_compare_and_swap(&forest_busy,0,1)){aj_busy=0;return AS_BUSY;}
 _gz_aj_stage=1;
 if(!metadata((U32)d,sizeof(*d))||cap<64||!aj_private((U32)p,cap,d)||!aj_private((U32)written,4,d)||
  !apart((U32)p,cap,(U32)written,4)){z=AS_ARGUMENT;goto done;}
 _gz_aj_stage=2;z=_gz_hr_stamp(stamp);if(z){z=AS_CHANGED;goto done;}
 z=_gz_ag_capture(&d->graph,aj_graph,sizeof(aj_graph),&gn);if(z)goto done;
 hs=(const AgHeap*)((const U8*)aj_graph+64);n=R32((const U8*)aj_graph,24);
 /* Classify the ENTIRE output, including its leading header, before writing
  * even the invalid publication marker. Never corrupt a boundary-straddling
  * native heap by checking only the embedded allocator image's address. */
 if(!aj_graph_private((U32)p,cap,hs,n)||!aj_graph_private((U32)written,4,hs,n)||!aj_graph_private((U32)d,sizeof(*d),hs,n)){z=AS_ARGUMENT;goto done;}
 *written=0;((AjHeader*)p)->valid=0;result_ok=1;
 _gz_aj_stage=3;z=forest_capture_locked(d,(U8*)p+64,cap-64,&nb);if(z)goto done;
 _gz_aj_stage=4;
 copy(&af,(U8*)p+64,64);if(af.count!=n||af.graph_bytes!=gn||!equal((U8*)p+128,aj_graph,gn)){z=AS_CHANGED;goto done;}
 if(n>(cap-64-nb)/16){z=AS_SPACE;goto done;}at=64+nb+n*16;ix=(AjIndex*)((U8*)p+64+nb);
 for(i=0;i<n;i++){
  _gz_aj_stage=0x100+i;
  ix[i].heap=hs[i].heap;ix[i].offset=at;ix[i].reserved=0;
  z=_gz_hr_capture(hs[i].heap,(U8*)p+at,cap-at,&ix[i].bytes);if(z)goto done;
  z=aj_pair((U8*)p+64,&af,i,(U8*)p+at,ix[i].bytes,stamp[0],stamp[1]);if(z)goto done;at+=ix[i].bytes;
 }
 /* The wider owner still provides writer exclusion. These checks catch
  * mismatched metadata generations and payload drift across the components. */
 z=establish(d,(U8*)p+128,af.graph_bytes);if(z)goto done;
 for(i=0;i<n;i++){
  _gz_aj_stage=0x200+i;
  AsDomain part;AsHeader ph;const ForestIndex*fi=index_of((U8*)p+64,&af);
  part_domain(d,i,&part);copy(&ph,(U8*)p+64+fi[i].offset,sizeof(ph));
  if(!matches(&part,(U8*)p+64+fi[i].offset,&ph)){z=AS_CHANGED;goto done;}
 }
 _gz_aj_stage=0x300;z=_gz_hr_stamp(after);if(z||!equal(stamp,after,sizeof(stamp))){z=AS_CHANGED;goto done;}
 zero(&h,sizeof(h));h.magic=AJ_MAGIC;h.version=1;h.bytes=at;h.count=n;h.afs=64;h.afs_bytes=nb;h.index=64+nb;h.data=64+nb+n*16;
 h.root=d->graph.root;h.session=d->graph.session;h.generation=d->generation;h.serial=stamp[0];h.release_serial=stamp[1];
 copy(p,&h,64);((AjHeader*)p)->checksum=aj_hash(p,at);__sync_synchronize();((AjHeader*)p)->valid=1;*written=at;_gz_aj_stage=0x400;z=0;
done:if(z&&result_ok)*written=0;release();__sync_synchronize();aj_busy=0;return z;
}
#include "joint_restore.inc"
