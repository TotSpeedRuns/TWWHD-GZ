/* Fixed CLOCK/PERSIST private borrows of the existing actual transaction
 * tokens. Include after actual WHL/NBD code. Never begins/ends HG or Gate. */
#include "wct_root_borrow.h"
typedef struct {u32 kind,mode,serial,frame,thread,identity,hg,seal;} WhlRootBorrow;
typedef char WhlRootBorrowSize[sizeof(WhlRootBorrow)==32?1:-1];
static WhlRootBorrow whl_root __attribute__((section(".bss.gz_whl_root")));
static void whl_root_seal(void){whl_root.seal=btr_hash(&whl_root,sizeof(whl_root)-4);__asm__ volatile("sync":::"memory");}
u32 _gz_wct_root_pending(void){const u32*p=(const u32*)&whl_root;u32 i;
 for(i=0;i<sizeof(whl_root)/4;i++)if(p[i])return 1;return 0;}
static u32 whl_root_valid(void){return (whl_root.kind==WHL_ROOT_CLOCK||whl_root.kind==WHL_ROOT_PERSIST)&&
 whl_root.mode>=2&&whl_root.mode<=5&&whl_root.serial&&whl_root.frame&&whl_root.thread&&
 whl_root.identity&&whl_root.hg&&whl_root.seal==btr_hash(&whl_root,sizeof(whl_root)-4);}
/* Exact scalar phase policy, shared by both real typed kernel plans.
 * This function alone issues no ownership or native access authority. */
static u32 whl_root_topology_phase(u32 kind,u32 mode,u32 phase){
 if(kind!=WHL_ROOT_CLOCK&&kind!=WHL_ROOT_PERSIST)return 0;
 if(mode==2)return phase==1;
 if(mode==3)return phase==2;
 if(mode==4)return phase==4;
 if(mode==5)return phase==1||phase==3||phase==5;
 return 0;
}
/* Read-only membership of an ACTUAL outstanding fixed root borrow. This
 * deliberately does not call root_held, CKR/PRK kernel_scope or owner common
 * validation: each may reach CTP again. ctp_held independently authenticates
 * both real tokens and all original WCT/ROH phase/service conditions. */
static u32 whl_root_topology_member(u32 token,u32 frame){u32 kind,mode,serial;
 if(!whl_root_valid()||!whl_valid()||whl.phase!=WHL_HELD||whl.error||
  whl.token!=token||whl.frame!=frame||frame!=_gz_pbac_frame||
  !whl.hg||!whl.identity||whl_root.frame!=frame||
  whl_root.thread!=whl.thread||whl_root.identity!=whl.identity||whl_root.hg!=whl.hg||
  whl.thread!=_gz_hg_platform_thread()||whl.parent!=_gz_wct_native_owned())return 0;
 kind=whl_root.kind;mode=whl_root.mode;serial=whl_root.serial;
 if(whl.borrowed!=4+kind||!_gz_srh_wct_root_member(kind,mode,serial,frame))return 0;
 if(kind==WHL_ROOT_CLOCK){
  if(_gz_ckr_entered!=1||_gz_ckr_kernel_mode!=mode||_gz_ckr_kernel_frame!=frame||
   !ckr_sealed()||_gz_ckr_plan.token!=token||_gz_ckr_plan.frame!=frame||
   !whl_root_topology_phase(kind,mode,_gz_ckr_plan.phase))return 0;
 }else if(kind==WHL_ROOT_PERSIST){
  if(_gz_prk_entered!=1||_gz_prk_kernel_mode!=mode||_gz_prk_kernel_frame!=frame||
   !prk_sealed()||_gz_prk_plan.token!=token||_gz_prk_plan.frame!=frame||
   !whl_root_topology_phase(kind,mode,_gz_prk_plan.phase))return 0;
 }else return 0;
 __asm__ volatile("sync":::"memory");
 return whl_root_valid()&&whl_valid()&&whl.phase==WHL_HELD&&!whl.error&&
  whl.token==token&&whl.frame==frame&&whl.borrowed==4+kind&&
  whl.identity==whl_root.identity&&whl.hg==whl_root.hg&&
  whl_root.kind==kind&&whl_root.mode==mode&&whl_root.serial==serial&&
  _gz_srh_wct_root_member(kind,mode,serial,frame)?1u:0u;
}
u32 _gz_wct_root_held(u32 kind,u32 mode,u32 serial,u32 frame,u32 actual_hg){
 if(!whl_root_valid()||whl_root.kind!=kind||whl_root.mode!=mode||whl_root.serial!=serial||
  whl_root.frame!=frame||whl_root.hg!=actual_hg||!whl_valid()||whl.borrowed!=4+kind||
  whl.identity!=whl_root.identity||whl.hg!=actual_hg||whl.frame!=frame||
  whl.thread!=whl_root.thread||_gz_hg_platform_thread()!=whl_root.thread||
  !_gz_wct_heap_held(whl.token,frame)||!_gz_srh_wct_root_context(kind,mode,serial,frame))return 0;
 __asm__ volatile("sync":::"memory");
 return whl_root_valid()&&whl.borrowed==4+kind&&whl.identity==whl_root.identity&&
  _gz_wct_heap_identity(whl.token,frame)==whl_root.identity?1u:0u;
}
u32 _gz_wct_root_borrow(u32 kind,u32 mode,u32 serial,u32 frame){u32 result=0;
 if((kind!=WHL_ROOT_CLOCK&&kind!=WHL_ROOT_PERSIST)||mode<2||mode>5||!serial||
  _gz_wct_root_pending()||!whl_valid()||whl.borrowed||
  !_gz_jwf_atomic_cas(&whl_busy,0,1))return 0;
 if(_gz_wgp_unreleased()||!_gz_wct_heap_held(whl.token,frame)||
  !_gz_srh_wct_root_context(kind,mode,serial,frame))goto done;
 whl_root.kind=kind;whl_root.mode=mode;whl_root.serial=serial;whl_root.frame=frame;
 whl_root.thread=whl.thread;whl_root.identity=whl.identity;whl_root.hg=whl.hg;whl_root_seal();
 whl.borrowed=4+kind;whl_seal();_gz_wct_heap_report[15]=whl.borrowed;
 if(!_gz_wct_root_held(kind,mode,serial,frame,whl.hg)){(void)whl_fail(WHL_ROOT_RETAINED);goto done;}
 result=whl.hg;
done:whl_busy=0;return result;
}
u32 _gz_wct_root_unborrow(u32 kind,u32 mode,u32 serial,u32 frame,u32 actual_hg){u32 z=WHL_ROOT_RETAINED;
 if(!_gz_jwf_atomic_cas(&whl_busy,0,1))return z;
 if(!_gz_wct_root_held(kind,mode,serial,frame,actual_hg))goto done;
 /* Clear only this private borrower. Both actual tokens remain in WHL/JWA. */
 bcl_zero(&whl_root,sizeof(whl_root));whl.borrowed=0;whl_seal();_gz_wct_heap_report[15]=0;z=0;
done:whl_busy=0;return z;
}
