/* Current save-root binding plus fixed clock-domain access.
 * The birth receipt alone is not authority: the actual wrapped allocator
 * exclusion and live generations for BOTH root and backing are required.
 * No general native/root pointer is returned. The original probe is read-only;
 * owner-bound apply/undo may write only the five enumerated clock fields.
 */
typedef struct {SpoU32 heap,generation,state,kind,start,bytes,parent,parent_generation;
 SpoU32 vtable,direction,born_frame,end_frame,ctor_site,dtor_site,flags,epoch;} SrhHeap;
typedef struct {SpoU32 pointer,generation,state,kind,heap,heap_generation,requested,alignment;
 SpoU32 block,capacity,padding,born_frame,end_frame,birth_site,end_site,release_token;
 SpoU32 flags,heap_epoch,missing,reserved;} SrhAllocation;
typedef struct {SpoU32 magic,bytes,version,busy,flags,serial,release_serial,active_heaps,active_allocations;
 SpoU32 births,allocations,release_begin,release_end,heap_end,dropped,session;} SrhControl;
typedef struct {SpoU32 magic,self,serial,site,live,frame,reserved[2];} SrhTicket;
/* Completed native resize copied and sealed only at genuine root initpost. */
typedef struct {SpoU32 token,operation,state,heap,heap_generation,old_epoch,pointer,request,site,frame;
 SpoU32 old_start,old_bytes,old_head,old_tail,have_old,returned;SrhAllocation old;} SrhMutation;
typedef char SrhMutationSize[sizeof(SrhMutation)==144?1:-1];
extern volatile SrhMutation _gz_hm_records[256];
extern volatile SpoU32 _gz_hm_control[16];
typedef struct {SpoU32 magic,bytes,valid,error,root,root_generation,parent,child,thread,boot_serial,scope_serial;
 SrhAllocation allocation,backing;SrhHeap heap,parent_heap;SrhMutation resize;} SrhBinding;
typedef char SrhBindingSize[sizeof(SrhBinding)==476?1:-1];
extern volatile SrhControl _gz_hl_control;
extern volatile SrhHeap _gz_hl_heaps[4096];
extern volatile SrhAllocation _gz_hl_projected[2];
extern SpoU32 _gz_hl_project_locked(SpoU32,SpoU32,SpoU32);
extern volatile SpoU32 _gz_hg_control[16];
extern SpoU32 _gz_hl_meta_try_lock(void);
extern void _gz_hl_meta_unlock(void);
extern SpoU32 _gz_hg_native_enter(SrhTicket*,SpoU32),_gz_hg_native_exit(SrhTicket*,SpoU32);
extern SpoU32 _gz_hg_platform_thread(void),_gz_hg_capture_begin(SpoU32);
extern SpoU32 _gz_hg_capture_seal(SpoU32),_gz_hg_capture_end(SpoU32);
extern volatile SpoU32 _gz_pbac_frame,_gz_pbac_menu,_gz_pbac_pending,_gz_pbac_loading;
extern volatile SpoU32 _gz_pbac_tools,_gz_pbac_world_resume_pending;
extern SpoU32 _gz_roh_user_guard(void),_gz_osa_pin_id(void),_gz_pbac_idle_for_capture(void);
extern unsigned char _gz_heap_slot[];
SrhBinding _gz_srh_binding;
static SpoU32 _gz_srh_seal,_gz_srh_probe_entered,_gz_srh_probe_frame,_gz_srh_probe_thread;
volatile SpoU32 _gz_srh_request,_gz_srh_ack;
SpoU32 _gz_srh_report[48]={0x53524831u,192};
/* Fixed clock snapshot destination, never a caller-selected native pointer. */
SpoU32 _gz_sck_values[8],_gz_sck_report[16]={0x53434b31u,64};
static SpoU32 _gz_sck_entered,_gz_sck_frame,_gz_sck_thread,_gz_sck_serial,_gz_sck_mode;
static SpoU32 _gz_spk_entered,_gz_spk_frame,_gz_spk_thread,_gz_spk_serial,_gz_spk_mode;
static SpoU32 _gz_rkc_entered,_gz_rkc_frame,_gz_rkc_thread,_gz_rkc_serial,_gz_rkc_mode,_gz_rkc_gate;
static SpoU32 rkc_context_owned(SpoU32);
static SpoU32 _gz_dth_entered,_gz_dth_frame,_gz_dth_thread,_gz_dth_serial,_gz_dth_gate;
static SpoU32 dth_context_owned(SpoU32);
extern SpoU32 _gz_gqk_context_owned(SpoU32),_gz_gqk_access_entered(void);
extern SpoU32 _gz_dck_context_owned(SpoU32),_gz_dck_access_entered(void);
extern SpoU32 _gz_doh_context_owned(SpoU32),_gz_doh_access_entered(void);
extern SpoU32 _gz_nsv_context_owned(SpoU32),_gz_nsv_access_entered(void);
extern SpoU32 _gz_nrt_context_owned(SpoU32),_gz_nrt_access_entered(void);
/* Kept separate so the NPC domain can exclude every OTHER root reader
 * without recursively detecting its own entered flag. */
SpoU32 _gz_srh_base_access_entered(void){return _gz_srh_probe_entered||_gz_spk_entered||_gz_sck_entered||_gz_rkc_entered||_gz_dth_entered;}
SpoU32 _gz_srh_any_access_entered(void){return _gz_srh_base_access_entered()||_gz_nsv_access_entered()||_gz_nrt_access_entered();}
extern SpoU32 _gz_prk_kernel_scope(SpoU32,SpoU32),_gz_prk_kernel_value(SpoU32,SpoU32);
extern SpoU32 _gz_ckb_capture_scope(SpoU32),_gz_ckr_kernel_count(void);
extern unsigned char _gz_r294_native_1046f0b0[];
#define SCK_AIR ((SpoU32)_gz_r294_native_1046f0b0+0x5b4cu)
extern SpoU32 _gz_ckr_kernel_scope(SpoU32,SpoU32),_gz_ckr_kernel_expected(SpoU32,SpoU32),_gz_ckr_kernel_target(SpoU32);

static SpoU32 srh_hash(const void*p,SpoU32 n){const unsigned char*b=p;SpoU32 h=2166136261u,i;
 for(i=0;i<n;i++)h=(h^b[i])*16777619u;return h;
}
static void srh_copy(void*d,const volatile void*s,SpoU32 n){SpoU32 i;unsigned char*to=d;const volatile unsigned char*from=s;
 for(i=0;i<n;i++)to[i]=from[i];
}
/* Fixed module-owned observer metadata only. Contention is a clean refusal;
 * this reader never changes the observer's global fault flags or state rows. */
static __attribute__((noinline,noclone)) SpoU32 srh_lock(void){
 if(_gz_hl_control.magic!=0x484c4231u||_gz_hl_control.bytes!=64||_gz_hl_control.version!=1)return 76;
 if(!_gz_hl_meta_try_lock())return 5;
 __sync_synchronize();return 0;
}
static __attribute__((noinline,noclone)) void srh_unlock(void){_gz_hl_meta_unlock();}
static const volatile SrhHeap*srh_heap(SpoU32 p){SpoU32 i,start=((p>>2)*0x9e3779b1u)>>20;
 for(i=0;i<4096;i++){const volatile SrhHeap*h=&_gz_hl_heaps[(start+i)&4095u];
  if(h->heap==p)return h;if(!h->heap)return 0;
 }return 0;
}
static const volatile SrhAllocation*srh_allocation(SpoU32 heap,SpoU32 p,SpoU32 slot){
 /* srh_lock remains held by every caller through the projection copies. */
 if(slot>=2||_gz_hl_project_locked(heap,p,slot))return 0;
 return _gz_hl_projected+slot;
}
static SpoU32 srh_inside(SpoU32 p,SpoU32 n,SpoU32 base,SpoU32 bytes){
 return n&&p>=base&&bytes<=0xffffffffu-base&&p<=0xffffffffu-n&&p+n<=base+bytes;
}
static SpoU32 srh_resize_matches(const SrhBinding*b,const volatile SrhAllocation*pa,
 const volatile SrhHeap*ph,const volatile SrhMutation*r){const volatile SrhAllocation*old=&r->old;
 /* Completed same-address shrink from the observed original backing only.
  * Pending, moved, grown, reset or recreated blocks cannot use this receipt. */
 return pa->flags==16&&r->token&&(r->operation==3||r->operation==4)&&r->state==2&&
  r->heap==b->parent&&r->heap_generation==ph->generation&&r->old_epoch==ph->epoch&&
  r->pointer==b->child&&r->returned==b->child&&r->request==pa->requested&&
  r->site==pa->birth_site&&r->frame==pa->born_frame&&r->have_old==1&&
  r->old_start==ph->start&&r->old_bytes==ph->bytes&&
  old->pointer==b->child&&old->generation&&old->generation<pa->generation&&
  old->state==1&&old->kind==1&&old->heap==b->parent&&old->heap_generation==ph->generation&&
  old->heap_epoch==ph->epoch&&!old->flags&&!old->end_frame&&!old->end_site&&!old->release_token&&
  !old->reserved&&old->missing==31&&old->capacity>pa->capacity&&
  old->block==pa->block&&old->padding==pa->padding&&old->alignment==pa->alignment&&
  old->requested>=pa->requested&&pa->capacity>=pa->requested&&
  srh_inside(b->root,SRO_CAPACITY,old->pointer,old->capacity);
}
static SpoU32 srh_resize_current(const SrhBinding*b,const volatile SrhAllocation*pa,
 const volatile SrhHeap*ph){SpoU32 i;const unsigned char*p=(const unsigned char*)&b->resize;
 if(!pa->flags){for(i=0;i<sizeof(b->resize);i++)if(p[i])return 0;return 1;}
 return srh_resize_matches(b,pa,ph,&b->resize);
}
static __attribute__((noinline,noclone)) SpoU32 srh_capture_resize(SrhBinding*b){const volatile SrhAllocation*pa;
 const volatile SrhHeap*ph;const volatile SrhMutation*found=0;SpoU32 i;
 /* Called only by fresh entered initpost under the common metadata lock.
  * Runtime leases never renew a receipt from the finite observer ring. */
 pa=srh_allocation(b->parent,b->child,1);ph=srh_heap(b->parent);
 if(!pa||!ph)return 78;
 for(i=0;i<sizeof(b->resize);i++)((unsigned char*)&b->resize)[i]=0;
 if(!pa->flags)return 0;
 if(_gz_hm_control[0]!=0x484d5531u||_gz_hm_control[1]!=64||_gz_hm_control[2]!=1||
  _gz_hm_control[3]||_gz_hm_control[6]||_gz_hm_control[15])return 87;
 for(i=0;i<256;i++)if(srh_resize_matches(b,pa,ph,&_gz_hm_records[i])&&
  _gz_hm_records[i].old.requested==_gz_spo_private.backing_bytes){
  if(found)return 87;found=&_gz_hm_records[i];
 }
 if(!found)return 87;srh_copy(&b->resize,found,sizeof(b->resize));return 0;
}
static __attribute__((noinline,noclone)) SpoU32 srh_tuple(SrhBinding*b){const volatile SrhHeap*h,*ph;const volatile SrhAllocation*a,*pa;
 if(_gz_hl_control.flags||_gz_hl_control.active_heaps>4096||_gz_hl_control.active_allocations>262144)return 77;
 h=srh_heap(b->child);ph=srh_heap(b->parent);a=srh_allocation(b->child,b->root,0);pa=srh_allocation(b->parent,b->child,1);
 if(!h||!ph||!a||!pa)return 78;
 if(h->state!=1||ph->state!=1||h->kind!=2||ph->kind!=1||!h->generation||!ph->generation||
  h->parent!=ph->heap||h->parent_generation!=ph->generation||h->flags||ph->flags)return 79;
 if(a->state!=1||pa->state!=1||a->kind!=2||pa->kind!=1||!a->generation||!pa->generation||
  a->flags||!srh_resize_current(b,pa,ph)||a->end_frame||pa->end_frame||a->release_token||pa->release_token||
  a->heap_generation!=h->generation||pa->heap_generation!=ph->generation||
  a->heap_epoch!=h->epoch||pa->heap_epoch!=ph->epoch||a->missing!=31||pa->missing!=31)return 80;
 if(a->requested<SRO_CAPACITY||a->capacity<SRO_CAPACITY||
  !srh_inside(b->root,SRO_CAPACITY,h->start,h->bytes)||
  !srh_inside(b->child,SPB_CHILD_BYTES,pa->pointer,pa->capacity)||
  !srh_inside(b->root,SRO_CAPACITY,pa->pointer,pa->capacity))return 81;
 srh_copy(&b->allocation,a,sizeof(*a));srh_copy(&b->backing,pa,sizeof(*pa));
 srh_copy(&b->heap,h,sizeof(*h));srh_copy(&b->parent_heap,ph,sizeof(*ph));return 0;
}
static void srh_boot_bind(void){SrhBinding*b=&_gz_srh_binding;SpoU32 z;
 /* Only the genuine, still-entered initpost note can issue initial stamps.
  * Failure is retained; later matching addresses cannot renew this birth. */
 if(_gz_spo_private.entered!=1||_gz_spo_private.phase!=SPO_READY||_gz_spo_private.poison||
  _gz_spo_private.gate||!spo_tuple()||_gz_spo_private.completed_serial!=_gz_spo_private.serial)return;
 if(b->root_generation>=_gz_spo_private.root_generation)return;
 b->valid=0;b->magic=0x53524231u;b->bytes=sizeof(*b);b->error=0;
 b->root=_gz_spo_private.root;b->root_generation=_gz_spo_private.root_generation;
 b->parent=_gz_spo_private.parent;b->child=_gz_spo_private.child;b->boot_serial=_gz_spo_private.serial;b->scope_serial=_gz_spo_private.scope_serial;
 b->thread=_gz_hg_platform_thread();z=b->thread?srh_lock():82;
 if(!z){z=srh_capture_resize(b);if(!z)z=srh_tuple(b);srh_unlock();}
 b->error=z;if(!z)b->valid=1;_gz_srh_seal=srh_hash(b,sizeof(*b));
}
/* Join the existing allocator exclusion BEFORE the original destructor note
 * retires the root. A concurrent lease completes before native destruction;
 * future leases see the retired origin. All original native paths still run. */
SpoU32 _gz_spo_dtor_note(SpoU32 root){SrhTicket t={0};SpoU32 ticket,z;
 ticket=_gz_hg_native_enter(&t,0x2720f9cu);
 z=spo_dtor_unheld(root);_gz_hg_native_exit(&t,ticket);return z;
}
static SpoU32 srh_sealed(void){const SrhBinding*b=&_gz_srh_binding;
 return b->magic==0x53524231u&&b->bytes==sizeof(*b)&&b->valid==1&&!b->error&&b->thread&&
  b->root_generation&&_gz_srh_seal==srh_hash(b,sizeof(*b));
}
static SpoU32 srh_idle(void){return _gz_pbac_menu==1&&!_gz_pbac_pending&&
 !*(volatile unsigned char*)&_gz_pbac_loading&&!_gz_pbac_tools&&!_gz_pbac_world_resume_pending&&
 _gz_pbac_idle_for_capture()&&!_gz_roh_user_guard()&&!_gz_osa_pin_id();}
/* Real fixed-frame callback scope. A nonce alone cannot issue this context.
 * HG independently binds its actual native thread and excludes mutators. */
extern SpoU32 _gz_bsg_context_owned(SpoU32),_gz_wgp_context_owned(SpoU32);
SpoU32 _gz_srh_context_owned(SpoU32 cookie){
 if(_gz_wgp_context_owned(cookie))return 1;
 if(_gz_bsg_context_owned(cookie))return 1;
 if(_gz_nrt_context_owned(cookie))return 1;
 if(_gz_nsv_context_owned(cookie))return 1;
 if(_gz_dck_context_owned(cookie))return 1;
 if(_gz_doh_context_owned(cookie))return 1;
 if(_gz_gqk_context_owned(cookie))return 1;
 if(dth_context_owned(cookie))return 1;
 if(rkc_context_owned(cookie))return 1;
 if(_gz_spk_entered==1&&cookie&&cookie==_gz_spk_serial&&_gz_spk_frame==_gz_pbac_frame&&
  _gz_spk_thread&&_gz_spk_thread==_gz_hg_platform_thread()&&
  _gz_prk_kernel_scope(_gz_spk_mode,_gz_spk_frame)&&srh_sealed())return 1;
 if(_gz_sck_entered==1&&cookie&&cookie==_gz_sck_serial&&_gz_sck_frame==_gz_pbac_frame&&
  _gz_sck_thread&&_gz_sck_thread==_gz_hg_platform_thread()&&
  ((_gz_sck_mode==1&&_gz_ckb_capture_scope(_gz_sck_frame))||
   (_gz_sck_mode>=2&&_gz_sck_mode<=5&&_gz_ckr_kernel_scope(_gz_sck_mode,_gz_sck_frame)))&&srh_sealed())return 1;
 return _gz_srh_probe_entered==1&&
 cookie==_gz_srh_request&&cookie==_gz_srh_ack+1&&_gz_srh_probe_frame==_gz_pbac_frame&&
 _gz_srh_probe_thread&&_gz_srh_probe_thread==_gz_hg_platform_thread()&&srh_sealed()&&srh_idle()?1:0;}
static SpoU32 srh_current(void){SrhBinding now;SpbValue origin;SpoU32 z;
 if(!srh_sealed()||_gz_spo_query(_gz_srh_binding.root,_gz_srh_binding.root_generation,&origin))return 83;
 if(origin.parent!=_gz_srh_binding.parent||origin.child!=_gz_srh_binding.child||
  origin.scope_serial!=_gz_srh_binding.scope_serial)return 83;
 srh_copy(&now,&_gz_srh_binding,sizeof(now));z=srh_lock();if(z)return z;z=srh_tuple(&now);srh_unlock();if(z)return z;
 if(now.allocation.generation!=_gz_srh_binding.allocation.generation||
  now.backing.generation!=_gz_srh_binding.backing.generation||
  now.heap.generation!=_gz_srh_binding.heap.generation||
  now.parent_heap.generation!=_gz_srh_binding.parent_heap.generation)return 84;
 return *(volatile SpoU32*)_gz_heap_slot==now.root?0:85;
}
/* Staged reads and owner-bound clock operations share the fixed-field kernel.
 * World/owner/recovery context is checked by the corresponding participant.
 * Callback-scope queries authenticate private scalars, not source/actor data. */
static SpoU32 sck_scope(SpoU32 mode,SpoU32 frame){return mode==1?_gz_ckb_capture_scope(frame):
 mode>=2&&mode<=5?_gz_ckr_kernel_scope(mode,frame):0;}
static SpoU32 sck_values_safe(const SpoU32*v,SpoU32 n){return (v[0]&0x7fffffffu)<0x7f800000u&&v[1]<=65535&&
 (v[2]&0x7fffffffu)<0x7f800000u&&v[3]<=65535&&(v[4]&0x7fffffffu)<0x7f800000u&&(n==5||(n==8&&v[7]<=255));}
static void sck_read(SpoU32 root){
 _gz_sck_values[0]=*(volatile SpoU32*)(root+0x44);
 _gz_sck_values[1]=*(volatile unsigned short*)(root+0x48);
 _gz_sck_values[2]=*(volatile SpoU32*)SPO_CLOCK_A;
 _gz_sck_values[3]=*(volatile unsigned short*)SPO_CLOCK_SHORT;
 _gz_sck_values[4]=*(volatile SpoU32*)SPO_CLOCK_B;
 _gz_sck_values[5]=*(volatile SpoU32*)SCK_AIR;
 _gz_sck_values[6]=*(volatile SpoU32*)(SCK_AIR+4);
 _gz_sck_values[7]=*(volatile unsigned char*)(SCK_AIR+0x64);
}
static __attribute__((noinline,noclone)) SpoU32 sck_access(SpoU32 mode){SpoU32 frame=_gz_pbac_frame,gate=0,z=0,i,root,n=0,target[8],expected[8],alternate[8];
 for(i=2;i<16;i++)_gz_sck_report[i]=0;
 if(_gz_sck_entered||_gz_spk_entered||_gz_rkc_entered||_gz_dth_entered||_gz_gqk_access_entered()||_gz_doh_access_entered()||_gz_dck_access_entered()||_gz_srh_probe_entered||!sck_scope(mode,frame)||!srh_sealed()||
  _gz_sck_serial==0xffffffffu)return 28;
 _gz_sck_serial++;_gz_sck_frame=frame;_gz_sck_thread=_gz_hg_platform_thread();_gz_sck_mode=mode;_gz_sck_entered=1;
 _gz_sck_report[2]=frame;_gz_sck_report[4]=_gz_sck_serial;_gz_sck_report[13]=mode;
 gate=_gz_hg_capture_begin(_gz_sck_serial);if(!gate){z=86;goto done;}
 z=_gz_hg_capture_seal(gate);if(z)goto done;
 z=srh_current();if(z)goto done;root=_gz_srh_binding.root;
 sck_read(root);
 n=mode==1?8:_gz_ckr_kernel_count();if(n!=5&&n!=8){z=28;goto done;}
 if(mode==3||mode==4){
  for(i=0;i<n;i++){
   target[i]=_gz_ckr_kernel_target(i);expected[i]=_gz_ckr_kernel_expected(i,0);
   alternate[i]=mode==4?_gz_ckr_kernel_expected(i,1):expected[i];
  }
  if(!sck_values_safe(target,n)||!sck_values_safe(expected,n)||!sck_values_safe(alternate,n)){z=77;goto done;}
  for(i=0;i<n;i++)if(_gz_sck_values[i]!=expected[i]&&_gz_sck_values[i]!=alternate[i]){z=77;goto done;}
  z=srh_current();if(z)goto done;z=_gz_hg_capture_seal(gate);if(z)goto done;
  if(!sck_scope(mode,frame)){z=5;goto done;}
  /* Fixed targets only. No native call, callback, source lookup or yield
   * occurs between these five stores under the exact allocation exclusion. */
  *(volatile SpoU32*)(root+0x44)=target[0];_gz_sck_report[14]|=1;
  *(volatile unsigned short*)(root+0x48)=(unsigned short)target[1];_gz_sck_report[14]|=2;
  *(volatile SpoU32*)SPO_CLOCK_A=target[2];_gz_sck_report[14]|=4;
  *(volatile unsigned short*)SPO_CLOCK_SHORT=(unsigned short)target[3];_gz_sck_report[14]|=8;
  *(volatile SpoU32*)SPO_CLOCK_B=target[4];_gz_sck_report[14]|=16;
  /* v1 archives never write the absent air fields. Fixed static native
   * targets join the same entered frame/thread/root/heap transaction. */
  if(n==8){*(volatile SpoU32*)SCK_AIR=target[5];_gz_sck_report[14]|=32;
   *(volatile SpoU32*)(SCK_AIR+4)=target[6];_gz_sck_report[14]|=64;
   *(volatile unsigned char*)(SCK_AIR+0x64)=(unsigned char)target[7];_gz_sck_report[14]|=128;}
  sck_read(root);for(i=0;i<n;i++)if(_gz_sck_values[i]!=target[i]){z=77;goto done;}
 }
 z=srh_current();if(!z)z=_gz_hg_capture_seal(gate);
done:
 if(gate){i=_gz_hg_capture_end(gate);if(!z)z=i;_gz_sck_report[5]=i;}
 if(!z&&!sck_scope(mode,frame))z=5;
 if(!z)for(i=0;i<5;i++)_gz_sck_report[6+i]=_gz_sck_values[i];
 _gz_sck_report[3]=z;_gz_sck_report[11]=_gz_hl_control.flags;
 _gz_sck_report[12]=_gz_hg_control[15];_gz_sck_entered=_gz_sck_mode=0;return z;
}
SpoU32 _gz_sck_capture(void){return sck_access(1);}
SpoU32 _gz_sck_owner_read(void){return sck_access(2);}
SpoU32 _gz_sck_owner_apply(void){return sck_access(3);}
SpoU32 _gz_sck_owner_undo(void){return sck_access(4);}
SpoU32 _gz_sck_owner_check(void){return sck_access(5);}
#include "persistent_access.inc"
#include "death_persistent_access.inc"
#include "render_capture_access.inc"
#include "death_read_access.inc"
void _gz_srh_step(void){SpoU32 req=_gz_srh_request,z=0,gate=0,i,root;
 if(!req||req==_gz_srh_ack)return;
 for(i=2;i<48;i++)_gz_srh_report[i]=0;
 _gz_srh_report[2]=req;_gz_srh_report[4]=_gz_pbac_frame;
 if(req!=_gz_srh_ack+1||_gz_srh_probe_entered||_gz_spk_entered||_gz_sck_entered||_gz_rkc_entered||_gz_dth_entered||_gz_gqk_access_entered()||_gz_doh_access_entered()||_gz_dck_access_entered()||!srh_idle()){z=5;goto done;}
 if(!srh_sealed()){z=_gz_srh_binding.error?_gz_srh_binding.error:83;goto done;}
 _gz_srh_probe_frame=_gz_pbac_frame;_gz_srh_probe_thread=_gz_hg_platform_thread();_gz_srh_probe_entered=1;
 gate=_gz_hg_capture_begin(req);_gz_srh_report[5]=gate;
 if(!gate){z=86;goto release;}
 z=_gz_hg_capture_seal(gate);if(z)goto release;
 z=srh_current();if(z)goto release;
 root=_gz_srh_binding.root;
 _gz_srh_report[6]=root;_gz_srh_report[7]=_gz_srh_binding.root_generation;
 _gz_srh_report[8]=_gz_srh_binding.allocation.generation;
 _gz_srh_report[9]=_gz_srh_binding.backing.generation;
 /* This exact save extent is born/bound, currently live and excluded above.
  * All reads are fixed fields; no caller/source pointer selects a target. */
 _gz_srh_report[10]=*(volatile SpoU32*)(root+0x44);
 _gz_srh_report[11]=*(volatile unsigned short*)(root+0x48);
 _gz_srh_report[12]=*(volatile SpoU32*)SPO_CLOCK_A;
 _gz_srh_report[13]=*(volatile unsigned short*)SPO_CLOCK_SHORT;
 _gz_srh_report[14]=*(volatile SpoU32*)SPO_CLOCK_B;
 z=srh_current();if(!z)z=_gz_hg_capture_seal(gate);
release:
 if(gate){_gz_srh_report[15]=_gz_hg_capture_end(gate);if(!z)z=_gz_srh_report[15];}
 _gz_srh_probe_entered=0;
done:_gz_srh_report[3]=z;_gz_srh_report[16]=_gz_hl_control.flags;
 _gz_srh_report[17]=_gz_srh_binding.error;_gz_srh_report[18]=_gz_hg_control[15];
 __sync_synchronize();_gz_srh_ack=req;
}
