#include "catalog.h"
#include "../native-scene-inventory-20261017/process.h"
#define R8(p,o) (*(volatile const u8*)((p)+(o)))
BcCatalog _gz_bc_catalog={.magic=BC_MAGIC,.bytes=sizeof(BcCatalog),.version=BC_VERSION,.row_bytes=sizeof(BcRow),.capacity=BC_CAP};
BcRow _gz_bc_result;
BcView _gz_bc_view_state={.magic=0x42435631u,.bytes=sizeof(BcView)};
static u32 _gz_bc_pending[BC_DEPTH];
static u32 _gz_bc_rows_sealed[BC_CAP];
static u32 _gz_bc_header_seal=0x35cd265fu;
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x01000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static u32 apart(u32 a,u32 n,u32 b,u32 m){return a+n<=b||b+m<=a;}
static u32 private_apart(u32 a,u32 n){return span(a,n)&&apart(a,n,(u32)&_gz_bc_view_state,sizeof(BcView))&&apart(a,n,(u32)&_gz_bc_catalog,sizeof(BcCatalog))&&apart(a,n,(u32)_gz_bc_pending,sizeof(_gz_bc_pending))&&apart(a,n,(u32)&_gz_bc_result,sizeof(BcRow))&&apart(a,n,(u32)&_gz_bc_header_seal,4)&&apart(a,n,(u32)_gz_bc_rows_sealed,sizeof(_gz_bc_rows_sealed));}
static u32 header_seal(void){u32 i,h=2166136261u;
 for(i=0;i<64;i++)h=(h^R8((u32)&_gz_bc_catalog,i))*16777619u;
 for(i=0;i<sizeof(_gz_bc_pending);i++)h=(h^R8((u32)_gz_bc_pending,i))*16777619u;
 return h;
}
static void finish(void){_gz_bc_header_seal=header_seal();}
static u32 header(void){const BcCatalog*c=&_gz_bc_catalog;
 return c->magic==BC_MAGIC&&c->bytes==sizeof(BcCatalog)&&c->version==BC_VERSION&&c->row_bytes==sizeof(BcRow)&&c->capacity==BC_CAP&&
 c->count<=BC_CAP&&c->valid_count<=c->count&&c->failed_requests<=c->count&&c->completed<=c->attempts&&
 c->depth==c->attempts-c->completed&&c->count<=c->attempts&&!c->reserved&&_gz_bc_header_seal==header_seal();
}
static u32 extended_origin(u32 origin){return origin==0x025d5a44u||origin==0x025d5c70u;}
static u32 null_origin(u32 origin,u32 profile){
 return ((origin==0x025ae6acu||origin==0x025c32ecu)&&profile==437)||
        ((origin==0x025ae6c4u||origin==0x025c3304u)&&profile==438);
}
static u32 seal(const BcRow*r){u32 i,h=2166136261u,v;for(i=0;i<sizeof(*r);i++){
 v=R8((u32)r,i);if(i>=60&&i<64)v=0;if(i>=12&&i<16)v=(i==15);h=(h^v)*16777619u;}return h;}
static u32 completion_seal(const BcRow*r){u32 i,h=2166136261u;
 for(i=0;i<sizeof(*r);i++)h=(h^R8((u32)r,i))*16777619u;
 return h;
}
u32 _gz_bc_row_valid(const BcRow*r){u32 i;
 if(!span((u32)r,sizeof(*r))||r->magic!=BC_ROW_MAGIC||r->bytes!=sizeof(*r)||r->version!=BC_VERSION||r->valid!=1||
 !r->sequence||r->sequence>BC_CAP||(r->origin!=0x025d5854u&&r->origin!=0x025dfab8u&&r->origin!=0x025c1180u&&!extended_origin(r->origin)&&!null_origin(r->origin,r->profile))||r->profile>483||!r->profile_entry||r->pid>=0xfffffffeu||
 r->frame_after-r->frame_before>=0x80000000u)return 0;
 if(null_origin(r->origin,r->profile)){
  if(r->append_audit||r->callback||r->callback_arg||r->reserved[0]||r->reserved[1]||r->reserved[2]!=0x4e554c31u)return 0;
  for(i=0;i<36;i++)if(r->append[i])return 0;
  return r->checksum==seal(r);
 }
 if(!span(r->append_audit,36)||r->append_audit<0x10000000u)return 0;
 if(r->origin!=0x025dfab8u){for(i=0;i<3;i++)if(r->reserved[i])return 0;}
 else if(!r->reserved[0]||r->reserved[2]!=0x46505231u||r->reserved[1]<0x10000000u||!span(r->reserved[1],0xb8))return 0;
 return r->checksum==seal(r);
}
static void bc_before(u32 layer,u32 profile,u32 callback,u32 callback_arg,u32 append,u32 origin,u32 null_append){
 BcCatalog*c=&_gz_bc_catalog;BcRow*r;u32 d,i,index=0xffffffffu,entry;
 if(!header()||c->blocked)return;
 if(c->attempts==0xffffffffu||c->depth==0xffffffffu){c->blocked=1;finish();return;}
 d=c->depth;c->depth=d+1;c->attempts++;
 if(d>=BC_DEPTH){c->depth_overflow++;finish();return;}
 _gz_bc_pending[d]=index;
 if(profile>483||(null_append?(!null_origin(origin,profile)||append||callback||callback_arg):(!private_apart(append,36)||append<0x10000000u))){c->invalid_inputs++;finish();return;}
 entry=_gz_bc_native_profiles[profile];
 if(!entry){c->invalid_inputs++;finish();return;}
 if(c->count==BC_CAP){c->overflow++;finish();return;}
 index=c->count++;r=&c->rows[index];_gz_bc_rows_sealed[index]=0;
 for(i=0;i<sizeof(*r)/4;i++)((volatile u32*)r)[i]=0;
 r->magic=BC_ROW_MAGIC;r->bytes=sizeof(*r);r->version=BC_VERSION;r->sequence=index+1;r->origin=origin;
 r->pid=0xffffffffu;r->profile=profile;r->profile_entry=entry;r->layer=layer;r->callback=callback;r->callback_arg=callback_arg;
 r->append_audit=append;r->frame_before=_gz_bc_native_frame;
 if(null_append)r->reserved[2]=0x4e554c31u;
 else for(i=0;i<36;i++)r->append[i]=R8(append,i);
 _gz_bc_rows_sealed[index]=completion_seal(r);_gz_bc_pending[d]=index;finish();
}
static void bc_after(u32 native_result,u32 process,u32 generation){
 BcCatalog*c=&_gz_bc_catalog;BcRow*r;u32 d,index;
 if(!header()||c->blocked)return;
 if(!c->depth){c->blocked=1;finish();return;}
 d=--c->depth;c->completed++;
 if(d>=BC_DEPTH){finish();return;}
 index=_gz_bc_pending[d];_gz_bc_pending[d]=0xffffffffu;
 if(index==0xffffffffu){finish();return;}
 if(index>=c->count){c->blocked=1;finish();return;}r=&c->rows[index];
 if(r->valid||r->sequence!=index+1||_gz_bc_rows_sealed[index]!=completion_seal(r)){c->blocked=1;finish();return;}
 r->pid=native_result;r->frame_after=_gz_bc_native_frame;
 if(native_result==0xffffffffu){c->failed_requests++;_gz_bc_rows_sealed[index]=completion_seal(r);finish();return;}
 if(native_result==0xfffffffeu||r->frame_after-r->frame_before>=0x80000000u){c->invalid_inputs++;_gz_bc_rows_sealed[index]=completion_seal(r);finish();return;}
 if(r->origin==0x025dfab8u){
  if(!process||!generation){c->invalid_inputs++;_gz_bc_rows_sealed[index]=completion_seal(r);finish();return;}
  r->reserved[0]=generation;r->reserved[1]=process;r->reserved[2]=0x46505231u;
 }
 r->checksum=seal(r);__asm__ volatile("sync":::"memory");r->valid=1;c->valid_count++;_gz_bc_rows_sealed[index]=completion_seal(r);finish();
}
u32 _gz_bc_lookup(u32 pid,u32 profile){
 const BcCatalog*c=&_gz_bc_catalog;const BcRow*found=0;BcRow*output=&_gz_bc_result;u32 i,j;
 output->valid=0;
 if(!header()||c->blocked||c->depth||pid>=0xfffffffeu||profile>483)return 1;
 for(i=0;i<c->count;i++){
  const BcRow*r=&c->rows[i];
  if(r->sequence!=i+1||_gz_bc_rows_sealed[i]!=completion_seal(r))return 1;
  if(!r->valid||r->pid!=pid||r->profile!=profile)continue;
  if(!_gz_bc_row_valid(r))return 1;if(found)return 2;found=r;
 }
 if(!found)return 1;
 for(j=0;j<sizeof(*found);j++)if(j<12||j>=16)((volatile u8*)output)[j]=R8((u32)found,j);
 if(output->checksum!=seal(output))return 1;
 __asm__ volatile("sync":::"memory");output->valid=1;return 0;
}

/* Fast factory inputs are APP36 only for the independently pinned actor
 * caller. Other users of fpcM_FastCreate are passed through without an APP read.
 * Original returned process memory is NEVER dereferenced here. */
volatile u32 _gz_bc_fast_diagnostic[8]={0x42434632u,32};
void _gz_bc_after(u32 native_result){bc_after(native_result,0,0);}
void _gz_bc_before(u32 layer,u32 profile,u32 callback,u32 data,u32 append){bc_before(layer,profile,callback,data,append,0x025d5854u,0);}
void _gz_bc_before_fast(u32 layer,u32 profile,u32 callback,u32 data,u32 append,u32 actor_caller){
 if(actor_caller!=0x025d5944u){_gz_bc_fast_diagnostic[3]++;return;}
 _gz_bc_fast_diagnostic[2]++;bc_before(layer,profile,callback,data,append,0x025dfab8u,0);
}
void _gz_bc_after_fast(u32 process,u32 actor_caller){u32 index;const PoEntry*e;
 if(actor_caller!=0x025d5944u)return;
 _gz_bc_fast_diagnostic[4]++;
 if(!header()||_gz_bc_catalog.blocked)return;
 if(!_gz_bc_catalog.depth||_gz_bc_catalog.depth>BC_DEPTH){_gz_bc_after(0xfffffffeu);return;}
 index=_gz_bc_pending[_gz_bc_catalog.depth-1];
 if(index==0xffffffffu){_gz_bc_after(0xfffffffeu);return;}
 if(index>=_gz_bc_catalog.count||_gz_bc_catalog.rows[index].origin!=0x025dfab8u){_gz_bc_after(0xfffffffeu);return;}
 if(!process){_gz_bc_after(0xffffffffu);return;}
 if(_gz_po_lookup(process)||!_gz_po_observation.valid||_gz_po_observation.flags){_gz_bc_fast_diagnostic[6]++;_gz_bc_after(0xfffffffeu);return;}
 e=&_gz_po_observation.entry;
 if(e->object!=process||e->state!=PO_LIVE||e->flags||!e->generation||e->pid>=0xfffffffeu||
  e->profile!=_gz_bc_catalog.rows[index].profile||e->profile_entry!=_gz_bc_catalog.rows[index].profile_entry||
  e->born_frame-_gz_bc_catalog.rows[index].frame_before>=0x80000000u){_gz_bc_fast_diagnostic[6]++;_gz_bc_after(0xfffffffeu);return;}
 _gz_bc_fast_diagnostic[5]++;bc_after(e->pid,process,e->generation);
}

/* Original stage actor APP36 has already received name-table argument/gbaName. */
void _gz_bc_before_stage(u32 layer,u32 profile,u32 callback,u32 data,u32 append){bc_before(layer,profile,callback,data,append,0x025c1180u,0);}

/* These four original callers request profiles437/438 with no append. */
void _gz_bc_before_null(u32 layer,u32 profile,u32 callback,u32 data,u32 append,u32 origin){bc_before(layer,profile,callback,data,append,origin,1);}

u32 _gz_bc_view_open(void){BcView*v=&_gz_bc_view_state;u32 i,valid=0;
 v->valid=0;
 if(!header()||_gz_bc_catalog.blocked||_gz_bc_catalog.depth)return 1;
 for(i=0;i<_gz_bc_catalog.count;i++){
  const BcRow*r=&_gz_bc_catalog.rows[i];
  if(r->sequence!=i+1||_gz_bc_rows_sealed[i]!=completion_seal(r))return 1;
  if(r->valid){if(!_gz_bc_row_valid(r))return 1;valid++;}
 }
 if(valid!=_gz_bc_catalog.valid_count||!header()||_gz_bc_catalog.depth)return 1;
 v->magic=0x42435631u;v->bytes=sizeof(*v);v->attempts=_gz_bc_catalog.attempts;
 v->completed=_gz_bc_catalog.completed;v->count=_gz_bc_catalog.count;v->header_seal=_gz_bc_header_seal;v->frame=_gz_bc_native_frame;
 __asm__ volatile("sync":::"memory");v->valid=1;return 0;
}
u32 _gz_bc_view_valid(void){const BcView*v=&_gz_bc_view_state;
 return v->magic==0x42435631u&&v->bytes==sizeof(*v)&&v->valid==1&&header()&&!_gz_bc_catalog.blocked&&!_gz_bc_catalog.depth&&
 v->attempts==_gz_bc_catalog.attempts&&v->completed==_gz_bc_catalog.completed&&v->count==_gz_bc_catalog.count&&
 v->header_seal==_gz_bc_header_seal&&v->frame==_gz_bc_native_frame;
}
const BcRow*_gz_bc_view_row(u32 index){const BcRow*r;
 if(!_gz_bc_view_valid()||index>=_gz_bc_view_state.count)return 0;
 r=&_gz_bc_catalog.rows[index];
 return r->sequence==index+1&&_gz_bc_rows_sealed[index]==completion_seal(r)?r:0;
}
void _gz_bc_view_close(void){_gz_bc_view_state.valid=0;}

/* Both pinned callsites obtain APP36 from native025D5638/025D55C0.
 * Unknown sites do not open OR close this observer's pending journal. */
void _gz_bc_before_extended(u32 layer,u32 profile,u32 callback,u32 data,u32 append,u32 origin){
 if(extended_origin(origin))bc_before(layer,profile,callback,data,append,origin,0);
}
void _gz_bc_after_extended(u32 result,u32 origin){if(extended_origin(origin))bc_after(result,0,0);}
