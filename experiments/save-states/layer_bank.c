#include "archive_layout.h"
/* Snapshot native registered process-layer ancestry beside original requests.
 * Historical layer/owner/APP addresses are values only during validation.
 * This module does not create/delete actors, set currentLayer or restore bytes.
 */
#include "layer_bank.h"
typedef unsigned short u16;
#define R32(p,o) (*(const volatile u32*)((p)+(o)))
#define R16(p,o) (*(const volatile u16*)((p)+(o)))
#define R8(p,o) (*(const volatile u8*)((p)+(o)))
LgBank _gz_lgb_work,_gz_lgb_staging;
volatile u32 _gz_lgb_ready;
extern VrWorld _gz_vws_source,_gz_vws_stage;
extern BbBank _gz_bcb_work,_gz_bcb_staging;
extern volatile u32 _gz_roh_live_frame,_gz_roh_scene_header[16];
extern u32 _gz_roh_user_guard(void),_gz_roh_work_replaced(void),_gz_vws_validate(const VrWorld*,u32);
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static u32 code(u32 p){return !(p&3)&&p>=0x02000020u&&p<0x10000000u;}
static void copy(void*d,const void*s,u32 n){u32 i;for(i=0;i<n/4;i++)((volatile u32*)d)[i]=((const volatile u32*)s)[i];}
static u32 same(const void*a,const void*b,u32 n){u32 i;for(i=0;i<n/4;i++)if(((const volatile u32*)a)[i]!=((const volatile u32*)b)[i])return 0;return 1;}
static u32 hash(const LgBank*b){u32 i,h=2166136261u,v;const u8*p=(const u8*)b;
 for(i=0;i<LG_HEAD+b->count*LG_ROW;i++){v=p[i];if(i>=12&&i<16)v=(i==15);if(i>=60&&i<64)v=0;h=(h^v)*16777619u;}
 p=(const u8*)b->actor_layer;for(i=0;i<b->actor_count*4;i++)h=(h^p[i])*16777619u;return h;
}
static u32 clocks(const VrWorld*w){return w->frame&&w->frame==_gz_roh_live_frame&&w->frame==_gz_bc_native_frame&&w->epoch==_gz_roh_scene_header[2]&&w->epoch&&
 _gz_roh_scene_header[0]==0x53425031u&&_gz_roh_scene_header[1]==1&&!(_gz_roh_scene_header[4]&0x84u);}
static u32 find_layer(const LgBank*b,u32 layer){u32 i;for(i=0;i<b->count;i++)if(b->rows[i].layer==layer)return i;return LG_NO;}
static u32 check(const LgBank*b,u32 n,const VrWorld*w,const BbBank*c,u32 valid){u32 i,j,k;
 if(n!=LG_BYTES||!span((u32)b,n)||_gz_vws_validate(w,sizeof(*w))||_gz_bcb_validate(c,c->bytes,w))return 25;
 if(b->magic!=LG_MAGIC||b->bytes!=n||b->version!=1||b->valid!=valid||!b->count||b->count>LG_CAP||b->actor_count!=w->count||b->actor_count>VR_CAPACITY||
  b->frame!=w->frame||b->epoch!=w->epoch||b->session!=w->session||b->world_hash!=w->checksum||b->world_sequence!=w->sequence||
  b->stage0!=w->stage0||b->stage1!=w->stage1||b->room!=w->room||b->layer!=w->layer||b->root>=b->count||b->ambient>=b->count||
  !b->next_id||b->next_id>=0xfffffffdu||b->flags||!b->node_type||!b->base_type||b->checksum!=hash(b)||
  b->registry_head!=b->rows[0].layer||b->registry_tail!=b->rows[b->count-1].layer)return 25;
 for(i=0;i<b->count;i++){
  const LgRow*r=&b->rows[i];
  if(r->index!=i||!span(r->layer,44)||!span(r->lists,r->list_count*12)||!r->list_count||r->list_count>16)return 25;
  for(j=0;j<5;j++)if(r->reserved[j])return 25;
  for(j=0;j<i;j++)if(r->layer==b->rows[j].layer||(r->owner&&r->owner==b->rows[j].owner)||r->id==b->rows[j].id)return 25;
  if(i==b->root){
   if(r->layer!=0x1048a9bcu||r->id||r->owner||r->parent!=LG_NO||r->lists!=0x1048a9e8u||r->list_count!=10||r->pid!=LG_NO||r->profile!=LG_NO||r->process_name!=LG_NO||r->profile_entry||r->parameters||r->base_methods||r->node_methods||r->generation)return 25;
  }else{
   if(!r->id||r->id>=b->next_id||!span(r->owner,0x1b0)||r->layer!=r->owner+0xc0||r->lists!=r->owner+0xec||r->list_count!=16||
    r->parent>=b->count||r->parent==i||!r->pid||r->pid>=0xfffffffeu||r->profile>483||r->process_name>511||
    !span(r->profile_entry,32)||!span(r->base_methods,16)||!span(r->node_methods,20)||!r->generation)return 25;
  }
  k=i;for(j=0;j<b->count&&k!=b->root;j++){k=b->rows[k].parent;if(k>=b->count)return 25;}
  if(k!=b->root)return 25;
 }
 for(i=0;i<b->actor_count;i++)if(b->actor_layer[i]>=b->count||b->rows[b->actor_layer[i]].layer!=c->rows[i].request.layer)return 25;
 return 0;
}
u32 _gz_lgb_validate(const LgBank*b,u32 n,const VrWorld*w,const BbBank*c){return check(b,n,w,c,1);}
/* Complete current registration is established before any wanted layer body.
 * Do not turn an absent historical address into a numeric RAM probe. */
static u32 registry(LgBank*b){u32 q,prev=0,i=0,head=R32(0x101f3b18u,0),tail=R32(0x101f3b18u,4),count=R32(0x101f3b18u,8);
 if(R32(0x101f3b24u,0)||!count||count>LG_CAP||head!=0x1048a9bcu||!tail)return 26;
 b->registry_head=head;b->registry_tail=tail;b->count=count;b->next_id=R32(0x101f3b28u,0);b->node_type=R32(0x101f3d60u,0);b->base_type=R32(0x101f3934u,0);
 if(!b->next_id||b->next_id>=0xfffffffdu||!b->node_type||!b->base_type)return 26;
 for(q=head;q;q=R32(q,8)){
  if(i>=count||!span(q,12)||R32(q,0)!=prev||R32(q,4)!=0x101f3b18u)return 26;
  b->rows[i].index=i;b->rows[i].layer=q;prev=q;i++;
 }
 if(i!=count||prev!=tail||R32(0x101f3b18u,0)!=head||R32(0x101f3b18u,4)!=tail||R32(0x101f3b18u,8)!=count)return 26;
 return 0;
}
static u32 registry_same(const LgBank*b){u32 q,prev=0,i=0;
 if(R32(0x101f3b24u,0)||R32(0x101f3b18u,0)!=b->registry_head||R32(0x101f3b18u,4)!=b->registry_tail||R32(0x101f3b18u,8)!=b->count||
  R32(0x101f3b28u,0)!=b->next_id||R32(0x101f3d60u,0)!=b->node_type||R32(0x101f3934u,0)!=b->base_type)return 26;
 for(q=b->registry_head;q;q=R32(q,8)){
  if(i>=b->count||q!=b->rows[i].layer||!span(q,12)||R32(q,0)!=prev||R32(q,4)!=0x101f3b18u)return 26;prev=q;i++;
 }
 return i==b->count&&prev==b->registry_tail?0:26;
}
/* Check every native list and process tag before reading the owner header. */
static u32 parent_of(const LgBank*b,u32 owner,u32*out){u32 i,j,p,q,prev,n,count,head,tail,hits=0,budget=4096;
 for(i=0;i<b->count;i++)for(j=0;j<b->rows[i].list_count;j++){
  p=b->rows[i].lists+j*12;head=R32(p,0);tail=R32(p,4);count=R32(p,8);q=head;prev=n=0;
  if(count>4096||(!count&&(head||tail))||(count&&(!head||!tail)))return 27;
  while(q){
   if(!budget--||n++>=count||!span(q,12)||R32(q,0)!=prev||R32(q,4)!=p)return 27;
   if(q==owner+0x18){if(!span(q,20)||R32(q,12)!=owner||R8(q,16)!=1)return 27;*out=i;hits++;}
   prev=q;q=R32(q,8);
  }
  if(n!=count||prev!=tail||R32(p,0)!=head||R32(p,4)!=tail||R32(p,8)!=count)return 27;
 }
 return hits==1?0:27;
}
static u32 capture_layers(LgBank*b,const VrWorld*w,const BbBank*c,u32 stage){PoHeader ph;PoEntry pe;u32 i,j,p,pr,m,z;
 if(!clocks(w)||_gz_vws_validate(w,sizeof(*w))||_gz_bcb_validate(c,c->bytes,w))return 25;
 if(stage&&_gz_roh_work_replaced())return 5;
 for(i=0;i<sizeof(*b)/4;i++)((volatile u32*)b)[i]=0;
 copy(&ph,(const void*)&_gz_po_header,sizeof(ph));if(ph.magic!=PO_MAGIC||ph.bytes!=64||ph.version!=1||ph.capacity!=PO_CAP||ph.rows!=(u32)_gz_po_rows||ph.busy||ph.flags||!ph.serial)return 26;
 z=registry(b);if(z)return z;
 b->root=find_layer(b,0x1048a9bcu);b->ambient=find_layer(b,R32(0x101f3ae8u,0));if(b->root==LG_NO||b->ambient==LG_NO)return 26;
 for(i=0;i<b->count;i++){
  LgRow*r=&b->rows[i];p=r->layer;if(!span(p,44))return 26;
  r->id=R32(p,12);r->owner=R32(p,24);r->lists=R32(p,16);r->list_count=R32(p,20);r->parent=LG_NO;r->pid=r->profile=r->process_name=LG_NO;
  if(i==b->root){if(r->id||r->owner||r->lists!=0x1048a9e8u||r->list_count!=10)return 26;}
  else if(!r->id||r->id>=b->next_id||!span(r->owner,0x1b0)||r->owner+0xc0!=p||r->lists!=r->owner+0xec||r->list_count!=16)return 26;
  if(!span(r->lists,r->list_count*12))return 26;
 }
 for(i=0;i<b->count;i++)if(i!=b->root){
  LgRow*r=&b->rows[i];p=r->owner;z=parent_of(b,p,&r->parent);if(z)return z;
  if(_gz_po_lookup(p)||!_gz_po_observation.valid||_gz_po_observation.flags)return 27;
  copy(&pe,(const void*)&_gz_po_observation.entry,sizeof(pe));
  if(pe.object!=p||pe.state!=PO_LIVE||pe.flags||pe.end_frame||!pe.generation||pe.allocation_bytes<0x1b0||!span(p,pe.allocation_bytes))return 27;
  if(R32(p,0)!=b->base_type||R8(p,12)!=2||R32(p,0xb8)!=b->node_type||R32(p,0x2c)!=b->rows[r->parent].layer||R32(p,0x24)!=p||R8(p,0x28)!=1)return 27;
  r->pid=R32(p,4);r->profile=R16(p,14);r->process_name=R16(p,8);r->profile_entry=R32(p,16);r->parameters=R32(p,0xb0);r->generation=pe.generation;
  if(r->profile>483||pe.pid!=r->pid||pe.profile!=r->profile||pe.process_name!=r->process_name||pe.profile_entry!=r->profile_entry)return 27;
  pr=R32(0x101f3ee0u,r->profile*4);if(pr!=r->profile_entry||!span(pr,32)||R16(pr,8)!=r->process_name)return 27;
  r->base_methods=R32(pr,12);r->node_methods=R32(pr,28);m=r->node_methods;
  if(!span(m,20)||!span(r->base_methods,16)||R32(p,0xbc)!=m||R32(p,0xa8)!=r->base_methods)return 27;
  for(j=0;j<5;j++)if(R32(m,j*4)&&!code(R32(m,j*4)))return 27;
 }
 for(i=0;i<w->count;i++){b->actor_layer[i]=find_layer(b,c->rows[i].request.layer);if(b->actor_layer[i]==LG_NO)return 28;}
 if(registry_same(b)||!clocks(w)||!same(&ph,(const void*)&_gz_po_header,sizeof(ph)))return 26;
 b->magic=LG_MAGIC;b->bytes=sizeof(*b);b->version=1;b->actor_count=w->count;b->frame=w->frame;b->epoch=w->epoch;b->session=w->session;b->world_hash=w->checksum;b->world_sequence=w->sequence;
 b->stage0=w->stage0;b->stage1=w->stage1;b->room=w->room;b->layer=w->layer;b->checksum=hash(b);
 if(check(b,sizeof(*b),w,c,0))return 25;
 __asm__ volatile("sync":::"memory");b->valid=1;return 0;
}
u32 _gz_lgb_stage_capture(void){u32 z;
 if(_gz_roh_user_guard())return 5;_gz_lgb_ready=0;
 z=capture_layers(&_gz_lgb_staging,&_gz_vws_stage,&_gz_bcb_staging,1);if(!z)_gz_lgb_ready=1;return z;
}
extern BbBank _gz_lgc_birth;
extern LgBank _gz_lgc_layers;
extern VrWorld _gz_lsp_live;
extern u32 _gz_srd_read_world(u32,u32),_gz_srd_read_bank(u32,u32),_gz_srd_read_layers(u32,u32);
u32 _gz_lgb_read_current(u32 token,u32 frame){u32 z;
 _gz_lgc_layers.valid=0;
 if(!_gz_srd_read_world(token,frame)||!_gz_srd_read_bank(token,frame)||!_gz_srd_read_layers(token,frame)||_gz_lsp_live.frame!=frame)return 42;
 z=capture_layers(&_gz_lgc_layers,&_gz_lsp_live,&_gz_lgc_birth,0);if(z)return z;
 if(!_gz_srd_read_world(token,frame)||!_gz_srd_read_bank(token,frame)||!_gz_srd_read_layers(token,frame)){
  _gz_lgc_layers.valid=0;return 42;
 }
 return 0;
}
u32 _gz_lgb_current_read_validate(u32 token,u32 frame){
 if(!_gz_srd_read_world(token,frame)||!_gz_srd_read_bank(token,frame)||!_gz_srd_read_layers(token,frame)||_gz_lsp_live.frame!=frame||!clocks(&_gz_lsp_live))return 42;
 if(_gz_lgb_validate(&_gz_lgc_layers,LG_BYTES,&_gz_lsp_live,&_gz_lgc_birth)||registry_same(&_gz_lgc_layers))return 26;
 return 0;
}
u32 _gz_lgb_validate_ready(void){return !_gz_lgb_ready||!clocks(&_gz_vws_stage)?25:check(&_gz_lgb_staging,LG_BYTES,&_gz_vws_stage,&_gz_bcb_staging,1);}
void _gz_lgb_commit(void){if(!_gz_roh_user_guard()&&_gz_lgb_ready){copy(&_gz_lgb_work,&_gz_lgb_staging,LG_BYTES);_gz_lgb_ready=0;}}
void _gz_lgb_discard(void){if(!_gz_roh_user_guard())_gz_lgb_ready=0;}
u32 _gz_lgb_validate_current(void){return _gz_lgb_validate(&_gz_lgb_work,LG_BYTES,&_gz_vws_source,&_gz_bcb_work);}
u32 _gz_lgb_archive_validate(u32 p,u32 n,u32 installed){u32 world,bank,g;
 if(!gz_arc_shape(p,n)||R32(p,172)!=sizeof(VrWorld)||R32(p,204)!=LG_BYTES)return 42;
 world=p+R32(p,168);bank=p+R32(p,192);g=p+R32(p,200);
 if(_gz_lgb_validate((const LgBank*)g,LG_BYTES,(const VrWorld*)world,(const BbBank*)bank))return 42;
 if(installed&&!same(&_gz_lgb_work,(const void*)g,LG_BYTES))return 42;return 0;
}
