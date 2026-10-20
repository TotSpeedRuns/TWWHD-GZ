/* Native death-UI cancellation participant. Dedicated ROH_UI10, current
 * allocation generations and the real outer DCK/HG lease are mandatory.
 * No object is created/freed, no saved pointer is accepted. The outer root,
 * controller, demo/event/audio and GameOver process still need cancellation;
 * mode8 therefore forces complete rollback rather than publishing revival.
 */
extern u32 _gz_dck_ui_context(u32,u32),_gz_dck_ui_gate(u32,u32);
extern u32 _gz_roh_direct_ui_common(u32,u32,u32);
extern u32 _gz_roh_direct_recovery_common(u32,u32);
enum {DUC_PREPARED=1,DUC_APPLYING=2,DUC_APPLIED=3,DUC_UNDOING=4,DUC_ROLLED=5};
enum {DUC_CONTEXT=690,DUC_BINDING=691,DUC_LAYOUT=692,DUC_CHANGED=693,DUC_LIMIT=694,DUC_CODE=695,DUC_EFFECT_LIFETIME=696,DUC_PARTICLE_BOUNDARY=697};
enum {DUC_EQUIP=8,DUC_CONFIG=9,DUC_GUARD=10,DUC_NODES=55};
typedef struct {
 u32 role,pointer,span;HlAllocation allocation;HlHeap heap;u8 before[600],after[600];
} DucNode;
typedef struct {
 u32 magic,bytes,valid,phase,token,frame,epoch,stage0,stage1,room,layer,root,nodes,step,mutated,error;
 DucNode node[DUC_NODES];
} DucPlan;
typedef char DucNodeSize[sizeof(DucNode)==1356?1:-1];
typedef char DucPlanSize[sizeof(DucPlan)==74644?1:-1];
static DucPlan _gz_duc_plan;
static u32 _gz_duc_seal,_gz_duc_active,_gz_duc_entered;
u32 _gz_duc_report[32]={0x44554331u,128};
#include "death_ui_cancel_pins.inc"
static u32 duc_bytes(void){return 64+_gz_duc_plan.nodes*sizeof(DucNode);}
static void duc_seal(void){_gz_duc_seal=btr_hash(&_gz_duc_plan,duc_bytes());}
static u32 duc_sealed(void){const DucPlan*p=&_gz_duc_plan;
 return p->magic==0x44554331u&&p->bytes==sizeof(*p)&&p->valid==1&&p->token&&p->nodes>=6&&p->nodes<=DUC_NODES&&
 p->phase>=DUC_PREPARED&&p->phase<=DUC_ROLLED&&_gz_duc_active==p->token&&_gz_duc_seal==btr_hash(p,duc_bytes());}
static u32 duc_code(void){u32 i;for(i=0;i<sizeof(duc_pins)/sizeof(duc_pins[0]);i++)
 if(btr_hash((const void*)duc_pins[i][0],duc_pins[i][1])!=duc_pins[i][2])return DUC_CODE;
 return 0;}
static u32 duc_apart(u32 a,u32 n){return dui_private_apart(a,n)&&
 !(a<(u32)&_gz_duc_plan+sizeof(_gz_duc_plan)&&(u32)&_gz_duc_plan<a+n)&&
 !(a<(u32)_gz_duc_report+sizeof(_gz_duc_report)&&(u32)_gz_duc_report<a+n);}
static u32 duc_gate(u32 token,u32 frame){u32 g;
 if(!_gz_dck_ui_context(token,frame)||frame!=_gz_pbac_frame)return 0;
 g=_gz_dck_ui_gate(token,frame);return g&&!_gz_hg_capture_seal(g)?g:0;}
static DucNode*duc_find(u32 role,u32 pointer){u32 i;DucPlan*p=&_gz_duc_plan;
 for(i=0;i<p->nodes;i++)if(p->node[i].role==role&&(!pointer||p->node[i].pointer==pointer))return p->node+i;
 return 0;}

extern u32 _gz_jel_child_epoch(u32),_gz_jel_edge_generation(u32,u32,u32,u32);
extern u32 _gz_jel_match(u32,u32,u32,u32,u32,u32);
extern u32 _gz_jel_pool_match(u32,u32,u32);
static u32 duc_effect_ready(u32 child,u32 edge,u32 effect){u32 epoch,generation;
 epoch=_gz_jel_child_epoch(child);
 generation=_gz_jel_edge_generation(child,epoch,edge,effect);
 if(!epoch||!generation||!_gz_jel_match(R32(0x1047b2d4u,0),effect,generation,edge==0x58?47:48,9,0))return DUC_EFFECT_LIFETIME;
 if(!_gz_jel_pool_match(R32(0x1047b2d4u,0),effect,generation))return DUC_EFFECT_LIFETIME;
 /* Verified birth/assignment is necessary, but does not stop a live calc or
  * render worker. Keep this branch unpromotable until that participant has a
  * genuine owner-bound lease. This is not a whole-game restore rejection. */
 return DUC_PARTICLE_BOUNDARY;
}
static u32 duc_add(u32 role,u32 a,u32 n,u32 vtable,u32 exact){DucPlan*p=&_gz_duc_plan;DucNode*d;u32 i,z;
 if(!n||n>600||p->nodes>=DUC_NODES||!duc_apart(a,n))return DUC_LIMIT;
 for(i=0;i<p->nodes;i++)if(a<p->node[i].pointer+p->node[i].span&&p->node[i].pointer<a+n){
  if(role==DUI_EFFECT&&p->node[i].role==DUI_EFFECT&&a==p->node[i].pointer&&n==p->node[i].span)return 0;
  return DUC_BINDING;
 }
 d=p->node+p->nodes;d->role=role;d->pointer=a;d->span=n;
 if(role<DUC_EQUIP){z=dui_allocation(role,a,n,&d->allocation,&d->heap);if(z)return DUC_BINDING;
  if(exact&&d->allocation.requested!=exact)return DUC_LAYOUT;}
 btr_copy(d->before,(const void*)a,n);btr_copy(d->after,d->before,n);
 if(vtable&&R32(d->before,4)!=vtable)return DUC_LAYOUT;
 p->nodes++;return 0;
}
static u32 duc_identity(void){DucPlan*p=&_gz_duc_plan;u32 i,z;HlAllocation a;HlHeap h;
 if(!duc_gate(p->token,p->frame)||p->epoch!=_gz_pbac_epoch||p->stage0!=_gz_pbac_stage[0]||p->stage1!=_gz_pbac_stage[1]||
  p->room!=_gz_pbac_room||p->layer!=_gz_pbac_layer||p->root!=R32(0x101f835cu,0))return DUC_CONTEXT;
 for(i=0;i<p->nodes;i++){const DucNode*d=p->node+i;
  if(!duc_apart(d->pointer,d->span))return DUC_BINDING;
  if(d->role>=DUC_EQUIP)continue;
  /* The discovery query bound interior effects to an entire current owner.
   * Under the same sealed allocation lease, revalidate that exact owner
   * instead of scanning262144 allocation rows for every effect at every step. */
  z=dto_allocation(d->allocation.pointer,d->allocation.requested,&a,&h);if(z)return DUC_BINDING;
  if(!btr_equal(&a,&d->allocation,sizeof(a))||!btr_equal(&h,&d->heap,sizeof(h)))return DUC_CHANGED;
 }
 return duc_gate(p->token,p->frame)?0:DUC_CONTEXT;
}
/* Check all untouched bytes too. During returned-failure recovery, only an
 * exact before/expected-after value is accepted at a predicted write byte. */
static u32 duc_image(u32 mode){DucPlan*p=&_gz_duc_plan;u32 i,j,x;
 for(i=0;i<p->nodes;i++){const DucNode*d=p->node+i;
  if(mode==2){if(!btr_equal((const void*)d->pointer,d->before,d->span)&&!btr_equal((const void*)d->pointer,d->after,d->span))return DUC_CHANGED;continue;}
  for(j=0;j<d->span;j++){
  x=R8(d->pointer,j);
  if(mode==0&&x!=d->before[j])return DUC_CHANGED;
  if(mode==1&&x!=d->after[j])return DUC_CHANGED;
 }}return 0;
}
static u32 duc_state(const DucNode*d){u32 a=d->pointer,cur=R32(d->before,0x14),ok=0;
 if(R32(d->before,8)!=0x1010a494u||R32(d->before,12)!=0x1010a454u||R32(d->before,0x10)!=a||
  R32(d->before,0x18)!=a+8||R32(d->before,0x1c)||R32(d->before,0x20)!=a+12||
  (R32(d->before,0x28)&&R32(d->before,0x28)!=a+12))return DUC_LAYOUT;
 if(d->role==DUI_ANIMATION)ok=cur==0x1049434cu||cur==0x1049437cu;
 if(d->role==DUI_CHILD)ok=cur==0x104943e8u||cur==0x10494414u||cur==0x10494440u||cur==0x1049446cu;
 if(d->role==DUI_WINDOW)ok=cur==0x1049dd78u||cur==0x1049dda8u||cur==0x1049dc94u||cur==0x1049dcc4u||cur==0x1049dcf4u||cur==0x1049dd24u;
 return ok?0:DUC_LAYOUT;
}
u32 _gz_duc_service_scope(u32 token,u32 frame){return duc_sealed()&&_gz_dck_ui_context(token,frame)&&
 _gz_duc_plan.token==token&&_gz_duc_plan.frame==frame&&_gz_duc_plan.phase>=DUC_PREPARED&&_gz_duc_plan.phase<=DUC_APPLIED;}
static void duc_report(u32 z){u32 i;const DucPlan*p=&_gz_duc_plan;
 for(i=2;i<32;i++)_gz_duc_report[i]=0;
 _gz_duc_report[2]=z;_gz_duc_report[3]=p->token;_gz_duc_report[4]=p->frame;_gz_duc_report[5]=p->phase;
 _gz_duc_report[6]=p->nodes;_gz_duc_report[7]=p->step;_gz_duc_report[8]=p->mutated;_gz_duc_report[9]=_gz_duc_seal;}
u32 _gz_duc_prepare(u32 token,u32 frame){DucPlan*p=&_gz_duc_plan;u32 root,anim,win,array,count,child,effect,i,z;
 if(_gz_duc_entered||_gz_duc_active||(_gz_duc_seal&&(!duc_sealed()||token<=p->token))||
  !duc_gate(token,frame)||_gz_roh_gl_context(token,frame,1)!=1||_gz_roh_owner[3]!=1||_gz_roh_owner[12]!=1)return DUC_CONTEXT;
 _gz_duc_entered=1;eks_zero(p,sizeof(*p));_gz_duc_seal=0;
 p->magic=0x44554331u;p->bytes=sizeof(*p);p->token=token;p->frame=frame;p->phase=DUC_PREPARED;
 p->epoch=_gz_pbac_epoch;p->stage0=_gz_pbac_stage[0];p->stage1=_gz_pbac_stage[1];p->room=_gz_pbac_room;p->layer=_gz_pbac_layer;
 z=duc_code();if(z)goto done;
 root=p->root=R32(0x101f835cu,0);z=duc_add(DUI_ROOT,root,588,0x1013ef28u,588);if(z)goto done;
 if(R32(root,8)!=root||R32(root,0x24)!=0x1013f6f8u){z=DUC_LAYOUT;goto done;}
 anim=R32(root,0x190);win=R32(root,0x1bc);
 z=duc_add(DUI_ANIMATION,anim,100,0x100f4e30u,100);if(z)goto done;
 z=duc_add(DUI_WINDOW,win,220,0x101046d8u,220);if(z)goto done;
 z=duc_state(duc_find(DUI_ANIMATION,anim));if(z)goto done;z=duc_state(duc_find(DUI_WINDOW,win));if(z)goto done;
 count=R32(anim,0x54);array=R32(anim,0x58);if(count>16||(!count&&array)){z=DUC_LIMIT;goto done;}
 if(count){z=duc_add(DUI_ARRAY,array,count*4,0,0);if(z)goto done;
  for(i=0;i<count;i++){child=R32(array,i*4);if(!child)continue;
   z=duc_add(DUI_CHILD,child,104,0x100f50a0u,104);if(z)goto done;z=duc_state(duc_find(DUI_CHILD,child));if(z)goto done;
   effect=R32(child,0x58);if(effect){z=duc_effect_ready(child,0x58,effect);if(z)goto done;z=duc_add(DUI_EFFECT,effect,600,0,0);if(z)goto done;}
   effect=R32(child,0x5c);if(effect){z=duc_effect_ready(child,0x5c,effect);if(z)goto done;z=duc_add(DUI_EFFECT,effect,600,0,0);if(z)goto done;}
  }
 }
 z=duc_add(DUC_EQUIP,0x10474cc0u,32,0,0);if(z)goto done;
 z=duc_add(DUC_CONFIG,0x1047b03cu,96,0,0);if(z)goto done;
 z=duc_add(DUC_GUARD,0x104753a8u,4,0,0);if(z)goto done;
 if(R32(0x104753a8u,0)!=1){z=DUC_LAYOUT;goto done;}
 z=duc_identity();if(z)goto done;z=duc_image(0);if(z)goto done;
 p->valid=1;_gz_duc_active=token;duc_seal();
done:p->error=z;if(p->valid)duc_seal();_gz_duc_entered=0;duc_report(z);return z;
}
static void duc_word(u8*p,u32 o,u32 x){*(u32*)(p+o)=x;}
static void duc_predict(DucNode*d,u32 state){u32 old=R32(d->after,0x14),a=d->pointer;
 duc_word(d->after,0x14,state);duc_word(d->after,0x1c,0);duc_word(d->after,0x20,a+12);
 duc_word(d->after,0x24,old);duc_word(d->after,0x28,a+12);
 if(d->role==DUI_ANIMATION){d->after[0x60]=0;if(old==0x1049437cu)d->after[0x61]=1;}
 if(d->role==DUI_CHILD){d->after[0x65]=0;if(old==0x10494440u)d->after[0x66]=1;}
 if(d->role==DUI_WINDOW&&old==0x1049dcc4u){DucNode*e=duc_find(DUC_EQUIP,0),*c=duc_find(DUC_CONFIG,0);u32 i;
  if(!d->before[0xcb]){for(i=0;i<3;i++)e->after[0x18+i]=e->before[0x1b+i];c->after[0x5a]=0;}
  if(d->before[0xcc]){c->after[0x40]=d->before[0xc8];c->after[0x5a]=d->before[0xcd];}
 }
}
static u32 duc_before_call(u32 token,u32 frame){u32 z;
 if(!duc_sealed()||!duc_gate(token,frame)||_gz_roh_owner[3]!=2||_gz_roh_owner[12]!=10)return DUC_CONTEXT;
 z=_gz_roh_direct_ui_common(token,frame,1);if(z)return z;
 z=duc_identity();if(z)return z;z=duc_image(1);if(z)return z;return duc_code();
}
static u32 duc_change(DucNode*d,u32 state){DucPlan*p=&_gz_duc_plan;u32 z=duc_before_call(p->token,p->frame);if(z)return z;
 duc_predict(d,state);p->step++;p->mutated=1;duc_seal();
 ((void(*)(u32,u32))0x020063c0u)(d->pointer+0x18,state);
 z=duc_identity();if(z)return z;return duc_image(1);
}
static u32 duc_hide(DucNode*child,u32 edge){DucPlan*p=&_gz_duc_plan;DucNode*d;u32 effect,z=duc_before_call(p->token,p->frame);if(z)return z;
 effect=R32(child->before,edge);if(effect){d=duc_find(DUI_EFFECT,effect);if(!d)return DUC_BINDING;
  duc_word(d->after,0x5c,BAD);duc_word(d->after,0x254,R32(d->after,0x254)|1);}
 p->step++;p->mutated=1;duc_seal();
 ((void(*)(u32))(edge==0x58?0x0266c698u:0x0266c6bcu))(child->pointer);
 z=duc_identity();if(z)return z;return duc_image(1);
}
u32 _gz_duc_apply(u32 token,u32 frame){DucPlan*p=&_gz_duc_plan;u32 i,z;
 if(_gz_duc_entered||!duc_sealed()||p->token!=token||p->frame!=frame||p->phase!=DUC_PREPARED)return DUC_CONTEXT;
 z=duc_before_call(token,frame);if(z)return z;_gz_duc_entered=1;p->phase=DUC_APPLYING;duc_seal();
 z=duc_change(duc_find(DUI_ANIMATION,0),0x1049434cu);if(z)goto done;
 for(i=0;i<p->nodes;i++)if(p->node[i].role==DUI_CHILD){
  z=duc_change(p->node+i,0x1049446cu);if(z)goto done;
  z=duc_hide(p->node+i,0x58);if(z)goto done;z=duc_hide(p->node+i,0x5c);if(z)goto done;
 }
 z=duc_change(duc_find(DUI_WINDOW,0),0x1049dd78u);if(z)goto done;
 z=duc_before_call(token,frame);if(!z)p->phase=DUC_APPLIED;
done:p->error=z;duc_seal();_gz_duc_entered=0;duc_report(z);return z;
}
u32 _gz_duc_rollback(u32 token,u32 frame){DucPlan*p=&_gz_duc_plan;u32 i,j,z;
 if(_gz_duc_entered||!duc_sealed()||p->token!=token||p->frame!=frame||!duc_gate(token,frame)||p->phase==DUC_ROLLED)return DUC_CONTEXT;
 z=_gz_roh_direct_recovery_common(token,frame);if(z)return z;z=duc_identity();if(z)return z;z=duc_image(2);if(z)return z;
 _gz_duc_entered=1;p->phase=DUC_UNDOING;duc_seal();
 /* Exact current-generation typed footprints only, never whole-object or
  * saved-pointer replay. Resource/layout words not touched by the native
  * callbacks remain read-only comparisons. No fallible call in this loop. */
 for(i=0;i<p->nodes;i++){const DucNode*d=p->node+i;for(j=0;j<d->span;j++)
  if(d->before[j]!=d->after[j])*(volatile u8*)(d->pointer+j)=d->before[j];}
 z=duc_image(0);if(!z)z=duc_identity();if(!z)z=_gz_roh_direct_recovery_common(token,frame);
 if(!z)p->phase=DUC_ROLLED;p->error=z;duc_seal();_gz_duc_entered=0;duc_report(z);return z;
}
u32 _gz_duc_prepared_exact(u32 token,u32 frame){u32 z;
 if(!_gz_duc_active)return !_gz_duc_plan.valid;
 if(_gz_duc_entered||!duc_sealed()||_gz_duc_plan.token!=token||_gz_duc_plan.frame!=frame||_gz_duc_plan.phase!=DUC_PREPARED)return 0;
 z=duc_identity();return !z&&!duc_image(0);
}
u32 _gz_duc_closure_key(u32 token,u32 frame){const DucPlan*p=&_gz_duc_plan;u32 key[5];
 if(_gz_duc_entered||!duc_sealed()||p->phase!=DUC_ROLLED||p->token!=token||p->frame!=frame||p->error||
  p->epoch!=_gz_pbac_epoch||p->root!=R32(0x101f835cu,0))return 0;
 key[0]=0x44554343u;key[1]=token;key[2]=frame;key[3]=_gz_duc_seal;key[4]=p->nodes;return btr_hash(key,sizeof(key));
}
void _gz_duc_retire_leaf(void){eks_zero(&_gz_duc_plan,sizeof(_gz_duc_plan));_gz_duc_seal=_gz_duc_active=_gz_duc_entered=0;}
