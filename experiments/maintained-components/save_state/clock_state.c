/* Typed clock packet. Canonical and environment clocks are captured together
 * under the real root lease. No canonical/actor/game-state stores occur here. */
#include "clock_state.h"
#include "archive_layout.h"
typedef unsigned char u8;
#define C32(p,o) (*(volatile const u32*)((u32)(p)+(o)))
CkbBank _gz_ckb_stage,_gz_ckb_source;
volatile u32 _gz_ckb_ready;
static u32 _gz_ckb_entered,_gz_ckb_frame,_gz_ckb_world_hash;
extern volatile u32 _gz_pbac_frame,_gz_pbac_pending,_gz_pbac_tools,_gz_pbac_world_resume_pending;
extern volatile u8 _gz_loading_store;
extern u32 _gz_sck_values[8],_gz_sck_capture(void),_gz_roh_user_guard(void),_gz_osa_pin_id(void),_gz_pbac_idle_for_capture(void);
extern volatile u32 _gz_slots_plan[];
static const u32 ckb_zero=0;
static u32 ckb_span(const void*p,u32 n){u32 a=(u32)p;return !(a&3)&&a>=0x10000000u&&a<0xbffe0000u&&n&&n<=0xbffe0000u-a;}
static u32 ckb_hash(const void*p){u32 i,h=2166136261u,b;
 for(i=0;i<CKB_BYTES;i++){b=((const u8*)p)[i];if(i>=72&&i<76)b=0;h=(h^b)*16777619u;}return h;
}
static u32 ckb_same(const void*a,const void*b,u32 n){u32 i;for(i=0;i<n;i++)if(((const u8*)a)[i]!=((const u8*)b)[i])return 0;return 1;}
static u32 ckb_finite(u32 x){return (x&0x7fffffffu)<0x7f800000u;}
static u32 ckb_quiet(void){return !_gz_pbac_pending&&!_gz_loading_store&&!_gz_pbac_world_resume_pending&&
 !_gz_roh_user_guard()&&!_gz_osa_pin_id()&&_gz_pbac_idle_for_capture();}
u32 _gz_ckb_capture_scope(u32 f){const VrWorld*w=&_gz_vws_stage;
 return _gz_ckb_entered==1&&f==_gz_ckb_frame&&f==_gz_pbac_frame&&w->frame==f&&
  w->complete==1&&w->checksum==_gz_ckb_world_hash&&ckb_quiet();
}
u32 _gz_ckb_validate(const CkbBank*p,u32 n,const VrWorld*w){u32 i;
 if(n!=CKB_BYTES||!ckb_span(p,CKB_BYTES)||_gz_vws_validate(w,sizeof(*w)))return 42;
 if(p->magic!=CKB_MAGIC||p->bytes!=CKB_BYTES||(p->version!=1&&p->version!=2)||p->valid!=1||p->checksum!=ckb_hash(p)||
  p->frame!=w->frame||p->epoch!=w->epoch||p->session!=w->session||p->stage0!=w->stage0||p->stage1!=w->stage1||
  p->room!=w->room||p->layer!=w->layer||p->world_hash!=w->checksum||p->world_sequence!=w->sequence||
  !ckb_finite(p->values[0])||!ckb_finite(p->values[2])||!ckb_finite(p->values[4])||p->values[1]>65535||p->values[3]>65535)return 42;
 /* v1 contains no air evidence. v2 preserves the exact signed meter words
 * and byte status; no guessed timer range or fabricated legacy values. */
 if(p->version==2&&p->reserved[2]>255)return 42;
 for(i=p->version==2?3:0;i<13;i++)if(p->reserved[i])return 42;return 0;
}
/* Staged-scope flag is private and never serialized. Each validation repeats
 * the fixed five native reads rather than trusting the earlier output array. */
static u32 ckb_read(void){const VrWorld*w=&_gz_vws_stage;u32 z;
 if(_gz_ckb_entered||!ckb_quiet()||w->frame!=_gz_pbac_frame)return 5;
 _gz_ckb_frame=w->frame;_gz_ckb_world_hash=w->checksum;_gz_ckb_entered=1;
 z=_gz_sck_capture();_gz_ckb_entered=0;return z;
}
u32 _gz_ckb_stage_capture(void){CkbBank*p=&_gz_ckb_stage;const VrWorld*w=&_gz_vws_stage;u32 i,z;
 _gz_ckb_ready=0;z=_gz_vws_validate_ready();if(z)return z;z=ckb_read();if(z)return z;
 for(i=0;i<sizeof(*p)/4;i++)((u32*)p)[i]=0;
 p->magic=CKB_MAGIC;p->bytes=CKB_BYTES;p->version=2;p->valid=1;p->frame=w->frame;p->epoch=w->epoch;p->session=w->session;
 p->stage0=w->stage0;p->stage1=w->stage1;p->room=w->room;p->layer=w->layer;p->world_hash=w->checksum;p->world_sequence=w->sequence;
 for(i=0;i<5;i++)p->values[i]=_gz_sck_values[i];
 for(i=0;i<3;i++)p->reserved[i]=_gz_sck_values[5+i];p->checksum=ckb_hash(p);
 z=_gz_ckb_validate(p,CKB_BYTES,w);if(z){p->valid=0;return z;}_gz_ckb_ready=1;return 0;
}
u32 _gz_ckb_validate_ready(void){u32 i,z;if(_gz_ckb_ready!=1)return 42;
 z=_gz_vws_validate_ready();if(z)return z;z=_gz_ckb_validate(&_gz_ckb_stage,CKB_BYTES,&_gz_vws_stage);if(z)return z;
 z=ckb_read();if(z)return z;for(i=0;i<5;i++)if(_gz_ckb_stage.values[i]!=_gz_sck_values[i])return 42;
 if(_gz_ckb_stage.version==2)for(i=0;i<3;i++)if(_gz_ckb_stage.reserved[i]!=_gz_sck_values[5+i])return 42;return 0;
}
void _gz_ckb_commit(void){if(_gz_ckb_ready==1&&!_gz_roh_user_guard()){
 u32 i;_gz_ckb_source.valid=0;
 for(i=0;i<CKB_BYTES;i++)if(i<12||i>=16)((u8*)&_gz_ckb_source)[i]=((const u8*)&_gz_ckb_stage)[i];
 __asm__ volatile("sync":::"memory");_gz_ckb_source.valid=1;_gz_ckb_ready=0;}}
void _gz_ckb_discard(void){if(!_gz_roh_user_guard())_gz_ckb_ready=0;}
u32 _gz_ckb_validate_current(void){return _gz_ckb_validate(&_gz_ckb_source,CKB_BYTES,&_gz_vws_source);}
u32 _gz_ckb_archive_validate(u32 p,u32 n,u32 installed){u32 c=gz_arc_shape(p,n),o,b;
 if(!c)return 42;if(c<27)return installed&&_gz_ckb_source.valid?42:0;
 o=C32(p,224);b=C32(p,228);
 if(_gz_ckb_validate((const CkbBank*)(p+o),b,(const VrWorld*)(p+C32(p,168))))return 42;
 return installed&&!ckb_same(&_gz_ckb_source,(const void*)(p+o),CKB_BYTES)?42:0;
}
u32 _gz_ckb_prepare_install(u32 p){u32 c=gz_arc_shape(p,C32(p,4));volatile u32*q=_gz_slots_plan+104;
 if(!c||_gz_ckb_archive_validate(p,C32(p,4),0))return 42;
 if(c<27){q[0]=(u32)&ckb_zero;q[1]=4;q[2]=(u32)&_gz_ckb_source.valid;q[3]=4;}
 else {q[0]=p+C32(p,224);q[1]=CKB_BYTES;q[2]=(u32)&_gz_ckb_source;q[3]=CKB_BYTES;}
 return 0;
}
