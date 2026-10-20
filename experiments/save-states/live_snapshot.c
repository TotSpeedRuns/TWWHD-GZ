/* Read current queued actors during one original nonyielding frame callback.
 * Private process-generation observations are not physical lifetime leases.
 * No saved actor/APP/layer address is followed and no actor is modified. */
#include "lifecycle.h"
extern volatile u32 _gz_roh_live_frame,_gz_roh_scene_header[16];
extern volatile u8 _gz_loading_store;
extern u32 _gz_srd_read_world(u32,u32),_gz_srd_read_bank(u32,u32);
VrWorld _gz_lsp_live;
PoEntry _gz_lsp_processes[VR_CAPACITY];
static PoHeader lsp_header;
static u32 live_span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;}
static u32 lr32(u32 p,u32 o){return *(volatile const u32*)(p+o);}
static u32 lr16(u32 p,u32 o){return *(volatile const u16*)(p+o);}
static u32 lr8(u32 p,u32 o){return *(volatile const u8*)(p+o);}
static void live_copy(u32 d,u32 s,u32 n){u32 i;for(i=0;i<n/4;i++)((volatile u32*)d)[i]=((volatile const u32*)s)[i];}
static u32 live_same(u32 a,u32 b,u32 n){u32 i;for(i=0;i<n/4;i++)if(((volatile const u32*)a)[i]!=((volatile const u32*)b)[i])return 0;return 1;}
static u32 live_capture(const VrWorld*source,u32 frame){
 VrWorld*w=&_gz_lsp_live;VrActor*r;PoEntry*f;u32 q,next,a,i,j,players=0;
 for(i=0;i<sizeof(*w)/4;i++)((volatile u32*)w)[i]=0;
 if(frame!=_gz_roh_live_frame||frame!=_gz_bc_native_frame||_gz_loading_store||lr8(0x10474342u,0)||lr32(0x101f39a4u,0)||lr32(0x101f3a1cu,0))return 10;
 live_copy((u32)&lsp_header,(u32)&_gz_po_header,sizeof(lsp_header));
 if(lsp_header.magic!=PO_MAGIC||lsp_header.bytes!=64||lsp_header.version!=1||lsp_header.capacity!=PO_CAP||
  lsp_header.rows!=(u32)_gz_po_rows||lsp_header.busy||lsp_header.flags||!lsp_header.serial)return 11;
 w->magic=VR_WORLD_MAGIC;w->bytes=sizeof(*w);w->version=1;w->frame=frame;w->epoch=_gz_roh_scene_header[2];
 w->stage0=lr32(0x104741e4u,0);w->stage1=lr32(0x104741e4u,4);w->room=lr8(0x1047e6c8u,0);w->layer=lr8(0x104741efu,0);
 w->link_actor=lr32(0x10474be4u,0);w->link_pid=LP_NO;w->queue_head=lr32(0x101f3328u,0);
 w->quiet=1;w->session=source->session;w->sequence=source->sequence+1;
 if(!w->sequence||!w->epoch||!live_span(w->queue_head,16)||!live_span(w->link_actor,0x300))return 12;
 for(q=w->queue_head;q;q=next){
  if(!live_span(q,16)||++w->node_count>4096)return 13;
  next=lr32(q,8);a=lr32(q,12);if(!a)continue;
  if(w->count==VR_CAPACITY||_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return 14;
  f=&_gz_lsp_processes[w->count];live_copy((u32)f,(u32)&_gz_po_observation.entry,sizeof(*f));
  if(f->object!=a||f->state!=PO_LIVE||f->flags||!f->generation||f->allocation_bytes<0x300||!live_span(a,f->allocation_bytes))return 15;
  r=&w->actors[w->count];r->actor=a;r->pid=lr32(a,4);r->profile=lr16(a,8);r->parameters=lr32(a,0xb0);
  r->set_id=lr16(a,0x2d8);r->parent_pid=lr32(a,0x2e8);r->init_state=lr8(a,12);
  for(i=0;i<5;i++)r->home[i]=lr32(a,0x2ec+i*4);
  if(r->pid!=f->pid||r->profile!=f->process_name||r->profile>=512||r->init_state!=2||lr16(a,14)!=f->profile||lr32(a,16)!=f->profile_entry)return 16;
  for(j=0;j<w->count;j++)if(w->actors[j].actor==a||w->actors[j].pid==r->pid)return 17;
  if(r->profile==168){if(a!=w->link_actor)return 18;w->link_pid=r->pid;players++;}
  w->count++;
 }
 if(players!=1||!live_same((u32)&lsp_header,(u32)&_gz_po_header,sizeof(lsp_header))||frame!=_gz_roh_live_frame||frame!=_gz_bc_native_frame||
  w->epoch!=_gz_roh_scene_header[2]||w->queue_head!=lr32(0x101f3328u,0)||w->link_actor!=lr32(0x10474be4u,0)||
  w->stage0!=lr32(0x104741e4u,0)||w->stage1!=lr32(0x104741e4u,4)||w->room!=lr8(0x1047e6c8u,0)||w->layer!=lr8(0x104741efu,0)||
  _gz_loading_store||lr8(0x10474342u,0)||lr32(0x101f39a4u,0)||lr32(0x101f3a1cu,0))return 19;
 w->checksum=_gz_vr_world_hash(w);__asm__ volatile("sync":::"memory");w->complete=1;return 0;
}
u32 _gz_lsp_build(u32 token,u32 frame){
 const VrWorld*s=(const VrWorld*)_gz_srd_read_world(token,frame);const BbBank*b=(const BbBank*)_gz_srd_read_bank(token,frame);u32 z;
 _gz_lsp_work.valid=0;if(!s||!b)return 20;
 z=live_capture(s,frame);if(z){_gz_lsp_live.complete=0;return z;}
 z=_gz_lsp_plan(s,b,&_gz_lsp_live,_gz_lsp_processes);if(z)return z;
 if(_gz_srd_read_world(token,frame)!=(u32)s||_gz_srd_read_bank(token,frame)!=(u32)b||
  frame!=_gz_roh_live_frame||!live_same((u32)&lsp_header,(u32)&_gz_po_header,sizeof(lsp_header))){_gz_lsp_work.valid=0;return 21;}
 return 0;
}
