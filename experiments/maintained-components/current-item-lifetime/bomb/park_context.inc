/* Include after actual coupled WHL and NBD implementations. BORROWS their
 * tokens, never acquires another Gate or releases either real token. */
#include "park_admission.h"
typedef struct {u32 active,halted,token,frame,mode,hg,identity,parent,thread,serial;} IpkContext;
typedef char IpkContextSize[sizeof(IpkContext)==40?1:-1];
static IpkContext ipk_context __attribute__((section(".bss.gz_ipk_context")));
/* ABI2: 5..8MAX (no independent TRY/worker/API-check/LEAVE); 11token,
 * 12frame,13WGPmode,14HG,15NBDserial,16thread,17WHLidentity,
 * 18/19MAX(no local IRQ call),20borrowSerial,21fresh held predicate,
 * 22WORLD0,23missing31,24READ0,25session0,26mode2=WHL borrow.
 * Private diagnostics are not a coherent cut or a capability. */
volatile u32 _gz_ipk_admission_report[32] __attribute__((section(".data.gz_ipk_context")))={
 0x49504b31u,128,2,0,0,UINT32_MAX,UINT32_MAX,UINT32_MAX,
 UINT32_MAX,0,0,0,0,0,0,0,0,0,UINT32_MAX,UINT32_MAX,
 0,0,0,31,0,0,2,0,0,0,0,0};
static void ipk_barrier(void){__asm__ volatile("sync":::"memory");}
u32 _gz_ipk_admission_outstanding(void){return ipk_context.active||ipk_context.halted?1u:0u;}
static u32 ipk_halt(u32 why){if(!_gz_ipk_admission_report[10])_gz_ipk_admission_report[10]=why;
 ipk_context.halted=1;_gz_ipk_admission_report[3]=3;
 _gz_ipk_admission_report[9]=_gz_ipk_admission_outstanding();return IPK_BROKEN;}
u32 _gz_ipk_context_owned(u32 token){u32*q=wgp();
 if(!token||!ipk_context.active||ipk_context.halted||token!=ipk_context.token||
  !q||q[4]!=token||q[6]!=ipk_context.frame||q[7]!=ipk_context.mode||q[45]!=1||
  q[9]!=ipk_context.hg||!wgp_held(token,ipk_context.mode)||
  _gz_wct_heap_identity(token,ipk_context.frame)!=ipk_context.identity||
  _gz_wct_native_owned()!=ipk_context.parent||_gz_hg_platform_thread()!=ipk_context.thread)return 0;
 ipk_barrier();return ipk_context.active&&!ipk_context.halted&&q[45]==1&&
  q[9]==ipk_context.hg&&_gz_wct_heap_held(token,ipk_context.frame)?1u:0u;}
u32 _gz_ipk_admission_begin(u32 token,u32 frame,u32 mode){u32*q=wgp(),identity,parent,thread;
 volatile u32*r=_gz_ipk_admission_report;
 if(_gz_ipk_admission_outstanding())return IPK_BROKEN;
 if(!q||mode<1||mode>3||q[4]!=token||q[6]!=frame||q[7]!=mode||q[45]!=1||!q[9]||!wgp_held(token,mode))return IPK_CONTEXT;
 identity=_gz_wct_heap_identity(token,frame);parent=_gz_wct_native_owned();thread=_gz_hg_platform_thread();
 if(!identity||!parent||!thread)return IPK_CONTEXT;
 if(ipk_context.serial==BAD||r[4]==BAD)return ipk_halt(1);
 ++ipk_context.serial;++r[4];ipk_context.token=token;ipk_context.frame=frame;ipk_context.mode=mode;
 ipk_context.hg=q[9];ipk_context.identity=identity;ipk_context.parent=parent;ipk_context.thread=thread;
 r[11]=token;r[12]=frame;r[13]=mode;r[14]=q[9];r[15]=parent;r[16]=thread;r[17]=identity;r[20]=ipk_context.serial;
 ipk_barrier();ipk_context.active=1;r[3]=2;r[9]=1;
 if(!_gz_ipk_context_owned(token))return ipk_halt(2);r[21]=1;return IPK_OK;}
u32 _gz_ipk_admission_check(void){if(!_gz_ipk_context_owned(ipk_context.token))return IPK_BROKEN;
 _gz_ipk_admission_report[21]=1;return IPK_OK;}
u32 _gz_ipk_admission_end(void){if(!_gz_ipk_context_owned(ipk_context.token))return ipk_halt(4);
 /* End only this private borrow. WHL retains both actual tokens. */
 ipk_barrier();ipk_context.active=0;_gz_ipk_admission_report[3]=0;_gz_ipk_admission_report[9]=0;return IPK_OK;}
