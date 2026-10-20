/* Profile294, ordinary placed inventory bomb, type0/STATE1/native procWait.
 * Includes fuse/pose/physics/embedded clocks and named collision POD. This
 * isolated module never changes a procedure pointer, owner, resource object,
 * particle emitter, global collision registration or native creation queue.
 * Root integration must reconstruct an expired bomb through native creation.
 */
#include "typed_core.h"
#define R8(p,o) (*(const volatile u8*)((u32)(p)+(o)))
#define R16(p,o) (*(const volatile u16*)((u32)(p)+(o)))
#define R32(p,o) (*(const volatile u32*)((u32)(p)+(o)))
#define I(o,n) {o,n,4,0,0}
#define F(o,n) {o,n,4,1,0}
#define H(o,n) {o,n,2,0,0}
#define B(o,n) {o,n,1,0,0}
#define FC(o) F(o,2),H((o)+8,3),B((o)+14,2)
#define SPH(o) I(o,2),I((o)+0x10,1),B((o)+0x14,1),I((o)+0x18,2), \
 I((o)+0x28,1),I((o)+0x2c,2),I((o)+0x40,1),I((o)+0x50,2), \
 B((o)+0x64,1),B((o)+0x6c,4),F((o)+0x70,9),I((o)+0x94,2), \
 B((o)+0xa8,1),B((o)+0xb0,4),F((o)+0xb4,9),I((o)+0xdc,2), \
 B((o)+0xf0,1),F((o)+0x118,4)
const PbField _gz_b294_core_fields[]={
 I(0x2e0,2),F(0x300,3),H(0x30c,3),F(0x314,3),H(0x320,3),
 H(0x328,3),F(0x330,3),F(0x33c,3),F(0x370,3),F(0x390,3),I(0x39c,1),
 FC(0x3c0),FC(0x44c),FC(0x4d8),FC(0x550),
 F(0x8d4,3),B(0x8e0,3),
 /* dCcD_Stts POD; owner/PID/vtables stay native-owned. */
 F(0x8e4,3),B(0x8f8,3),B(0x904,3),H(0x908,1),I(0x91c,1),SPH(0x920),
 B(0xa70,8),I(0xa78,1),H(0xa7c,4),F(0xa84,9),
 B(0xaf8,1),F(0xafc,1),B(0xb00,7),I(0xb08,1),F(0xb0c,3),I(0xb4c,1)
};
const u32 _gz_b294_core_field_count=sizeof(_gz_b294_core_fields)/sizeof(_gz_b294_core_fields[0]);
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&p+n>=p&&p+n<=0xbffe0000u;}
static u32 finite(u32 v){return (v&0x7f800000u)!=0x7f800000u;}
static void copy(u8* d,const u8* s,u32 n){u32 i;for(i=0;i<n;i++)d[i]=s[i];}
static u32 bytes(void){u32 i,n=0;for(i=0;i<_gz_b294_core_field_count;i++)n+=_gz_b294_core_fields[i].count*_gz_b294_core_fields[i].width;return n;}
/* Byte decoding also handles the deliberately packed, unaligned row. */
static u32 saved(const PbRow* r,u32 o,u32 n){
 u32 i,j,p=0,v=0;
 for(i=0;i<_gz_b294_core_field_count;i++){
  const PbField* f=&_gz_b294_core_fields[i];u32 k=f->count*f->width;
  if(o>=f->offset&&o+n<=f->offset+k){for(j=0;j<n;j++)v=(v<<8)|r->fields[p+o-f->offset+j];return v;}
  p+=k;
 }
 return 0xffffffffu;
}
static u32 live_fields(void* a){u32 i,j;for(i=0;i<_gz_b294_core_field_count;i++)if(_gz_b294_core_fields[i].finite)for(j=0;j<_gz_b294_core_fields[i].count;j++)if(!finite(R32(a,_gz_b294_core_fields[i].offset+j*4)))return 28;return 0;}
u32 _gz_b294_core_supported(void* a){
 if(!span((u32)a,0xb50)||R16(a,8)!=294)return 28;
 if(R32(a,0x5c8)!=0||R32(a,0xb0)!=0x80000001u)return 28;
 /* GHS ptmf has two shorts and a function word, verified in procWait_init. */
 if(R16(a,0x3ac)!=0||R16(a,0x3ae)!=0xffff||R32(a,0x3b0)!=0x020c7358u)return 28;
 if(R16(a,0xa7c)<1||R16(a,0xa7c)>150||R8(a,0xb05)||R8(a,0x8e1))return 28;
 if(R32(a,0x2e0)&0x2000)return 28; /* carried state is a different owner closure */
 if(!span(R32(a,0x3bc),0xb0))return 28;
 return live_fields(a);
}
u32 _gz_b294_core_record_safe(const PbRow* r){
 u32 i,j,p=0;
 if(r->valid!=1||r->magic!=PB_MAGIC||r->bytes!=PB_ROW_BYTES||r->kind!=294||r->parameters!=0x80000001u||r->scalar_bytes!=bytes()||r->scalar_bytes>PB_FIELD_BYTES)return 28;
 if(r->context.room>255||r->context.layer>255)return 28;
 if(!span(r->actor,0xb50)||!span(r->resources[0],0xb0)||r->pid==0xffffffffu)return 28;
 if(saved(r,0xa7c,2)<1||saved(r,0xa7c,2)>150||saved(r,0xb05,1)||saved(r,0x8e1,1)||(saved(r,0x2e0,4)&0x2000))return 28;
 for(i=0;i<_gz_b294_core_field_count;i++){
  const PbField* f=&_gz_b294_core_fields[i];
  if(f->finite)for(j=0;j<f->count;j++){
   const u8* b=r->fields+p+j*4;
   u32 v=((u32)b[0]<<24)|((u32)b[1]<<16)|((u32)b[2]<<8)|b[3];
   if(!finite(v))return 28;
  }
  p+=f->count*f->width;
 }
 return 0;
}
u32 _gz_b294_core_capture(void* a,PbRow* r,const PbContext* c){
 u32 i,p=0,n;if(_gz_b294_core_supported(a)||c->room!=R8(a,0x326)||c->room>255||c->layer>255)return 28;
 n=bytes();if(n>PB_FIELD_BYTES)return 28;
 r->valid=0;r->magic=PB_MAGIC;r->bytes=PB_ROW_BYTES;r->kind=294;r->pid=R32(a,4);r->actor=(u32)a;r->parameters=R32(a,0xb0);r->parent=R32(a,0x2e8);
 r->resources[0]=R32(a,0x3bc);r->resources[1]=R32((void*)r->resources[0],0xac);
 r->resources[2]=R32(a,0x448);r->resources[3]=R32(a,0x45c);r->resources[4]=R32(a,0x4e8);r->resources[5]=R32(a,0x560);
 copy(r->home,(const u8*)a+0x2ec,20);r->context=*c;r->scalar_bytes=n;
 for(i=0;i<_gz_b294_core_field_count;i++){n=_gz_b294_core_fields[i].count*_gz_b294_core_fields[i].width;copy(r->fields+p,(const u8*)a+_gz_b294_core_fields[i].offset,n);p+=n;}
 r->valid=1;return 0;
}





u32 _gz_b294_core_bank_record_safe(const PbBank* b){
 u32 i,j,total=0;if(b->valid!=1||b->magic!=0x50424231||b->count>PB_CAPACITY||b->bytes!=PB_BANK_BYTES)return 28;
 for(i=0;i<4;i++)if(b->reserved[i])return 28;
 for(i=0;i<b->count;i++){
  if(_gz_b294_core_record_safe(&b->rows[i]))return 28;
  {u32 owner=saved(&b->rows[i],0xa70,1);if(owner>1)return 28;total+=owner;}
  for(j=0;j<i;j++)if(b->rows[i].pid==b->rows[j].pid)return 28;
 }
 return total<=3?0:28;
}


u32 _gz_b294_core_bank_capture(PbBank* b,u32 q,const PbContext* c){
 u32 i,n=0,limit=4096;b->valid=0;b->magic=0x50424231;b->bytes=PB_BANK_BYTES;b->count=0;
 for(i=0;i<4;i++)b->reserved[i]=0;
 while(q){
  u32 a;if(!limit--||!span(q,16))return 28;a=R32((void*)q,12);
  if(a){
   if(!span(a,12))return 28;
   if(R16((void*)a,8)==294){if(n==PB_CAPACITY||_gz_b294_core_capture((void*)a,&b->rows[n],c))return 28;n++;}
  }
  q=R32((void*)q,8);
 }
 b->count=n;b->valid=1;return _gz_b294_core_bank_record_safe(b);
}


