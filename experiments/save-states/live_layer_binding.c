#include "live_layer_binding.h"
extern volatile u32 _gz_roh_live_frame,_gz_roh_registry_revision,_gz_roh_scene_header[16];
extern volatile u8 _gz_loading_store;
extern u32 _gz_srd_kind(u32,u32),_gz_srd_serial(u32,u32);
extern u32 _gz_srd_read_world(u32,u32),_gz_srd_read_bank(u32,u32),_gz_srd_read_layers(u32,u32);
BbBank _gz_lgc_birth;
LgBank _gz_lgc_layers;
LcView _gz_lgc_view;
static PoHeader lgc_ph;
static u32 lgc_catalog[16];
static u32 lgc_r32(u32 p){return *(volatile const u32*)p;}
static u32 lgc_r8(u32 p){return *(volatile const u8*)p;}
static void lgc_copy(void*d,const void*s,u32 n){u32 i;for(i=0;i<n/4;i++)((volatile u32*)d)[i]=((volatile const u32*)s)[i];}
static u32 lgc_same(const void*a,const void*b,u32 n){u32 i;for(i=0;i<n/4;i++)if(((volatile const u32*)a)[i]!=((volatile const u32*)b)[i])return 0;return 1;}
static u32 lgc_hash(const LcView*v){u32 h=2166136261u,i,j,x;for(i=0;i<32;i++){x=i==3?1:i==31?0:((const u32*)v)[i];for(j=0;j<4;j++){h=(h^(x>>24))*16777619u;x<<=8;}}return h;}
static u32 lgc_stable(u32 token,u32 frame){const LcView*v=&_gz_lgc_view;const VrWorld*w=&_gz_lsp_live;
 return token&&token==v->token&&frame&&frame==v->frame&&frame==_gz_roh_live_frame&&frame==_gz_bc_native_frame&&
  _gz_srd_kind(token,frame)==v->context_kind&&_gz_srd_serial(token,frame)==v->context_serial&&
  _gz_srd_read_world(token,frame)==v->source_world&&_gz_srd_read_bank(token,frame)==v->source_birth&&_gz_srd_read_layers(token,frame)==v->source_layers&&
  v->registry_revision==_gz_roh_registry_revision&&v->current_epoch==_gz_roh_scene_header[2]&&
  !(_gz_roh_scene_header[4]&0x84u)&&!_gz_loading_store&&!lgc_r8(0x10474342u)&&!lgc_r32(0x101f39a4u)&&!lgc_r32(0x101f3a1cu)&&
  lgc_same(&lgc_ph,(const void*)&_gz_po_header,sizeof(lgc_ph))&&lgc_same(lgc_catalog,(const void*)&_gz_bc_catalog,sizeof(lgc_catalog))&&
  w->frame==frame&&w->epoch==v->current_epoch&&w->queue_head==lgc_r32(0x101f3328u)&&w->link_actor==lgc_r32(0x10474be4u)&&
  w->stage0==lgc_r32(0x104741e4u)&&w->stage1==lgc_r32(0x104741e8u)&&w->room==lgc_r8(0x1047e6c8u)&&w->layer==lgc_r8(0x104741efu);
}
u32 _gz_lgc_read(u32 token,u32 frame){const LcView*v=&_gz_lgc_view;
 if(v->magic!=0x4c435631u||v->bytes!=sizeof(*v)||v->version!=2||v->valid!=1||v->result||!v->runtime_validation_required||v->mutation_permission||
  v->reserved||!v->context_kind||v->context_kind>2||!v->context_serial||v->checksum!=lgc_hash(v)||!lgc_stable(token,frame))return 0;
 if(_gz_lgb_current_read_validate(token,frame)||_gz_lgm_work.valid!=1||_gz_lgm_work.checksum!=v->plan_hash||
  _gz_lgm_hash(&_gz_lgm_work)!=v->plan_hash||_gz_lgc_birth.checksum!=v->current_birth_hash||
  _gz_lgc_layers.checksum!=v->current_layers_hash||_gz_lsp_live.checksum!=v->current_world_hash)return 0;
 return (u32)v;
}
u32 _gz_lgc_build(u32 token,u32 frame){LcView*v=&_gz_lgc_view;const VrWorld*sw;const BbBank*sb;const LgBank*sg;u32 i,z;
 for(i=0;i<sizeof(*v)/4;i++)((volatile u32*)v)[i]=0;
 v->magic=0x4c435631u;v->bytes=sizeof(*v);v->version=2;v->token=token;v->frame=frame;v->runtime_validation_required=1;
 v->context_kind=_gz_srd_kind(token,frame);v->context_serial=_gz_srd_serial(token,frame);
 v->source_world=_gz_srd_read_world(token,frame);v->source_birth=_gz_srd_read_bank(token,frame);v->source_layers=_gz_srd_read_layers(token,frame);
 if(!token||!frame||!v->context_kind||!v->context_serial||!v->source_world||!v->source_birth||!v->source_layers){z=42;goto bad;}
 sw=(const VrWorld*)v->source_world;sb=(const BbBank*)v->source_birth;sg=(const LgBank*)v->source_layers;
 lgc_copy(&lgc_ph,(const void*)&_gz_po_header,sizeof(lgc_ph));lgc_copy(lgc_catalog,(const void*)&_gz_bc_catalog,sizeof(lgc_catalog));
 v->registry_revision=_gz_roh_registry_revision;v->process_serial=lgc_ph.serial;v->catalog_attempts=lgc_catalog[8];v->catalog_completed=lgc_catalog[9];
 z=_gz_lsp_build(token,frame);if(z)goto bad;
 v->current_epoch=_gz_lsp_live.epoch;if(!lgc_stable(token,frame)){z=43;goto bad;}
 z=_gz_bcb_read_current(token,frame);if(z)goto bad;
 z=_gz_lgb_read_current(token,frame);if(z)goto bad;
 z=_gz_lgm_plan(sg,sw,sb,&_gz_lgc_layers,&_gz_lsp_live,&_gz_lgc_birth);if(z)goto bad;
 if(!lgc_stable(token,frame)){z=43;goto bad;}
 v->source_world_hash=sw->checksum;v->source_birth_hash=sb->checksum;v->source_layers_hash=sg->checksum;
 v->current_world_hash=_gz_lsp_live.checksum;v->current_birth_hash=_gz_lgc_birth.checksum;v->current_layers_hash=_gz_lgc_layers.checksum;v->plan_hash=_gz_lgm_work.checksum;
 v->source_epoch=sw->epoch;v->required_unbound=_gz_lgm_work.required_unbound;v->required_count=_gz_lgm_work.required_count;v->bound_count=_gz_lgm_work.bound_count;
 v->checksum=lgc_hash(v);__asm__ volatile("sync":::"memory");v->valid=1;
 if(!_gz_lgc_read(token,frame)){z=43;goto bad;}return 0;
bad:v->valid=0;v->result=z;return z;
}
