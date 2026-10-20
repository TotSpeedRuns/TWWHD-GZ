#include "archive_layout.h"
/* Native PF2 capture storage, additive to the retained legacy pot recipes.
 * This producer/validator grants no collision/lifetime or replay authority. */
#include "../pot-family-state-20261017/family_state.h"
#include "../void-rebuild-correspondence-20261015/correspondence.h"
typedef unsigned char u8;
#define R32(p,o) (*(volatile const u32*)((p)+(o)))
#define R16(p,o) (*(volatile const unsigned short*)((p)+(o)))
#define R8(p,o) (*(volatile const u8*)((p)+(o)))
#define W32(p,o,v) (*(volatile u32*)((p)+(o))=(v))
enum {PB_MAGIC=0x50464232u,PB_HEADER=64,PB_CAP=64,PB_BYTES=64+64*PF_BYTES};
u8 _gz_pfb_work[PB_BYTES] __attribute__((aligned(4)));
u8 _gz_pfb_staging[PB_BYTES] __attribute__((aligned(4)));
volatile u32 _gz_pfb_ready,_gz_pfb_report[8]={0x50464252u,32,1};
extern VrWorld _gz_vws_source,_gz_vws_stage;
extern u8 _gz_pb_work[];
extern volatile u32 _gz_roh_scene_header[16];
extern u32 _gz_roh_user_guard(void),_gz_roh_work_replaced(void);
extern u32 _gz_vws_validate(const VrWorld*,u32),_gz_pb_validate(u32,u32);
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static u32 apart(u32 p,u32 n,u32 q,u32 m){return p+n>=p&&q+m>=q&&(p+n<=q||q+m<=p);}
static u32 same(u32 p,u32 q,u32 n){u32 i;for(i=0;i<n;i++)if(R8(p,i)!=R8(q,i))return 0;return 1;}
static void copy(u32 d,u32 s,u32 n){u32 i;for(i=0;i<n;i++)*(volatile u8*)(d+i)=R8(s,i);}
static u32 hash(u32 p,u32 n){u32 i,h=2166136261u;for(i=0;i<n;i++)h=(h^((i>=56&&i<60)?0:(i>=12&&i<16)?(i==15?1:0):R8(p,i)))*16777619u;return h;}
static u32 clocks(const VrWorld*w){return _gz_pt_epoch==w->frame&&_gz_roh_scene_header[0]==0x53425031u&&
 _gz_roh_scene_header[1]==1&&!(_gz_roh_scene_header[4]&0x84u)&&_gz_roh_scene_header[2]==w->epoch&&w->epoch&&w->epoch<=0x00ffffffu;}
static u32 member(const VrWorld*w,u32 p){u32 i;for(i=0;i<w->count;i++)if(w->actors[i].pid==R32(p,20))return i;return VR_CAPACITY;}
static u32 check(u32 p,u32 n,const VrWorld*w,u32 valid){u32 i,j,count,pots=0,pf,index;const VrActor*a;
 if(n<PB_HEADER||n>PB_BYTES||!span(p,n)||_gz_vws_validate(w,sizeof(*w)))return 25;
 if(R32(p,0)!=PB_MAGIC||R32(p,4)!=n||R32(p,8)!=2||R32(p,12)!=valid||R32(p,16)>PB_CAP||
  n!=PB_HEADER+R32(p,16)*PF_BYTES||R32(p,20)!=w->frame||R32(p,24)!=w->epoch||R32(p,28)!=w->session||
  R32(p,32)!=w->stage0||R32(p,36)!=w->stage1||R32(p,40)!=w->room||R32(p,44)!=w->layer||
  R32(p,48)!=w->checksum||R32(p,52)!=w->sequence||R32(p,56)!=hash(p,n)||R32(p,60))return 25;
 count=R32(p,16);for(i=0;i<w->count;i++)if(w->actors[i].profile==453)pots++;
 if(count!=pots)return 25;
 for(i=0;i<count;i++){
  pf=p+PB_HEADER+i*PF_BYTES;if(_gz_pf_packet_check(pf))return 25;
  index=member(w,pf);if(index==VR_CAPACITY)return 25;a=&w->actors[index];
  if(a->profile!=453||R32(pf,24)!=w->frame||R32(pf,64)!=a->actor||R32(pf,36)!=a->parameters||
   R32(pf,40)!=a->set_id||R32(pf,44)!=a->parent_pid||!same(pf+72,(u32)a->home,20))return 25;
  for(j=0;j<i;j++)if(R32(p+PB_HEADER+j*PF_BYTES,20)==a->pid)return 25;
 }
 return 0;
}
/* Validate saved typed state against BOTH captured world identity and the
 * actual legacy constructor observations. No saved actor pointer is read. */
u32 _gz_pfb_validate(u32 p,u32 n,const VrWorld*w,u32 old,u32 old_bytes){u32 e,i,j,pf,row,count;
 e=check(p,n,w,1);if(e)return e;
 if(_gz_pb_validate(old,old_bytes)||R32(old,16)!=R32(p,16)||R32(old,28)!=w->frame||
  R32(old,48)!=w->stage0||R32(old,52)!=w->stage1||R32(old,56)!=((w->room<<8)|w->layer))return 25;
 count=R32(p,16);
 for(i=0;i<count;i++){
  pf=p+PB_HEADER+i*PF_BYTES;row=0;
  for(j=0;j<count;j++)if(R32(old+64+j*1120,0)==R32(pf,20)){if(row)return 25;row=old+64+j*1120;}
  if(!row||R32(row,4)!=453||R32(row,8)!=R32(pf,28)||R32(row,12)!=R32(pf,32)||
   R32(row,24)!=R32(pf,44)||R32(row,28)!=R32(pf,40)||R32(row,68)!=R32(pf,36)||R32(row,72)!=R32(pf,48)||
   !same(row+36,pf+72,16)||!same(row+54,pf+90,2)||R16(row,52)!=R32(pf,48)||R16(pf,88))return 25;
 }
 return 0;
}
u32 _gz_pfb_stage_capture(void){const VrWorld*w=&_gz_vws_stage;u32 p=(u32)_gz_pfb_staging,i,n=0,pf,e,bytes;
 if(_gz_roh_user_guard())return 5;
 _gz_pfb_ready=0;
 if(_gz_vws_validate(w,sizeof(*w))||!clocks(w))return 25;
 for(i=0;i<w->count;i++)if(w->actors[i].profile==453)n++;
 if(n>PB_CAP)return 25;
 bytes=PB_HEADER+n*PF_BYTES;
 if(!apart(p,bytes,(u32)w,sizeof(*w)))return 25;
 for(i=0;i<w->count;i++)if(!apart(p,bytes,w->actors[i].actor,w->actors[i].profile==453?2364:0x300))return 25;
 e=_gz_roh_work_replaced();if(e)return e;
 W32(p,12,0);for(i=0;i<bytes;i++)_gz_pfb_staging[i]=0;n=0;
 for(i=0;i<w->count;i++)if(w->actors[i].profile==453){
  pf=p+PB_HEADER+n*PF_BYTES;e=_gz_pf_capture(w->actors[i].actor,pf,w->actors[i].pid,w->frame);if(e)return e;n++;
 }
 W32(p,0,PB_MAGIC);W32(p,4,bytes);W32(p,8,2);W32(p,16,n);W32(p,20,w->frame);W32(p,24,w->epoch);W32(p,28,w->session);
 W32(p,32,w->stage0);W32(p,36,w->stage1);W32(p,40,w->room);W32(p,44,w->layer);W32(p,48,w->checksum);W32(p,52,w->sequence);
 W32(p,56,hash(p,bytes));e=check(p,bytes,w,0);if(e)return e;if(!clocks(w))return 9;
 __asm__ volatile("sync":::"memory");W32(p,12,1);_gz_pfb_ready=1;return 0;
}
u32 _gz_pfb_validate_ready(void){u32 e;if(!_gz_pfb_ready||!clocks(&_gz_vws_stage))return 25;
 e=check((u32)_gz_pfb_staging,R32((u32)_gz_pfb_staging,4),&_gz_vws_stage,1);return e;
}
void _gz_pfb_commit(void){if(_gz_roh_user_guard())return;if(_gz_pfb_ready){copy((u32)_gz_pfb_work,(u32)_gz_pfb_staging,R32((u32)_gz_pfb_staging,4));_gz_pfb_ready=0;}}
void _gz_pfb_discard(void){if(_gz_roh_user_guard())return;_gz_pfb_ready=0;}
u32 _gz_pfb_validate_current(void){return _gz_pfb_validate((u32)_gz_pfb_work,R32((u32)_gz_pfb_work,4),&_gz_vws_source,(u32)_gz_pb_work,R32((u32)_gz_pb_work,8));}
/* Caller supplies a genuine metadata-issued source extent/read pin. This
 * repeats the directory shape before reading the new component's members. */
u32 _gz_pfb_archive_validate(u32 p,u32 n,u32 installed){u32 pf,pbytes,world,old,oldbytes,e;
 if(!gz_arc_shape(p,n)||R32(p,172)!=sizeof(VrWorld))return 42;
 pf=p+R32(p,184);pbytes=R32(p,188);world=p+R32(p,168);old=p+R32(p,160);oldbytes=R32(p,164);
 e=_gz_pfb_validate(pf,pbytes,(const VrWorld*)world,old,oldbytes);if(e)return 42;
 if(installed&&!same((u32)_gz_pfb_work,pf,pbytes))return 42;
 return 0;
}
