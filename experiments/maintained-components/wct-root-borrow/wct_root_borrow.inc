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
