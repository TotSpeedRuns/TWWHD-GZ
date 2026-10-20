/* Bomb294 immutable source adapter. No main-schema/component-count assumption.
 * No Create/Delete, replay, bind, owner count transfer, effect or game stores. */
#include "inventory_bomb.h"
#include "typed_core.inc"
#define NOINLINE __attribute__((noinline))
u8 _gz_b294_source[B294_BYTES] __attribute__((aligned(4)));
u8 _gz_b294_stage[B294_BYTES] __attribute__((aligned(4)));
volatile u32 _gz_b294_ready;
extern u32 _gz_roh_user_guard(void);
static u32 native_span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static u32 input_span(u32 p,u32 n){return native_span(p,n)||((p==(u32)_gz_b294_source||p==(u32)_gz_b294_stage)&&n<=B294_BYTES);}
static u32 hash(u32 p){u32 i,h=2166136261u;for(i=0;i<B294_BYTES;i++)h=(h^((i>=16&&i<20)?0:R8(p,i)))*16777619u;return h;}
static u32 equal(u32 a,u32 b,u32 n){u32 i;for(i=0;i<n;i++)if(R8(a,i)!=R8(b,i))return 0;return 1;}
static void zero(u32 p,u32 n){u32 i;for(i=0;i<n;i++)*(volatile u8*)(p+i)=0;}
static void copy_bytes(u32 d,u32 s,u32 n){u32 i;for(i=0;i<n;i++)*(volatile u8*)(d+i)=R8(s,i);}
static u32 overlap(u32 a,u32 an,u32 b,u32 bn){return a<b+bn&&b<a+an;}
static u32 context_ok(const PbContext*c){u32 p=(u32)c;return !(p&3)&&p>=0x01000000u&&p<0xbffe0000u&&sizeof(*c)<=0xbffe0000u-p&&c->room<=255&&c->layer<=255;}
u32 _gz_b294_archive_bytes(void){return B294_BYTES;}
u32 _gz_b294_validate(u32 p,u32 n){u32 i,j;const PbBank*b;
 if(n!=B294_BYTES||!input_span(p,n))return 28;
 if(R32(p,0)!=0x42323934u||R32(p,4)!=B294_VERSION||R32(p,8)!=n||R32(p,12)!=1||hash(p)!=R32(p,16))return 28;
 for(i=48;i<B294_HEADER;i++)if(R8(p,i))return 28;
 if(R32(p,28)>255||R32(p,32)>255||R32(p,44)!=453)return 28;
 b=(const PbBank*)(p+B294_HEADER);
 if(_gz_b294_core_bank_record_safe(b))return 28;
 for(i=0;i<b->count;i++){
  const PbRow*r=&b->rows[i];
  if(!equal((u32)&r->context,p+20,sizeof(PbContext)))return 28;
  for(j=r->scalar_bytes;j<PB_FIELD_BYTES;j++)if(r->fields[j])return 28;
  for(j=0;j<sizeof(r->reserved);j++)if(r->reserved[j])return 28;
 }
 for(i=B294_HEADER+32+b->count*PB_ROW_BYTES;i<B294_BYTES;i++)if(R8(p,i))return 28;
 return 0;
}
/* Frame/world context is copied before any staging write. Caller owns the
 * coherent frame; a second context comparison catches a changed token. */
NOINLINE u32 _gz_b294_stage_capture(u32 q,const PbContext*c){u32 e,p=(u32)_gz_b294_stage;PbContext start;
 if(_gz_roh_user_guard())return 5;
 _gz_b294_ready=0;
 if(!context_ok(c))return 28;
 if(overlap((u32)c,sizeof(*c),(u32)_gz_b294_source,B294_BYTES)||overlap((u32)c,sizeof(*c),p,B294_BYTES))return 28;
 start=*c;zero(p,B294_BYTES);
 e=_gz_b294_core_bank_capture((PbBank*)(p+B294_HEADER),q,&start);if(e)return e;
 if(!equal((u32)c,(u32)&start,sizeof(start)))return 26;
 *(volatile u32*)(p+0)=0x42323934u;*(volatile u32*)(p+4)=B294_VERSION;*(volatile u32*)(p+8)=B294_BYTES;
 copy_bytes(p+20,(u32)&start,sizeof(start));*(volatile u32*)(p+44)=453;
 *(volatile u32*)(p+12)=1;*(volatile u32*)(p+16)=hash(p);
 e=_gz_b294_validate(p,B294_BYTES);if(e){*(volatile u32*)(p+12)=0;return e;}
 if(!equal((u32)c,(u32)&start,sizeof(start)))return 26;
 __asm__ volatile("sync":::"memory");_gz_b294_ready=1;return 0;
}
NOINLINE u32 _gz_b294_validate_ready(const PbContext*c){u32 e,p=(u32)_gz_b294_stage;
 if(_gz_b294_ready!=1||!context_ok(c))return 28;
 if(!equal(p+20,(u32)c,sizeof(*c)))return 26;
 e=_gz_b294_validate(p,B294_BYTES);if(e)return e;
 return equal(p+20,(u32)c,sizeof(*c))?0:26;
}
NOINLINE void _gz_b294_discard(void){if(!_gz_roh_user_guard())_gz_b294_ready=0;}
/* Infallible caller-authorized final tail. No native callback or lookup. */
NOINLINE void _gz_b294_commit(void){u32 d=(u32)_gz_b294_source,s=(u32)_gz_b294_stage;
 if(_gz_roh_user_guard()||_gz_b294_ready!=1)return;
 *(volatile u32*)(d+12)=0;copy_bytes(d,s,12);copy_bytes(d+16,s+16,B294_BYTES-16);
 __asm__ volatile("sync":::"memory");*(volatile u32*)(d+12)=1;_gz_b294_ready=0;
}
NOINLINE u32 _gz_b294_capture(u32 q,const PbContext*c){u32 e=_gz_b294_stage_capture(q,c);
 if(!e)e=_gz_b294_validate_ready(c);if(e){_gz_b294_discard();return e;}_gz_b294_commit();return 0;
}
u32 _gz_b294_restore_admission(u32 p,u32 n){u32 e=_gz_b294_validate(p,n);if(e)return e;return R32(p+B294_HEADER,8)?28:0;}
