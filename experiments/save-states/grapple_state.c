/* Typed native HIMO2 simulation capture. Model/collision/FX/audio ownership and
 * unknown rope-entry bytes are named pending replay dependencies, not copied
 * process pointers. This component does not enable a successful restore. */
#include "grapple_state.h"
#include "archive_layout.h"
#define R32(p,o) (*(volatile const u32*)((p)+(o)))
#define R8(p,o) (*(volatile const u8*)((p)+(o)))
typedef struct {u32 offset,bytes,floats;} HrpField;
static const HrpField hrp_fields[]={
 {0x300,12,1},{0x30c,8,0},{0x314,12,1},{0x320,8,0},{0x328,6,0},{0x330,12,1},{0x33c,12,1},{0x370,8,1},
 {0x3b8,10,0},{0x3d0,12,1},{0x3e8,4,0},{0x3f4,9,0},{0x400,4,1},{0x408,24,1},{0x424,2,0},
 {0x21d4,4,0},{0x21d8,32,1},{0x21f8,6,0},{0x2388,4,1},{0x2680,4,0},{0x2684,8,1},{0x281c,1,0},
 {0x2ae4,8,1},{0x2aec,8,0},{0x2af4,4,1},{0x2af8,4,0},{0x2afc,12,1},{0x2b08,2,0},{0x2b0c,36,1},
 {0x2b30,16,1},{0x2b40,4,0},{0x2b44,8,1},{0x2b4c,1,0},{0x2b50,20,1}};
enum {HRP_PENDING=95,HRP_UNRESOLVED=32};
HrpBank _gz_hrp_source,_gz_hrp_stage;
volatile u32 _gz_hrp_ready;
/* Diagnostic evidence only; never serialized as replay data or restored. */
u8 _gz_hrp_native_audit[0x2d68];
u32 _gz_hrp_native_audit_meta[4];
extern VrWorld _gz_vws_stage,_gz_vws_source;
extern BbBank _gz_bcb_staging,_gz_bcb_work;
extern volatile u32 _gz_roh_live_frame,_gz_roh_live_epoch;
extern u32 _gz_vws_validate(const VrWorld*,u32),_gz_vws_validate_ready(void),_gz_vws_validate_current(void);
extern u32 _gz_roh_user_guard(void),_gz_roh_work_replaced(void);
static void copy(void*d,const void*s,u32 n){u32 i;for(i=0;i<n;i++)((u8*)d)[i]=((volatile const u8*)s)[i];}
static u32 same(const void*a,const void*b,u32 n){u32 i;for(i=0;i<n;i++)if(((volatile const u8*)a)[i]!=((volatile const u8*)b)[i])return 0;return 1;}
static u32 span(const void*p,u32 n){u32 a=(u32)p;return !(a&3)&&a>=0x10000000u&&a<0xbffe0000u&&n&&n<=0xbffe0000u-a;}
static u32 hash(const void*p,u32 n,u32 off,u32 valid){u32 i,h=2166136261u,b;for(i=0;i<n;i++){b=((volatile const u8*)p)[i];if(i>=off&&i<off+4)b=0;if(valid&&i>=12&&i<16)b=i==15;h=(h^b)*16777619u;}return h;}
static u32 finite(u32 v){return (v&0x7fffffffu)<0x7f800000u;}
static u32 layout(void){return hash(hrp_fields,sizeof(hrp_fields),0xffffffffu,0);}
u32 _gz_hrp_field_count(void){return sizeof(hrp_fields)/sizeof(hrp_fields[0]);}
u32 _gz_hrp_field_word(u32 i){return i<_gz_hrp_field_count()?(hrp_fields[i].offset<<16)|hrp_fields[i].bytes:0;}
u32 _gz_hrp_field_float(u32 i){return i<_gz_hrp_field_count()?hrp_fields[i].floats:0;}
static u32 value_valid(const HrpRow*r){u32 i,j,k=0,b;
 for(i=0;i<_gz_hrp_field_count();i++){
  const HrpField*f=&hrp_fields[i];if(k+f->bytes>512)return 0;
  if(f->floats)for(j=0;j<f->bytes;j+=4)if(!finite(R32((u32)r->scalars,k+j)))return 0;
  b=(f->bytes+3)&~3u;for(j=f->bytes;j<b;j++)if(r->scalars[k+j])return 0;k+=b;
 }
 if(r->scalar_bytes!=k)return 0;
 for(i=k;i<512;i++)if(r->scalars[i])return 0;
 for(i=0;i<200;i++)for(j=0;j<16;j+=4)if(!finite(R32((u32)r->points,i*20+j)))return 0;
 return 1;
}
static u32 scalar_value(const HrpRow*r,u32 offset,u32 bytes){u32 i,j,k=0,v=0;
 for(i=0;i<_gz_hrp_field_count();i++){const HrpField*f=&hrp_fields[i];
  if(offset>=f->offset&&bytes<=f->bytes&&offset-f->offset<=f->bytes-bytes){
   for(j=0;j<bytes;j++)v=(v<<8)|r->scalars[k+offset-f->offset+j];return v;
  }k+=(f->bytes+3)&~3u;
 }return 0xffffffffu;
}
static void ref_capture(HrpRef*r,u32 a,const VrWorld*w,const BbBank*b,u32 active){u32 i;
 r->audit=a;r->pid=r->profile=r->world_index=0xffffffffu;r->generation=0;r->kind=0;
 if(!a)return;
 if(!active){r->kind=3;return;}
 for(i=0;i<w->count;i++)if(w->actors[i].actor==a){const PoEntry*e=&b->rows[i].process;
  r->pid=e->pid;r->profile=e->process_name;r->generation=e->generation;r->world_index=i;r->kind=1;return;
 }
 r->kind=2;
}
static u32 ref_valid(const HrpRef*r,const VrWorld*w,const BbBank*b){const PoEntry*e;
 if(!r->kind)return !r->audit&&r->pid==0xffffffffu&&r->profile==0xffffffffu&&!r->generation&&r->world_index==0xffffffffu;
 if(r->kind==2||r->kind==3)return r->audit&&r->pid==0xffffffffu&&r->profile==0xffffffffu&&!r->generation&&r->world_index==0xffffffffu;
 if(r->kind!=1||r->world_index>=w->count)return 0;e=&b->rows[r->world_index].process;
 return r->audit==e->object&&r->pid==e->pid&&r->profile==e->process_name&&r->generation==e->generation;
}
u32 _gz_hrp_validate(const HrpBank*p,u32 bytes,const VrWorld*w,const BbBank*b){u32 i,j,count=0,unresolved;
 if(bytes<HRP_HEADER||bytes>HRP_BYTES||(bytes&3)||!span(p,bytes)||!span(w,sizeof(*w))||!span(b,BB_HEADER)||
  _gz_vws_validate(w,sizeof(*w))||_gz_bcb_validate(b,b->bytes,w))return 28;
 if(p->magic!=HRP_MAGIC||p->bytes!=bytes||p->version!=1||p->valid!=1||p->count>HRP_CAP||bytes!=HRP_HEADER+p->count*HRP_ROW||
  p->frame!=w->frame||p->epoch!=w->epoch||p->session!=w->session||p->stage0!=w->stage0||p->stage1!=w->stage1||p->room!=w->room||p->layer!=w->layer||
  p->world_hash!=w->checksum||p->world_sequence!=w->sequence||p->layout!=layout()||p->checksum!=hash(p,bytes,56,1))return 28;
 for(i=0;i<16;i++)if(p->reserved[i])return 28;
 for(i=0;i<w->count;i++)if(w->actors[i].profile==446)count++;
 if(count!=p->count)return 28;
 for(i=0;i<p->count;i++){const HrpRow*r=&p->rows[i];
  if(r->world_index>=w->count||w->actors[r->world_index].profile!=446||r->layout!=layout()||r->ref_count!=103||r->point_count>99||r->active_targets>100||
   !same(&r->process,&b->rows[r->world_index].process,sizeof(PoEntry))||!same(&r->request,&b->rows[r->world_index].request,sizeof(BcRow))||
   r->checksum!=hash(r,HRP_ROW,196,0)||!value_valid(r)||r->mode!=scalar_value(r,0x3f8,4)||
   r->point_count!=scalar_value(r,0x3e8,4)||r->active_targets!=scalar_value(r,0x281c,1))return 28;
  for(j=0;j<i;j++)if(p->rows[j].world_index==r->world_index)return 28;
  unresolved=0;for(j=0;j<103;j++){
   if(!ref_valid(&r->refs[j],w,b))return 28;
   if(r->refs[j].kind==3&&(!j||j>100||j<=r->active_targets))return 28;
   if(j&&j<=100&&j>r->active_targets&&(r->refs[j].kind==1||r->refs[j].kind==2))return 28;
   if(r->refs[j].kind==2||r->refs[j].kind==3)unresolved=HRP_UNRESOLVED;
  }
  if(r->flags!=(HRP_PENDING|unresolved))return 28;
  for(j=0;j<3;j++)if(r->reserved[j])return 28;
  for(j=0;j<sizeof(r->tail);j++)if(r->tail[j])return 28;
 }
 return 0;
}
static u32 process_same(const PoEntry*e){return !_gz_po_lookup(e->object)&&_gz_po_observation.valid&&!_gz_po_observation.flags&&same(e,&_gz_po_observation.entry,sizeof(*e));}
static u32 capture_row(HrpRow*r,u32 index,const VrWorld*w,const BbBank*b){u32 a=w->actors[index].actor,i,j,k=0;
 const PoEntry*e=&b->rows[index].process;
 if(e->process_name!=446||e->allocation_bytes<0x2d68||!process_same(e))return 28;
 r->world_index=index;r->flags=HRP_PENDING;r->layout=layout();r->ref_count=103;r->mode=R32(a,0x3f8);r->point_count=R32(a,0x3e8);r->active_targets=R8(a,0x281c);
 if(r->point_count>99||r->active_targets>100)return 28;
 copy(&r->process,e,sizeof(*e));copy(&r->request,&b->rows[index].request,sizeof(BcRow));
 for(i=0;i<_gz_hrp_field_count();i++){copy(r->scalars+k,(const void*)(a+hrp_fields[i].offset),hrp_fields[i].bytes);k+=(hrp_fields[i].bytes+3)&~3u;}
 r->scalar_bytes=k;
 for(i=0;i<200;i++){u32 p=a+(i<100?0x42c+i*36:0x123c+(i-100)*36);
  copy(r->points+i*20,(const void*)(p+12),16);copy(r->points+i*20+16,(const void*)(p+28),4);
 }
 ref_capture(&r->refs[0],R32(a,0x267c),w,b,1);
 for(i=0;i<100;i++)ref_capture(&r->refs[i+1],R32(a,0x268c+i*4),w,b,i<r->active_targets);
 ref_capture(&r->refs[101],R32(0x10464678,0),w,b,1);ref_capture(&r->refs[102],R32(0x1046467c,0),w,b,1);
 for(i=0;i<103;i++)if(r->refs[i].kind==2||r->refs[i].kind==3)r->flags|=HRP_UNRESOLVED;
 r->model_audit[0]=R32(a,0x2820);r->model_audit[1]=R32(a,0x204c+0x184);r->model_audit[2]=R32(a,0x2200+0x184);r->model_audit[3]=R32(a,0x238c+0x184);
 /* Padding and unknown point-entry bytes are excluded from replay. No model,
  * collider, line buffer, callback or actor address is installed anywhere. */
 for(j=0;j<3;j++)r->reserved[j]=0;
 if(!value_valid(r)||!process_same(e))return 28;r->checksum=hash(r,HRP_ROW,196,0);return 0;
}
u32 _gz_hrp_stage_capture(void){HrpBank*p=&_gz_hrp_stage;const VrWorld*w=&_gz_vws_stage;const BbBank*b=&_gz_bcb_staging;u32 i,z,serial;
 if(_gz_roh_user_guard())return 5;_gz_hrp_ready=0;
 z=_gz_vws_validate_ready();if(z)return z;z=_gz_bcb_validate(b,b->bytes,w);if(z)return z;
 z=_gz_roh_work_replaced();if(z)return z;serial=_gz_po_header.serial;
 for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;
 p->magic=HRP_MAGIC;p->version=1;p->frame=w->frame;p->epoch=w->epoch;p->session=w->session;p->stage0=w->stage0;p->stage1=w->stage1;p->room=w->room;p->layer=w->layer;
 p->world_hash=w->checksum;p->world_sequence=w->sequence;p->layout=layout();
 for(i=0;i<w->count;i++)if(w->actors[i].profile==446){
  if(p->count==HRP_CAP)return 28;z=capture_row(&p->rows[p->count],i,w,b);if(z)return z;
  if(!p->count){copy(_gz_hrp_native_audit,(const void*)w->actors[i].actor,sizeof(_gz_hrp_native_audit));
   _gz_hrp_native_audit_meta[0]=w->frame;_gz_hrp_native_audit_meta[1]=w->actors[i].pid;
   _gz_hrp_native_audit_meta[2]=b->rows[i].process.generation;_gz_hrp_native_audit_meta[3]=w->actors[i].actor;}
  p->count++;
 }
 if(serial!=_gz_po_header.serial||w->frame!=_gz_roh_live_frame||w->epoch!=_gz_roh_live_epoch)return 28;
 p->bytes=HRP_HEADER+p->count*HRP_ROW;p->valid=1;p->checksum=hash(p,p->bytes,56,1);
 z=_gz_hrp_validate(p,p->bytes,w,b);if(z){p->valid=0;return z;}_gz_hrp_ready=1;return 0;
}
u32 _gz_hrp_validate_ready(void){const HrpBank*p=&_gz_hrp_stage;u32 z,i,j,k,a,q;const HrpRow*r;
 if(_gz_hrp_ready!=1)return 28;z=_gz_vws_validate_ready();if(z)return z;
 z=_gz_hrp_validate(p,p->bytes,&_gz_vws_stage,&_gz_bcb_staging);if(z)return z;
 for(i=0;i<p->count;i++){r=&p->rows[i];a=r->process.object;if(!process_same(&r->process))return 28;k=0;
  for(j=0;j<_gz_hrp_field_count();j++){if(!same(r->scalars+k,(const void*)(a+hrp_fields[j].offset),hrp_fields[j].bytes))return 28;k+=(hrp_fields[j].bytes+3)&~3u;}
  for(j=0;j<200;j++){q=a+(j<100?0x42c+j*36:0x123c+(j-100)*36);
   if(!same(r->points+j*20,(const void*)(q+12),16)||!same(r->points+j*20+16,(const void*)(q+28),4))return 28;}
  if(r->refs[0].audit!=R32(a,0x267c)||r->refs[101].audit!=R32(0x10464678,0)||r->refs[102].audit!=R32(0x1046467c,0))return 28;
  for(j=0;j<100;j++)if(r->refs[j+1].audit!=R32(a,0x268c+j*4))return 28;
  if(r->model_audit[0]!=R32(a,0x2820)||r->model_audit[1]!=R32(a,0x21d0)||r->model_audit[2]!=R32(a,0x2384)||r->model_audit[3]!=R32(a,0x2510))return 28;
 }
 return 0;
}
void _gz_hrp_commit(void){u32 i,n=_gz_hrp_stage.bytes;if(_gz_roh_user_guard()||_gz_hrp_ready!=1||n<HRP_HEADER||n>HRP_BYTES)return;
 _gz_hrp_source.valid=0;for(i=0;i<n;i++)if(i<12||i>=16)((u8*)&_gz_hrp_source)[i]=((u8*)&_gz_hrp_stage)[i];
 __asm__ volatile("sync":::"memory");_gz_hrp_source.valid=1;_gz_hrp_ready=0;
}
void _gz_hrp_forget_legacy(void){if(!_gz_roh_user_guard())_gz_hrp_source.valid=0;}
u32 _gz_hrp_validate_current(void){u32 z=_gz_vws_validate_current();if(z)return z;return _gz_hrp_validate(&_gz_hrp_source,_gz_hrp_source.bytes,&_gz_vws_source,&_gz_bcb_work);}
u32 _gz_hrp_archive_validate(u32 p,u32 n,u32 installed){u32 c=gz_arc_shape(p,n),o,b;
 if(!c)return 42;if(c==25)return installed&&_gz_hrp_source.valid?42:0;
 o=R32(p,216);b=R32(p,220);
 if(_gz_hrp_validate((const HrpBank*)(p+o),b,(const VrWorld*)(p+R32(p,168)),(const BbBank*)(p+R32(p,192))))return 42;
 return installed&&!same(&_gz_hrp_source,(const void*)(p+o),b)?42:0;
}
extern volatile u32 _gz_slots_plan[];
static const u32 _gz_hrp_legacy_zero=0;
u32 _gz_hrp_prepare_install(u32 p){u32 n=R32(p,4),c=gz_arc_shape(p,n);volatile u32*q=_gz_slots_plan+100;
 if(!c||_gz_hrp_archive_validate(p,n,0))return 42;
 if(c==25){q[0]=(u32)&_gz_hrp_legacy_zero;q[1]=4;q[2]=(u32)&_gz_hrp_source.valid;q[3]=4;}
 else{q[0]=p+R32(p,216);q[1]=R32(p,220);q[2]=(u32)&_gz_hrp_source;q[3]=HRP_BYTES;}
 return 0;
}
