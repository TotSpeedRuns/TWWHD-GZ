/* Actual fixed coreinit allocation, never a guessed entry in the game heap
 * tracker. The original MCA initializer alone calls this function, then zeroes
 * and publishes its three fixed regions. No runtime path releases this pool
 * or replaces any region pointer; the source audit pins that closed use set.
 * Receipt publication is not native actor-lifetime authority. Native actor
 * reads retain their independent root gate, process and game-heap checks. */
#include "keese_private_pool.h"
extern u32 _gz_kmp_native_base(u32),_gz_kmp_native_allocate(u32,u32,u32);
extern const u8 _gz_kmp_allocate_return[];
extern volatile u32 _gz_mca_blend_work,_gz_mca_blend_saved,_gz_mca_blend_observed;
extern u32 _gz_krc_private_apart(u32,u32),_gz_hg_platform_thread(void);
KmpIdentity _gz_kmp_identity;
static u32 _gz_kmp_entered;
u32 _gz_kmp_report[16]={0x4b4d5031u,64};
static u32 kmp_hash(const KmpIdentity*p){u32 i,h=2166136261u;const u8*q=(const u8*)p;
 for(i=0;i<sizeof(*p);i++)h=(h^((i>=8&&i<12)||(i>=44&&i<48)?0:q[i]))*16777619u;return h;}
u32 _gz_kmp_private_apart(u32 p,u32 n){return
 epj_apart(p,n,(u32)&_gz_kmp_identity,sizeof(_gz_kmp_identity))&&
 epj_apart(p,n,(u32)&_gz_kmp_entered,4)&&
 epj_apart(p,n,(u32)_gz_kmp_report,sizeof(_gz_kmp_report));}
u32 _gz_kmp_allocate(void){KmpIdentity*p=&_gz_kmp_identity;u32 thread,heap,buffer,frame=_gz_pbac_frame;
 u32 site=(u32)__builtin_return_address(0);
 if(site!=(u32)_gz_kmp_allocate_return||_gz_kmp_entered||p->valid||p->generation||
  _gz_mca_blend_work||_gz_mca_blend_saved||_gz_mca_blend_observed||_gz_roh_user_guard())return 0;
 thread=_gz_hg_platform_thread();if(!thread||!frame)return 0;
 _gz_kmp_entered=1;_gz_kmp_report[2]++;_gz_kmp_report[3]=frame;
 heap=_gz_kmp_native_base(1);if(!heap){_gz_kmp_report[4]=1;goto fail;}
 /* These exact size/alignment arguments and the returned pointer are owned
  * by this call. Callers cannot supply or revive a historical allocation. */
 buffer=_gz_kmp_native_allocate(heap,0x2d0000,32);
 if(!buffer){_gz_kmp_report[4]=2;goto fail;}
 /* A nonnull return consumes this allocation attempt even if later validation
  * fails. Never orphan it and mint another receipt from a retry. */
 p->generation=1;
 if(!epj_ram(buffer,0x2d0000)||(buffer&31)||!_gz_krc_private_apart(buffer,0x2d0000)||
  _gz_hg_platform_thread()!=thread||_gz_pbac_frame!=frame){_gz_kmp_report[4]=3;goto fail;}
 eks_zero(p,sizeof(*p));p->magic=0x4b4d5031u;p->bytes=sizeof(*p);p->generation=1;
 p->pointer=buffer;p->heap=heap;p->allocation_bytes=0x2d0000;p->alignment=32;
 p->thread=thread;p->born_frame=frame;p->return_site=site;p->checksum=kmp_hash(p);
 __asm__ volatile("sync":::"memory");p->valid=1;
 _gz_kmp_report[4]=0;_gz_kmp_report[5]=buffer;_gz_kmp_report[6]=heap;
 _gz_kmp_entered=0;return buffer;
fail:_gz_kmp_entered=0;return 0;}
u32 _gz_kmp_validate(void){const KmpIdentity*p=&_gz_kmp_identity;u32 i;
 if(_gz_kmp_entered||p->magic!=0x4b4d5031u||p->bytes!=sizeof(*p)||p->valid!=1||p->generation!=1||
  !p->heap||p->allocation_bytes!=0x2d0000||p->alignment!=32||!p->born_frame||
  p->return_site!=(u32)_gz_kmp_allocate_return||p->checksum!=kmp_hash(p)||
  p->thread!=_gz_hg_platform_thread()||!p->thread||!epj_ram(p->pointer,0x2d0000)||(p->pointer&31)||
  _gz_mca_blend_work!=p->pointer||_gz_mca_blend_saved!=p->pointer+0xf0000||
  _gz_mca_blend_observed!=p->pointer+0x1e0000||!_gz_kmp_private_apart(p->pointer,0x2d0000))return 602;
 for(i=0;i<28;i++)if(p->reserved[i])return 602;return 0;}
