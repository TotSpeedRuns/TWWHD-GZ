/* One original-game process arena. No native heap/allocation body follows.
 * No late join to a sampled SDK/HL generation: both births occur before the
 * native RootHeap factory returns to its publishing caller. */
#include "process_arena.h"
extern u32 OSDisableInterrupts(void),OSRestoreInterrupts(u32);
extern u32 MEMGetBaseHeapHandle(u32);
extern int OSDynLoad_Acquire(const char*,u32*);
extern int OSDynLoad_FindExport(u32,int,const char*,void*);
/* Native CODE identities remain section-relative through the RPX loader.
 * These are address scalars only; no native instruction is followed. */
extern const unsigned char _native_game_literal_0273b86c[];
extern const unsigned char _native_game_literal_0273b8f4[];
extern const unsigned char _native_game_literal_02753a18[];
extern const unsigned char _native_game_literal_02753a9c[];
extern const unsigned char _native_game_literal_027567c0[];
extern const unsigned char _native_game_literal_02757624[];
extern const unsigned char _native_game_literal_02757628[];
extern const unsigned char _native_game_literal_0284c6ec[];
extern const unsigned char _native_game_literal_0284c708[];
extern const unsigned char _native_game_literal_0284ca4c[];
extern const unsigned char _native_game_literal_0284ca68[];
extern const unsigned char _native_game_literal_02859d38[];
extern const unsigned char _native_game_literal_02859d4c[];
extern const unsigned char _native_game_literal_0286ceb4[];
extern const unsigned char _native_game_literal_0286d2d0[];
extern const unsigned char _native_game_literal_0286d610[];
extern const unsigned char _native_game_literal_0286dc6c[];
extern const unsigned char _native_game_literal_0287dfe4[];

typedef u32 (*GpaFindContain)(void*);
#define GPA_DATA __attribute__((section(".data.gz_gpa")))
#define GPA_BSS __attribute__((section(".bss.gz_gpa")))
u32 _gz_gpa_report[32] GPA_DATA={0x47504131u,128,2};
static u32 gpa_busy GPA_BSS, gpa_faults GPA_BSS;
typedef struct {u32 ticket,serial,site;} GpaRelease;
static GpaRelease gpa_releases[32] GPA_BSS;
static u32 gpa_release_serial GPA_BSS,gpa_active_releases GPA_BSS;
static u32 gpa_pending_release_serial GPA_BSS;
static u32 gpa_resolver_state GPA_BSS,gpa_coreinit_module GPA_BSS;
static GpaFindContain gpa_find_contain GPA_BSS;
enum { P_PHASE=3,P_SERIAL=4,P_FLAGS=5,P_POINTER=6,P_BYTES=7,P_ALIGN=8,
 P_SDKHEAP=9,P_TARGET=10,P_SDKSITE=11,P_SDKLR=12,P_THREAD=13,
 P_ROOT=14,P_ROOTGEN=15,P_START=16,P_SPAN=17,P_CTORSITE=18,
 P_FACTORYLR=19,P_FACTORYTHREAD=20,P_TICKET=21,P_TOK=22,P_BORN=23,
 P_RETIRE=24,P_RETIRESITE=25,P_MATCHES=26,P_RESOLVE=27,
 P_MODULE=28,P_EXPORT=29,P_ACQUIRE_RAW=30,P_EXPORT_RAW=31 };
static void fault(u32 f){__sync_fetch_and_or(&gpa_faults,f);}
static u32 enter(u32*prior){*prior=OSDisableInterrupts();
 if(__sync_bool_compare_and_swap(&gpa_busy,0,1))return 1;
 OSRestoreInterrupts(*prior);return 0;
}
static void leave(u32 prior){__sync_synchronize();gpa_busy=0;OSRestoreInterrupts(prior);}
static u32 header(void){return _gz_gpa_report[0]==0x47504131u&&
 _gz_gpa_report[1]==128&&_gz_gpa_report[2]==2;}
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&
 p<0xc0000000u&&n>200&&!(n&3)&&n<=0xc0000000u-p;}
/* Mapping of t is supplied by the fixed native wrapper's owned stack.
 * This predicate does not turn an arbitrary numerical stack shape into it. */
static u32 ticket(const HgTicket*t,u32 s,u32 site){const volatile HgControl*c=&_gz_hg_control;
 return t&&s&&t->magic==HG_TICKET&&t->self==(u32)t&&t->serial==s&&
 t->site==site&&t->live==1&&c->magic==HG_MAGIC&&c->bytes==64&&
 c->version==1&&!c->flags&&c->state&&!(c->state&HG_EXCLUSIVE);
}
static void retire(u32 why,u32 site){_gz_gpa_report[P_PHASE]=GPA_RETIRED;
 _gz_gpa_report[P_RETIRE]=why;_gz_gpa_report[P_RETIRESITE]=site;}
/* Fixed original root SDK entry only, after CPU state save and BEFORE HG,
 * SK, HL or GPA metadata lock acquisition. coreinit is already a mandatory
 * imported resident module; this does not load an optional game resource. */
void _gz_gpa_resolve(u32 lr){u32 module=0;GpaFindContain found=0;int a,z;
 if(lr!=(u32)_native_game_literal_02757628){fault(GPA_PROTOCOL);return;}
 if(!__sync_bool_compare_and_swap(&gpa_resolver_state,0,1)){
  __sync_synchronize();
  if(gpa_resolver_state!=2||!gpa_coreinit_module||!gpa_find_contain)fault(GPA_SDK_OWNER);
  return;
 }
 _gz_gpa_report[P_RESOLVE]=1;
 a=OSDynLoad_Acquire("coreinit.rpl",&module);
 _gz_gpa_report[P_ACQUIRE_RAW]=(u32)a;
 if(a||!module)goto failed;
 z=OSDynLoad_FindExport(module,0,"MEMFindContainHeap",&found);
 _gz_gpa_report[P_EXPORT_RAW]=(u32)z;
 if(z||!found||((u32)found&3))goto failed;
 /* Only actual successful exported code/handle becomes the cached target.
  * No export result or pointer is supplied by a host, fixture or raw alias. */
 gpa_coreinit_module=module;gpa_find_contain=found;
 _gz_gpa_report[P_MODULE]=module;_gz_gpa_report[P_EXPORT]=(u32)found;
 __sync_synchronize();gpa_resolver_state=2;_gz_gpa_report[P_RESOLVE]=2;return;
failed:fault(GPA_SDK_OWNER);__sync_synchronize();gpa_resolver_state=3;
 _gz_gpa_report[P_RESOLVE]=3;
}
void _gz_gpa_sdk_prepare(const HgTicket*t,u32 s,u32 n,u32 align,u32 target,u32 lr){
 u32 prior,thread,base;
 if(!ticket(t,s,(u32)_native_game_literal_02757624)||lr!=(u32)_native_game_literal_02757628||align!=4||!target||
    (target&3)||n<=200||(n&3)){fault(GPA_PROTOCOL);return;}
 thread=_gz_hg_platform_thread();base=MEMGetBaseHeapHandle(1);
 if(!thread||!base){fault(GPA_SDK_OWNER);return;}
 if(!enter(&prior)){fault(GPA_DROP);return;}
 if(!header()||gpa_faults){fault(GPA_PROTOCOL);goto done;}
 if(_gz_gpa_report[P_PHASE]!=GPA_COLD||_gz_gpa_report[P_SERIAL]){
  fault(GPA_REPEATED);retire(GPA_REPEATED,(u32)_native_game_literal_02757624);goto done;}
 if(gpa_active_releases){retire(GPA_NATIVE,(u32)_native_game_literal_02757624);goto done;}
 _gz_gpa_report[P_SERIAL]=1; /* Single birth; never reset/rearm/reuse. */
 _gz_gpa_report[P_BYTES]=n;
 _gz_gpa_report[P_ALIGN]=align;_gz_gpa_report[P_SDKHEAP]=base;
 _gz_gpa_report[P_TARGET]=target;_gz_gpa_report[P_SDKSITE]=(u32)_native_game_literal_02757624;
 _gz_gpa_report[P_SDKLR]=lr;_gz_gpa_report[P_THREAD]=thread;
 _gz_gpa_report[P_TICKET]=(u32)t;_gz_gpa_report[P_TOK]=s;
 gpa_pending_release_serial=gpa_release_serial;
 _gz_gpa_report[P_PHASE]=GPA_PENDING;
done:_gz_gpa_report[P_FLAGS]=gpa_faults;leave(prior);
}
void _gz_gpa_sdk_return(const HgTicket*t,u32 s,u32 p,u32 n,u32 align,u32 target,u32 lr){
 u32 prior,thread,base,owner;
 if(!ticket(t,s,(u32)_native_game_literal_02757624)||lr!=(u32)_native_game_literal_02757628){fault(GPA_NATIVE);return;}
 /* The native allocator has returned. No SDK/HL/GPA metadata lock is held
  * across these genuine SDK queries. Returned p is not yet published. */
 thread=_gz_hg_platform_thread();base=MEMGetBaseHeapHandle(1);
 __sync_synchronize();
 if(gpa_resolver_state!=2||!gpa_coreinit_module||!gpa_find_contain){fault(GPA_SDK_OWNER);return;}
 owner=p?gpa_find_contain((void*)p):0;
 if(!ticket(t,s,(u32)_native_game_literal_02757624)){fault(GPA_NATIVE);return;}
 if(!enter(&prior)){fault(GPA_DROP);return;}
 if(!header()||gpa_faults||_gz_gpa_report[P_PHASE]!=GPA_PENDING||
    _gz_gpa_report[P_TICKET]!=(u32)t||_gz_gpa_report[P_TOK]!=s||
    _gz_gpa_report[P_THREAD]!=thread||_gz_gpa_report[P_BYTES]!=n||
    _gz_gpa_report[P_ALIGN]!=align||_gz_gpa_report[P_TARGET]!=target||
    base!=_gz_gpa_report[P_SDKHEAP]||owner!=base||!span(p,n)||
    gpa_active_releases||gpa_release_serial!=gpa_pending_release_serial){
  retire(GPA_NATIVE,(u32)_native_game_literal_02757624);goto done;}
 _gz_gpa_report[P_POINTER]=p;_gz_gpa_report[P_PHASE]=GPA_ALLOCATED;
done:_gz_gpa_report[P_FLAGS]=gpa_faults;leave(prior);
}
void _gz_gpa_factory_begin(const HgTicket*t,u32 s,u32 p,u32 n,u32 lr){u32 prior,thread;
 if(lr!=(u32)_native_game_literal_027567c0)return; /* Other root factories cannot adopt this arena. */
 if(!ticket(t,s,(u32)_native_game_literal_02753a18)){fault(GPA_NATIVE);return;}
 thread=_gz_hg_platform_thread();if(!thread){fault(GPA_NATIVE);return;}
 if(!enter(&prior)){fault(GPA_DROP);return;}
 if(!header()||gpa_faults||_gz_gpa_report[P_PHASE]!=GPA_ALLOCATED||
    p!=_gz_gpa_report[P_POINTER]||n!=_gz_gpa_report[P_BYTES]||
    thread!=_gz_gpa_report[P_THREAD]){fault(GPA_PROTOCOL);goto done;}
 _gz_gpa_report[P_FACTORYLR]=lr;_gz_gpa_report[P_FACTORYTHREAD]=thread;
 _gz_gpa_report[P_TICKET]=(u32)t;_gz_gpa_report[P_TOK]=s;
 _gz_gpa_report[P_PHASE]=GPA_CONSTRUCTING;
done:_gz_gpa_report[P_FLAGS]=gpa_faults;leave(prior);
}
void _gz_gpa_hl_born(const HlHeap*h,const HlControl*c){u32 prior;
 if(h->kind!=HL_EXP||h->ctor_site!=(u32)_native_game_literal_02753a9c||h->parent)return;
 /* No native API/heap query here. These are the actual producer's private
  * rows under its IRQ-safe HL metadata lock, before root publication. */
 if(!enter(&prior)){fault(GPA_DROP);return;}
 if(!header()||gpa_faults||_gz_gpa_report[P_PHASE]!=GPA_CONSTRUCTING||
    c->magic!=HL_MAGIC||c->bytes!=64||c->version!=1||c->busy!=1||c->flags||
    c->active_heaps>HL_HEAPS||c->active_allocations>HL_ALLOCS||
    h->state!=HL_LIVE||!h->generation||h->parent_generation||h->flags||h->epoch||
    h->vtable!=(u32)_gz_hl_exp_table||h->direction!=1||
    h->heap!=_gz_gpa_report[P_POINTER]||h->start!=h->heap||
    h->bytes!=_gz_gpa_report[P_BYTES]){fault(GPA_PROTOCOL);goto done;}
 _gz_gpa_report[P_ROOT]=h->heap;_gz_gpa_report[P_ROOTGEN]=h->generation;
 _gz_gpa_report[P_START]=h->start;_gz_gpa_report[P_SPAN]=h->bytes;
 _gz_gpa_report[P_CTORSITE]=h->ctor_site;_gz_gpa_report[P_BORN]=1;
 _gz_gpa_report[P_PHASE]=GPA_BORN;
done:_gz_gpa_report[P_FLAGS]=gpa_faults;leave(prior);
}
void _gz_gpa_factory_return(const HgTicket*t,u32 s,u32 p){u32 prior,thread;
 if(!ticket(t,s,(u32)_native_game_literal_02753a18)){fault(GPA_NATIVE);return;}
 thread=_gz_hg_platform_thread();
 if(!enter(&prior)){fault(GPA_DROP);return;}
 /* A different root-factory call never adopts a previously seen tuple. */
 if(_gz_gpa_report[P_TICKET]!=(u32)t||_gz_gpa_report[P_TOK]!=s)goto done;
 if(!header()||gpa_faults||_gz_gpa_report[P_PHASE]!=GPA_BORN||
    !_gz_gpa_report[P_BORN]||p!=_gz_gpa_report[P_ROOT]||
    thread!=_gz_gpa_report[P_FACTORYTHREAD]){fault(GPA_PROTOCOL);goto done;}
 _gz_gpa_report[P_PHASE]=GPA_LIVE;
done:_gz_gpa_report[P_FLAGS]=gpa_faults;leave(prior);
}
static u32 release_kind(u32 site){
 if(site==(u32)_native_game_literal_0273b86c||site==(u32)_native_game_literal_0273b8f4)return 1; /* default free */
 if(site==(u32)_native_game_literal_0284c6ec||site==(u32)_native_game_literal_0284ca4c||site==(u32)_native_game_literal_02859d38||site==(u32)_native_game_literal_02859d4c||
    site==(u32)_native_game_literal_0286ceb4||site==(u32)_native_game_literal_0286d610)return 2; /* Exp free */
 if(site==(u32)_native_game_literal_0284c708||site==(u32)_native_game_literal_0284ca68||site==(u32)_native_game_literal_0286d2d0||site==(u32)_native_game_literal_0286dc6c||
    site==(u32)_native_game_literal_0287dfe4)return 3; /* Exp/Frm destroy */
 return 0;}
void _gz_gpa_sdk_release(const HgTicket*t,u32 s,u32 a0,u32 a1){u32 prior,k,i;
 /* Site is supplied by that wrapper's actual ticket, not a host selection. */
 if(!t){fault(GPA_PROTOCOL);return;}
 k=release_kind(t->site);if(!k||!ticket(t,s,t->site)){fault(GPA_PROTOCOL);return;}
 if(!enter(&prior)){fault(GPA_DROP);return;}
 if(!header()){fault(GPA_PROTOCOL);goto done;}
 for(i=0;i<32;i++)if(!gpa_releases[i].serial)break;
 if(i==32||gpa_release_serial==0xffffffffu||gpa_active_releases>=32){
  fault(GPA_REPEATED);goto done;}
 gpa_releases[i].ticket=(u32)t;gpa_releases[i].site=t->site;
 gpa_releases[i].serial=s;++gpa_release_serial;++gpa_active_releases;
 /* A destruction/free may overlap a not-yet-observed returned address.
  * Refuse that birth, including releases already running at its begin. */
 if(_gz_gpa_report[P_PHASE]==GPA_PENDING)retire(GPA_NATIVE,t->site);
 if(_gz_gpa_report[P_PHASE]&&_gz_gpa_report[P_PHASE]!=GPA_RETIRED&&
    ((k==1&&a0==_gz_gpa_report[P_POINTER])||
     (k==2&&a0==_gz_gpa_report[P_SDKHEAP]&&a1==_gz_gpa_report[P_POINTER])||
     (k==3&&a0==_gz_gpa_report[P_SDKHEAP])))retire(k,t->site);
done:_gz_gpa_report[P_FLAGS]=gpa_faults;leave(prior);
}
void _gz_gpa_sdk_release_return(const HgTicket*t,u32 s){u32 prior,i;
 if(!t||!release_kind(t->site)||!ticket(t,s,t->site)){fault(GPA_NATIVE);return;}
 if(!enter(&prior)){fault(GPA_DROP);return;}
 for(i=0;i<32;i++)if(gpa_releases[i].serial==s&&gpa_releases[i].ticket==(u32)t&&
                         gpa_releases[i].site==t->site)break;
 if(i==32||!gpa_active_releases)fault(GPA_PROTOCOL);
 else {gpa_releases[i].serial=0;gpa_releases[i].ticket=0;
       gpa_releases[i].site=0;--gpa_active_releases;}
 _gz_gpa_report[P_FLAGS]=gpa_faults;leave(prior);
}
void _gz_gpa_hl_ending(u32 p,u32 kind,u32 site){u32 prior;
 /* Genuine destructor scalars, before fallible HL producer admission.
  * No native or private HL row pointer is followed on this path. */
 if(!p||p!=*(volatile u32*)(_gz_gpa_report+P_POINTER))return;
 if(!enter(&prior)){fault(GPA_DROP);return;}
 if(kind<HL_EXP||kind>HL_CUSTOM)fault(GPA_NATIVE);
 if(!header())fault(GPA_PROTOCOL);
 else retire(4,site);
 _gz_gpa_report[P_FLAGS]=gpa_faults;leave(prior);
}
u32 _gz_gpa_try_match(u32 token,u32 root,u32 generation,u32 start,u32 bytes){
 u32 prior,z=GPA_MISSING,i,at;const HlHeap*h=0;
 if(_gz_hg_capture_seal(token)!=HG_OK)return GPA_CHANGED;
 if(!_gz_hl_meta_try_lock())return GPA_BUSY;
 /* Fresh observer health/current identity, not a caller's sampled tuple.
  * Once per cut. No game body follows and no GPA -> HL lock acquisition. */
 if(_gz_hl_control.magic!=HL_MAGIC||_gz_hl_control.bytes!=64||
    _gz_hl_control.version!=1||_gz_hl_control.busy!=1||_gz_hl_control.flags||
    _gz_hl_control.active_heaps>HL_HEAPS||_gz_hl_control.active_allocations>HL_ALLOCS){
  z=GPA_FAULT;goto hl_done;}
 at=((root>>2)*0x9e3779b1u)>>20;
 for(i=0;i<HL_HEAPS;i++){
  const HlHeap*q=_gz_hl_heaps+((at+i)&(HL_HEAPS-1));
  if(!q->heap)break;
  if(q->heap==root){h=q;break;}
 }
 if(!h||!generation||h->state!=HL_LIVE||h->kind!=HL_EXP||
    h->generation!=generation||h->start!=start||h->bytes!=bytes||h->parent||
    h->parent_generation||h->vtable!=(u32)_gz_hl_exp_table||h->direction!=1||
    h->flags||h->ctor_site!=(u32)_native_game_literal_02753a9c)goto hl_done;
 if(!enter(&prior)){z=GPA_BUSY;goto hl_done;}
 if(!header()||gpa_faults)z=GPA_FAULT;
 else if(_gz_gpa_report[P_PHASE]==GPA_LIVE&&_gz_gpa_report[P_SERIAL]==1&&
    root==_gz_gpa_report[P_ROOT]&&generation==_gz_gpa_report[P_ROOTGEN]&&
    start==_gz_gpa_report[P_START]&&bytes==_gz_gpa_report[P_SPAN])z=GPA_MATCH;
 _gz_gpa_report[P_FLAGS]=gpa_faults;leave(prior);
hl_done:_gz_hl_meta_unlock();
 if(_gz_hg_capture_seal(token)!=HG_OK)return GPA_CHANGED;
 return z;
}
