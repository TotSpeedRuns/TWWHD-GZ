/* Combined normal face replay leaf. This is NOT owner/admission authority.
 * The caller must hold the current registered process/solid-heap and both
 * resident archive lifetimes, as well as the appropriate replay/undo ticket.
 * Source animation pointers are never accepted. All resolved wrappers belong
 * to the CURRENT archive table, certified by the owning caller.
 *
 * Use the exact native Link pattern setter, including its second-object
 * mouth exclusion. Use the low scroll SDK setters to avoid the higher Link
 * setter's RNG call, then restore captured blink/frame scalars explicitly.
 */
#include "face_journal.c"
extern void _gz_face_native_link_pattern(u32,u32,u32);
extern void _gz_face_native_set_scroll(u32,u32);
extern u32 _gz_face_native_bind_scroll(u32,u32);
extern u8 _gz_face_native_pattern_resource_vtable[],_gz_face_native_scroll_resource_vtable[];
extern u8 _gz_face_native_safe_string_vtable[],_gz_face_native_safe_string_terminate[];
extern volatile u32 _gz_face_native_blink;

typedef struct {
 u32 magic,bytes;
 u8 recipes[16];
 u32 frames[18]; /* each: current/start/end/rate bits, canonical mode, zero context audit */
 u32 blink,flags;
} FaceSource;
typedef char FaceSourceSize[sizeof(FaceSource)==104?1:-1];
typedef struct {u32 wrappers[2],flags;u8 recipes[16];u32 blink;} FaceExtra;
typedef char FaceExtraSize[sizeof(FaceExtra)==32?1:-1];
typedef struct {
 u32 magic,bytes,valid,reserved;
 FaceBounds bounds;u32 wrappers[2];FaceSource source;FaceExtra before;
 FacePlan before_plans[3],target_plans[3];FaceUndo undo[3];
} FaceReplayPlan;
typedef char FaceReplayPlanSize[sizeof(FaceReplayPlan)==6528?1:-1];
typedef struct {u32 magic,bytes,valid,phase,plan_hash,error,native_calls,reserved;} FaceReplayRun;
enum {FR_SOURCE=248,FR_BINDING=249,FR_PRIVATE=250,FR_CHANGED=251,FR_SDK=252};
static const u32 fr_offsets[]={0x644,0x69c,0x6f4};
static u32 fr_finite(u32 a){return (a&0x7fffffffu)<0x7f800000u;}
static void fr_copy(void*d,const void*s,u32 n){u32 i;for(i=0;i<n;i++)((volatile u8*)d)[i]=((const volatile u8*)s)[i];}
static u32 fr_recipe(const u8*s,u32*out){
 if(U16(s,4)!=65535||U16(s,6)!=65535)return FR_SOURCE;
 *out=U16(s,2);if(*out==65535)*out=U16(s,0);
 return *out==65535?FR_SOURCE:0;
}
static u32 fr_source(const FaceSource*s){u32 i,j,id;
 if(s->magic!=0x46525331u||s->bytes!=sizeof(*s))return FR_SOURCE;
 if(fr_recipe(s->recipes,&id)||fr_recipe(s->recipes+8,&id))return FR_SOURCE;
 for(i=0;i<3;i++){
  for(j=0;j<4;j++)if(!fr_finite(s->frames[i*6+j]))return FR_SOURCE;
  if(s->frames[i*6+4]>1||s->frames[i*6+5])return FR_SOURCE;
 }
 return 0;
}
/* face_flags is read by the owning caller from the validated body-field
 * record at actor offset6980; it is never read from a saved actor address. */
u32 _gz_face_source_normalize(const u8*record,u32 face_flags,FaceSource*out){FaceSource s;u32 i;
 if(!record||!out||U32(record,4704)!=0x46435232u||U32(record,4708)!=80)return FR_SOURCE;
 s.magic=0x46525331u;s.bytes=sizeof(s);fr_copy(s.recipes,record+4500,16);
 fr_copy(s.frames,record+4712,72);s.blink=U32(record,4564);s.flags=face_flags;
 for(i=0;i<3;i++)if(!face_equal(record+4528+12*i,&s.frames[6*i],12))return FR_SOURCE;
 if(fr_source(&s))return FR_SOURCE;
 fr_copy(out,&s,sizeof(s));return 0;
}
static u32 fr_model_index(const FaceBounds*b,u32 name){u32 dict,count,i,p,j;
 if(!fpc_contract_relative(b->model_file,b->model_bytes,b->model+0x18,24,&dict)||!fpc_contract_dictionary(b->model_file,b->model_bytes,dict,&count))return 0xffffffffu;
 for(i=0;i<count;i++){
  if(!fpc_contract_relative(b->model_file,b->model_bytes,dict+24+i*16+8,1,&p))return 0xffffffffu;
  for(j=0;j<=1024;j++){u8 x=U8(name,j);if(x!=U8(p,j))break;if(!x)return i;}
 }
 return 0xffffffffu;
}
static u32 fr_materials(const FaceBounds*b,u32 resource,u32 pattern){u32 rows,i,n,name,index;
 n=U16(resource,pattern?0x16:0x14);
 if(!fpc_contract_relative(b->animation,b->animation_bytes,resource+0x2c,n*(pattern?28:32),&rows)||!rows)return FR_BINDING;
 for(i=0;i<n;i++){
  if(!fpc_contract_relative(b->animation,b->animation_bytes,rows+i*(pattern?28:32)+(pattern?12:16),1,&name)||!name)return FR_BINDING;
  index=fr_model_index(b,name);if(index>=24)return FR_BINDING;
  /* The exact native second-pattern mapping uses material index+1 for
   * non-mouth rows. Certify that additional slot before the native call. */
  if(pattern&&!(U32(name-4,0)==5&&U32(name,0)==0x6d6f7574u&&U16(name,4)==0x6800u)&&index+1>=24)return FR_BINDING;
 }
 return 0;
}
static u32 fr_wrapper(u32 p,u32 kind,u32*data){
 if(!fpc_contract_whole(p,16)||(p&3)||U32(p,4)!=(u32)(kind?_gz_face_native_pattern_resource_vtable:_gz_face_native_scroll_resource_vtable))return FR_BINDING;
 *data=U32(p,12);return fpc_contract_whole(*data,0x30)?0:FR_BINDING;
}
static void fr_extra_read(u32 a,FaceExtra*e){
 e->wrappers[0]=U32(a,0x5f4);e->wrappers[1]=U32(a,0x5fc);e->flags=U32(a,0x6980);
 fr_copy(e->recipes,(const void*)(a+0x65d0),8);fr_copy(e->recipes+8,(const void*)(a+0x65e0),8);e->blink=_gz_face_native_blink;
}
static void fr_extra_write(u32 a,const FaceExtra*e){
 *(volatile u32*)(a+0x5f4)=e->wrappers[0];*(volatile u32*)(a+0x5fc)=e->wrappers[1];
 *(volatile u32*)(a+0x6980)=e->flags;fr_copy((void*)(a+0x65d0),e->recipes,8);
 fr_copy((void*)(a+0x65e0),e->recipes+8,8);_gz_face_native_blink=e->blink;
}
static u32 fr_private(const FaceBounds*b,const FaceReplayPlan*p,const FaceReplayRun*r){u32 a=(u32)p,n=sizeof(*p),x=(u32)r,m=sizeof(*r);
 if(!fpc_contract_whole(b->actor,b->actor_bytes)||b->actor_bytes<0x8284||b->actor_bytes>0x100000||
 !fpc_contract_whole(b->heap,b->end-b->heap)||b->heap+0x9c>b->head||b->head>b->tail||b->tail>b->end||
 !fpc_contract_whole(b->animation,b->animation_bytes)||!fpc_contract_whole(b->model_file,b->model_bytes))return 0;
 if((a|x)&3||!fpc_contract_whole(a,n)||!fpc_contract_whole(x,m)||!face_disjoint(a,n,x,m))return 0;
 if(!face_disjoint(a,n,b->actor,b->actor_bytes)||!face_disjoint(x,m,b->actor,b->actor_bytes)||
 !face_disjoint(a,n,b->heap,b->end-b->heap)||!face_disjoint(x,m,b->heap,b->end-b->heap)||
 !face_disjoint(a,n,b->animation,b->animation_bytes)||!face_disjoint(x,m,b->animation,b->animation_bytes)||
 !face_disjoint(a,n,b->model_file,b->model_bytes)||!face_disjoint(x,m,b->model_file,b->model_bytes))return 0;
 return 1;
}
static u32 fr_safe_string(void){return U32(_gz_face_native_safe_string_vtable,20)==(u32)_gz_face_native_safe_string_terminate;}
static u32 fr_current(const FaceReplayPlan*p){u32 i,z;FacePlan q;
 if(!fr_safe_string()||U32(p->bounds.actor,0x444)==0||U32(U32(p->bounds.actor,0x444),0)!=p->bounds.model)return FR_BINDING;
 for(i=0;i<3;i++){
  z=_gz_face_contract(&p->bounds,p->before_plans[i].object,p->before_plans[i].resource,i<2,&q);if(z)return z;
  if(!face_equal(&q,&p->before_plans[i],sizeof(q)))return FR_CHANGED;
  z=_gz_face_contract(&p->bounds,p->target_plans[i].object,p->target_plans[i].resource,i<2,&q);if(z)return z;
  if(!face_equal(&q,&p->target_plans[i],sizeof(q)))return FR_CHANGED;
 }
 return 0;
}
static u32 fr_sealed(const FaceReplayPlan*p,const FaceReplayRun*r){return p->magic==0x46525031u&&p->bytes==sizeof(*p)&&p->valid==1&&
 r->magic==0x46525231u&&r->bytes==sizeof(*r)&&r->valid==1&&r->phase>=1&&r->phase<=5&&r->plan_hash==face_hash(p,sizeof(*p));}
static u32 fr_before_matches(const FaceReplayPlan*p,u32 body_applied){u32 i,j,k,t;FaceExtra e;fr_extra_read(p->bounds.actor,&e);
 if(e.flags!=(body_applied?p->source.flags:p->before.flags))return FR_CHANGED;
 e.flags=p->before.flags;if(!face_equal(&e,&p->before,sizeof(e)))return FR_CHANGED;
 for(i=0;i<3;i++){
  const FacePlan*q=&p->before_plans[i];const FaceUndo*u=&p->undo[i];k=0;
  for(j=0;j<q->range_count;j++)for(t=0;t<q->ranges[j].count;t++){
   if(!face_equal((const void*)(q->ranges[j].base+t*q->ranges[j].stride),u->data+k,q->ranges[j].width))return FR_CHANGED;
   k+=q->ranges[j].width;
  }
  if(k!=u->used)return FR_PRIVATE;
 }
 return 0;
}
u32 _gz_face_replay_prepare(const FaceBounds*b,const u32*current_wrappers,const FaceSource*s,FaceReplayPlan*p,FaceReplayRun*r){u32 i,z,data[2],old[2];
 if(!b||!s||!current_wrappers||!p||!r||!fr_private(b,p,r))return FR_PRIVATE;
 if(!face_disjoint((u32)p,sizeof(*p),(u32)b,sizeof(*b))||!face_disjoint((u32)r,sizeof(*r),(u32)b,sizeof(*b))||
 !face_disjoint((u32)p,sizeof(*p),(u32)s,sizeof(*s))||!face_disjoint((u32)r,sizeof(*r),(u32)s,sizeof(*s))||
 !face_disjoint((u32)p,sizeof(*p),(u32)current_wrappers,8)||!face_disjoint((u32)r,sizeof(*r),(u32)current_wrappers,8))return FR_PRIVATE;
 if(fr_source(s))return FR_SOURCE;
 if(!fr_safe_string()||!fpc_contract_whole(U32(b->actor,0x444),4)||U32(U32(b->actor,0x444),0)!=b->model)return FR_BINDING;
 for(i=0;i<2;i++){
  z=fr_wrapper(current_wrappers[i],!i,&data[i]);if(z)return z;
  z=fr_wrapper(U32(b->actor,i?0x5fc:0x5f4),!i,&old[i]);if(z)return z;
 }
 for(i=0;i<sizeof(*p);i++)((volatile u8*)p)[i]=0;
 for(i=0;i<sizeof(*r);i++)((volatile u8*)r)[i]=0;
 p->magic=0x46525031u;p->bytes=sizeof(*p);fr_copy(&p->bounds,b,sizeof(*b));fr_copy(&p->source,s,sizeof(*s));
 p->wrappers[0]=current_wrappers[0];p->wrappers[1]=current_wrappers[1];fr_extra_read(b->actor,&p->before);
 for(i=0;i<3;i++){
  u32 o=b->actor+fr_offsets[i];
  if(U32(o,0x44)!=old[i==2]||U32(o,0x18))return FR_BINDING;
  z=_gz_face_contract(&p->bounds,o,old[i==2],i<2,&p->before_plans[i]);if(z)return z;
  z=_gz_face_contract(&p->bounds,o,data[i==2],i<2,&p->target_plans[i]);if(z)return z;
  z=_gz_face_journal_capture(&p->bounds,&p->before_plans[i],&p->undo[i]);if(z)return z;
 }
 z=fr_materials(b,data[0],1);if(z)return z;z=fr_materials(b,data[1],0);if(z)return z;
 z=fr_current(p);if(z)return z;z=fr_before_matches(p,0);if(z)return z;
 p->valid=1;r->magic=0x46525231u;r->bytes=sizeof(*r);r->valid=1;r->phase=1;r->plan_hash=face_hash(p,sizeof(*p));return 0;
}
static u32 fr_saved_matches(const FaceReplayPlan*p){u32 i,j,a=p->bounds.actor;const FaceSource*s=&p->source;
 if(U32(a,0x5f4)!=p->wrappers[0]||U32(a,0x5fc)!=p->wrappers[1]||U32(a,0x6980)!=s->flags||
 !face_equal((const void*)(a+0x65d0),s->recipes,8)||!face_equal((const void*)(a+0x65e0),s->recipes+8,8)||_gz_face_native_blink!=s->blink)return FR_CHANGED;
 for(i=0;i<3;i++){
  u32 o=a+fr_offsets[i],fn=(u32)(s->frames[6*i+4]?_gz_face_native_frame_loop:_gz_face_native_frame_clamp);
  for(j=0;j<4;j++)if(U32(o,4+4*j)!=s->frames[6*i+j])return FR_CHANGED;
  if(U32(o,0x14)!=fn||U32(o,0x18)||U32(o,0x44)!=p->target_plans[i].resource)return FR_CHANGED;
 }
 return 0;
}
/* body_applied==1 is usable only after the owning caller certifies its
 * already-APPLIED body participant: that participant shares the flag word.
 * Every other face before-image byte must still match exactly. */
u32 _gz_face_replay_apply(const FaceReplayPlan*p,FaceReplayRun*r,u32 body_applied){u32 i,j,z,a;const FaceSource*s;
 if(!p||!r||body_applied>1||!fr_private(&p->bounds,p,r)||!fr_sealed(p,r)||r->phase!=1)return FR_PRIVATE;
 z=fr_current(p);if(z)return z;z=fr_before_matches(p,body_applied);if(z)return z;
 a=p->bounds.actor;s=&p->source;r->phase=2;
 _gz_face_native_link_pattern(a,p->wrappers[0],s->flags>>16);r->native_calls++;
 _gz_face_native_set_scroll(a+0x6f4,p->target_plans[2].resource);r->native_calls++;
 z=_gz_face_native_bind_scroll(a+0x6f4,p->bounds.model);r->native_calls++;
 if(z!=0xffff0000u){r->error=FR_SDK;return FR_SDK;}
 *(volatile u32*)(a+0x5fc)=p->wrappers[1];*(volatile u32*)(a+0x6980)=s->flags;
 fr_copy((void*)(a+0x65d0),s->recipes,8);fr_copy((void*)(a+0x65e0),s->recipes+8,8);
 for(i=0;i<3;i++){
  u32 o=a+fr_offsets[i];for(j=0;j<4;j++)*(volatile u32*)(o+4+4*j)=s->frames[6*i+j];
  *(volatile u32*)(o+0x14)=(u32)(s->frames[6*i+4]?_gz_face_native_frame_loop:_gz_face_native_frame_clamp);
  *(volatile u32*)(o+0x18)=0;
 }
 _gz_face_native_blink=s->blink;
 z=fr_current(p);if(!z)z=fr_saved_matches(p);r->error=z;if(z)return z;r->phase=3;return 0;
}
u32 _gz_face_replay_undo(const FaceReplayPlan*p,FaceReplayRun*r){u32 i,z;
 if(!p||!r||!fr_private(&p->bounds,p,r)||!fr_sealed(p,r)||r->phase==5)return FR_PRIVATE;
 z=fr_current(p);if(z)return z;
 r->phase=4;
 for(i=3;i;i--){z=_gz_face_journal_undo(&p->bounds,&p->before_plans[i-1],&p->undo[i-1]);if(z){r->error=z;return z;}}
 fr_extra_write(p->bounds.actor,&p->before);
 z=fr_current(p);if(!z)z=fr_before_matches(p,0);r->error=z;if(z)return z;r->phase=5;return 0;
}
