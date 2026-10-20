#include "abort_state.h"
u32 _gz_pbac_ready_callback(void*,void*);
#define R8(p,o) (*(const volatile u8*)((u32)(p)+(o)))
#define R16(p,o) (*(const volatile u16*)((u32)(p)+(o)))
#define R32(p,o) (*(const volatile u32*)((u32)(p)+(o)))
#define BAD 0xffffffffu
/* Diagnostic-only admission. Main constructor admission remains untouched. */
u32 _gz_pba_append_cleanup_enabled;
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&p+n>=p&&p+n<=0xbffe0000u;}
static u32 finite(u32 v){return (v&0x7f800000u)!=0x7f800000u;}
static void copy_key(PbaKey* d,const PbaKey* s){u32 i;for(i=0;i<sizeof(*d);i++)((volatile u8*)d)[i]=((const u8*)s)[i];}
static u32 key_safe(const PbaKey*);
static u32 header(const PbaTracker* t){return t->magic==PBA_MAGIC&&t->bytes==sizeof(*t)&&t->phase<=PBA_DONE&&t->cleanup_admitted==1&&t->append_free_calls<=1&&t->cancel_succeeded<=1&&key_safe(&t->key);}
static u32 key_safe(const PbaKey* k){
 u32 i;if(!k->token||k->slot>9||!span(k->archive,16)||k->room>255||k->layer>255||k->pid==BAD)return 0;
 if((k->callback&3)||((k->callback<0x02000020u||k->callback>=0x10000000u)&&k->callback!=(u32)_gz_pbac_ready_callback))return 0;
 for(i=0;i<3;i++)if(!finite(R32(k,40+i*4)))return 0;
 return span(k->family[0],4);
}
/* Every returned address was found in today's native queue. Stored audit
 * pointers are comparison values only and are never dereferenced. */
static u32 find_request(u32 q,u32 pid){
 u32 found=0,n=4096;while(q){u32 r;if(!n--||!span(q,20))return BAD;r=R32((void*)q,12);
  if(!span(r,0x48))return BAD;
  if(R32((void*)r,0x3c)==pid){if(found||!span(r,0x60)||q!=r)return BAD;found=r;}
  q=R32((void*)q,8);
 }return found;
}
static u32 find_actor(u32 q,u32 pid){
 u32 found=0,n=4096;while(q){u32 a;if(!n--||!span(q,16))return BAD;a=R32((void*)q,12);
  if(a){if(!span(a,12))return BAD;if(R32((void*)a,4)==pid){if(found||R16((void*)a,8)!=294)return BAD;found=a;}}
  q=R32((void*)q,8);
 }return found;
}
static u32 request_owned(const PbaTracker* t,u32 r){
 return r==t->request_audit&&R32((void*)r,0x34)==(u32)_gz_pba_native_standard_methods&&R16((void*)r,0x50)==294&&R32((void*)r,0x3c)==t->key.pid&&
  R32((void*)r,0x44)==t->request_layer&&R32((void*)r,0x54)==t->append_audit&&
  R32((void*)r,0x58)==t->key.callback&&R32((void*)r,0x5c)==0&&R8((void*)r,0x10)==1;
}
static u32 append_owned(const PbaKey* k,u32 p){
 u32 i;if(!span(p,0x24)||R32((void*)p,0)!=0x80000001u||R16((void*)p,0x16)!=0xffff||R32((void*)p,0x1c)!=BAD||R8((void*)p,0x21)!=k->home[18])return 0;
 for(i=0;i<12;i++)if(R8((void*)p,4+i)!=k->home[i])return 0;
 for(i=0;i<6;i++)if(R8((void*)p,0x10+i)!=k->home[12+i])return 0;
 return 1;
}
static u32 actor_owned(const PbaTracker* t,u32 a){
 u32 i,m;if(!span(a,0xb50)||R32((void*)a,4)!=t->key.pid||R16((void*)a,8)!=294||R16((void*)a,0x2d8)!=0xffff||R32((void*)a,0xb0)!=0x80000001u||R32((void*)a,0x2e8)!=BAD)return 0;
 for(i=0;i<19;i++)if(R8((void*)a,0x2ec+i)!=t->key.home[i])return 0;
 m=R32((void*)a,0x3bc);if(!span(m,0xb0)||R32((void*)m,0xac)!=t->key.family[0])return 0;
 if(R32((void*)a,0x448)!=t->key.family[1]||R32((void*)a,0x45c)!=t->key.family[2]||R32((void*)a,0x4e8)!=t->key.family[3]||R32((void*)a,0x560)!=t->key.family[4]||R32((void*)a,0x964)!=a+0x8e4)return 0;
 return 1;
}
static u32 born_free(u32 a){
 return R32((void*)a,0x5c8)==0&&R16((void*)a,0x3ac)==0&&R16((void*)a,0x3ae)==0xffff&&R32((void*)a,0x3b0)==0x020c7358u&&
  R8((void*)a,0xa70)==0&&R32((void*)a,0xa78)==1&&!(R32((void*)a,0x2e0)&0x2000)&&!R8((void*)a,0xb05)&&!R8((void*)a,0x8e1);
}
static u32 code_address(u32 p){return !(p&3)&&p>=0x02000020u&&p<0x10000000u;}
static u32 heap_contract(u32* heap,u32* table,u32* method){
 u32 h=_gz_pba_native_heap,v,f;
 if(!span(h,0x10))return 0;
 v=R32((void*)h,0x0c);if(!span(v,0x40))return 0;
 f=R32((void*)v,0x3c);if(!code_address(f))return 0;
 *heap=h;*table=v;*method=f;return 1;
}
static u32 same_heap(const PbaTracker* t){
 u32 h,v,f;return heap_contract(&h,&v,&f)&&h==t->allocation_heap&&v==t->allocation_heap_vtable&&f==t->allocation_free_method;
}
/* These aliases are the live native queue HEADS, not caller snapshots captured
 * before Cancel. No cached request pointer is read after the native call. */
static u32 retire_cancelled_append(PbaTracker* t){
 u32 r,a,d;
 /* Once cancellation has begun, unexpected actor ownership, queue corruption,
  * failed native Cancel or changed heap is sticky. A later empty queue cannot
  * establish that a briefly born process did not already free the append. */
 if(t->last_reason>=28)return t->last_reason;
 if(_gz_pba_append_cleanup_enabled!=1||t->cleanup_admitted!=1){t->last_reason=31;return 31;}
 if(t->cancel_succeeded!=1||t->append_free_calls){t->last_reason=28;return 28;}
 r=find_request(_gz_pba_native_create_queue,t->key.pid);
 a=find_actor(_gz_pba_native_actor_queue,t->key.pid);
 d=find_actor(_gz_pba_native_delete_queue,t->key.pid);
 if(r==BAD||a==BAD||d==BAD||a||d){t->last_reason=28;return 28;}
 if(r){t->last_reason=5;return 5;}
 if(!same_heap(t)){t->last_reason=32;return 32;}
 /* Native cMl::free is the exact normal append-free primitive. Mark before
  * calling it and never touch the allocation's contents after cancellation. */
 t->append_free_calls=1;
 _gz_pba_native_free_append((void*)t->append_audit);
 t->phase=PBA_DONE;t->last_reason=0;return 0;
}
u32 _gz_pba_arm(PbaTracker* t,const PbaKey* k,const PbaQueues* q){
 u32 r,p,h,v,f;if(_gz_pba_append_cleanup_enabled!=1)return 31;
 if(!heap_contract(&h,&v,&f))return 32;
 if(!key_safe(k)||(t->phase!=PBA_IDLE&&(!header(t)||t->phase!=PBA_DONE)))return 28;
 r=find_request(q->create_head,k->pid);if(!r||r==BAD||R32((void*)r,0x34)!=(u32)_gz_pba_native_standard_methods||R32((void*)r,0x40)||R8((void*)r,0x14)||R8((void*)r,0x15)||R16((void*)r,0x50)!=294||R32((void*)r,0x58)!=k->callback||R32((void*)r,0x5c))return 28;
 p=R32((void*)r,0x54);if(!append_owned(k,p))return 28;
 t->magic=PBA_MAGIC;t->bytes=sizeof(*t);t->phase=PBA_OWNED;t->ready=0;copy_key(&t->key,k);
 t->request_audit=r;t->request_layer=R32((void*)r,0x44);t->append_audit=p;
 t->actor_audit=0;t->profile_audit=0;t->methods_audit=0;t->model_audit=0;t->cancel_calls=0;t->delete_calls=0;t->last_reason=0;
 t->allocation_heap=h;t->allocation_heap_vtable=v;t->allocation_free_method=f;
 t->append_free_calls=0;t->cancel_succeeded=0;t->cleanup_admitted=1;
 return request_owned(t,r)?0:28;
}
u32 _gz_pba_abort(PbaTracker* t,u32 token){
 if(!header(t)||!token||token!=t->key.token)return 28;
 if(t->phase==PBA_DONE)return 0;
 if(t->phase!=PBA_OWNED&&t->phase!=PBA_ABORTING&&t->phase!=PBA_CANCELLING&&t->phase!=PBA_RETIRING)return 28;
 if(t->phase==PBA_OWNED)t->phase=PBA_ABORTING;
 return 0;
}
u32 _gz_pba_constructor_ready(PbaTracker* t,void* actor,const PbaQueues* q){
 u32 r,a=(u32)actor;if(!header(t)||t->phase<PBA_OWNED||t->phase>PBA_RETIRING)return 28;
 r=find_request(q->create_head,t->key.pid);
 if(!r||r==BAD||!request_owned(t,r)||R32((void*)r,0x40)!=a||!actor_owned(t,a)||!born_free(a))return 28;
 if(t->ready&&(t->actor_audit!=a||t->profile_audit!=R32(actor,0x10)||t->methods_audit!=R32(actor,0xa8)||t->model_audit!=R32(actor,0x3bc)))return 28;
 t->actor_audit=a;t->profile_audit=R32(actor,0x10);t->methods_audit=R32(actor,0xa8);t->model_audit=R32(actor,0x3bc);t->ready=1;
 return 0;
}
u32 _gz_pba_tick(PbaTracker* t,u32 token,const PbaQueues* q){
 u32 r,a,d,ok;if(!header(t)||token!=t->key.token||!token)return 28;
 if(t->phase==PBA_DONE)return 0;
 if(t->phase>=PBA_ABORTING&&t->last_reason>=28)return t->last_reason;
 if(_gz_pba_append_cleanup_enabled!=1){t->last_reason=31;return 31;}
 if(t->phase==PBA_OWNED)return 5;
 if(t->phase==PBA_CANCELLING)return retire_cancelled_append(t);
 if(t->phase<PBA_ABORTING||t->phase>PBA_RETIRING)return 28;
 r=find_request(q->create_head,t->key.pid);a=find_actor(q->actor_head,t->key.pid);d=find_actor(q->delete_head,t->key.pid);
 if(r==BAD||a==BAD||d==BAD){t->last_reason=28;return 28;}
 if(r){
  if(!request_owned(t,r)){t->last_reason=28;return 28;}
  if(R8((void*)r,0x14)||R8((void*)r,0x15))return 5;
  /* A partial mpRes remains native framework-owned. Only the original
   * unallocated request is eligible for native Cancel plus append retirement. */
  if(R32((void*)r,0x40))return 5;
  if(a||d||t->ready||t->cancel_calls||!append_owned(&t->key,t->append_audit)){t->last_reason=28;return 28;}
  if(!same_heap(t)){t->last_reason=32;return 32;}
  t->phase=PBA_CANCELLING;t->cancel_calls++;
  ok=_gz_pba_native_cancel_request((void*)r);
  if(ok!=1){t->last_reason=28;return 28;}
  t->cancel_succeeded=1;t->last_reason=0;
  return retire_cancelled_append(t);
 }
 if(d){
  if(!t->ready||d!=t->actor_audit||!actor_owned(t,d)){t->last_reason=28;return 28;}
  t->phase=PBA_RETIRING;return 5;
 }
 if(!a){
  /* An uncertified request disappearing without our successful Cancel may
   * have failed before allocation (append still owned) or after allocation
   * (native actor cleanup may already have freed it). Do not guess either. */
  if(!t->ready&&!t->cancel_succeeded){t->last_reason=28;return 28;}
  t->phase=PBA_DONE;t->last_reason=0;return 0;
 }
 if(!t->ready)return 5;
 if(a!=t->actor_audit||!actor_owned(t,a)||R32((void*)a,0x10)!=t->profile_audit||R32((void*)a,0xa8)!=t->methods_audit||R32((void*)a,0x3bc)!=t->model_audit){t->last_reason=28;return 28;}
 /* A replayed/counted bomb needs a distinct post-apply rollback, so this
  * helper cannot decrement Link twice or touch a new scene's Link count. */
 if(!born_free(a)){t->last_reason=30;return 30;}
 if(t->phase==PBA_RETIRING||R8((void*)a,0xc)==3)return 5;
 ok=_gz_pba_native_delete_actor((void*)a);
 if(ok){t->phase=PBA_RETIRING;t->delete_calls++;}
 t->last_reason=ok?0:5;return 5;
}
u32 _gz_pba_commit(PbaTracker* t,u32 token){
 if(!header(t)||token!=t->key.token||!token||t->phase!=PBA_OWNED||!t->ready)return 28;
 t->phase=PBA_DONE;return 0;
}
u32 _gz_pba_should_pause(const PbaTracker* t,void* current_actor){
 u32 a=(u32)current_actor;if(!header(t)||t->phase<PBA_OWNED||t->phase>PBA_RETIRING||!t->ready||a!=t->actor_audit)return 0;
 return actor_owned(t,a);
}
