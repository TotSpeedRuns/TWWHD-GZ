#include "archive_layout.h"
/* Original constructor inputs belong to the captured world, not live addresses.
 * Typed read/serialization only; no physical lifetime or replay permission. */
#include "birth_bank.h"
#define R8(p,o) (*(volatile const u8*)((p)+(o)))
#define R32(p,o) (*(volatile const u32*)((p)+(o)))
BbBank _gz_bcb_work,_gz_bcb_staging;
volatile u32 _gz_bcb_ready;
extern VrWorld _gz_vws_source,_gz_vws_stage;
extern volatile u32 _gz_pt_epoch,_gz_roh_scene_header[16];
extern u32 _gz_roh_user_guard(void),_gz_roh_work_replaced(void),_gz_vws_validate(const VrWorld*,u32);
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x10000000u&&p<0xbffe0000u&&n&&n<=0xbffe0000u-p;}
static u32 apart(u32 a,u32 n,u32 b,u32 m){return a+n>=a&&b+m>=b&&(a+n<=b||b+m<=a);}
static void copy(u32 d,u32 s,u32 n){u32 i;for(i=0;i<n;i++)*(volatile u8*)(d+i)=R8(s,i);}
static u32 same(u32 a,u32 b,u32 n){u32 i;for(i=0;i<n;i++)if(R8(a,i)!=R8(b,i))return 0;return 1;}
static u32 hash(const BbBank*b,u32 n){u32 i,h=2166136261u,v;for(i=0;i<n;i++){
 v=R8((u32)b,i);if(i>=12&&i<16)v=(i==15);if(i>=56&&i<60)v=0;h=(h^v)*16777619u;}return h;}
static u32 clocks(const VrWorld*w){return _gz_pt_epoch==w->frame&&_gz_bc_native_frame==w->frame&&
 _gz_roh_scene_header[0]==0x53425031u&&_gz_roh_scene_header[1]==1&&!(_gz_roh_scene_header[4]&0x84u)&&
 _gz_roh_scene_header[2]==w->epoch&&w->epoch&&w->epoch<=0x00ffffffu;}
static u32 process_valid(const PoEntry*e,const VrActor*a,u32 frame){
 return e->object==a->actor&&e->pid==a->pid&&e->process_name==a->profile&&e->state==PO_LIVE&&e->generation&&
 !e->flags&&!e->end_frame&&e->profile<=483&&e->profile_entry&&e->base_bytes&&
 e->allocation_bytes==e->base_bytes+e->other_bytes&&e->allocation_bytes>=e->base_bytes&&
 e->allocation_bytes<=0x400000u&&frame-e->born_frame<0x80000000u;
}
static u32 entry_valid(const BbEntry*e,u32 i,const VrWorld*w){const PoEntry*p=&e->process;const BcRow*r=&e->request;
 if(e->index!=i||e->valid!=1||e->requested_profile!=p->profile||e->generation!=p->generation||
  !process_valid(p,&w->actors[i],w->frame)||!_gz_bc_row_valid(r)||r->pid!=p->pid||r->profile!=p->profile||r->profile_entry!=p->profile_entry||
  p->born_frame-r->frame_after>=0x80000000u||w->frame-r->frame_after>=0x80000000u)return 0;
 if(r->origin==0x025dfab8u&&(r->reserved[0]!=p->generation||r->reserved[1]!=p->object))return 0;
 return 1;
}
static u32 check(const BbBank*b,u32 n,const VrWorld*w,u32 valid){u32 i;
 if(n<BB_HEADER||n>BB_BYTES||!span((u32)b,n)||_gz_vws_validate(w,sizeof(*w)))return 25;
 if(b->magic!=BB_MAGIC||b->bytes!=n||b->version!=1||b->valid!=valid||b->count!=w->count||b->count>BB_CAP||n!=BB_HEADER+b->count*BB_ROW||
  b->frame!=w->frame||b->epoch!=w->epoch||b->session!=w->session||b->stage0!=w->stage0||b->stage1!=w->stage1||b->room!=w->room||b->layer!=w->layer||
  b->world_hash!=w->checksum||b->world_sequence!=w->sequence||b->checksum!=hash(b,n)||b->reserved||b->reserved2||
  b->catalog_version!=BC_VERSION||b->catalog_count>BC_CAP||b->catalog_count<b->count||b->catalog_attempts!=b->catalog_completed||
  b->catalog_completed<b->catalog_count||!b->process_serial)return 25;
 for(i=0;i<b->count;i++)if(!entry_valid(&b->rows[i],i,w))return 25;
 return 0;
}
u32 _gz_bcb_validate(const BbBank*b,u32 n,const VrWorld*w){return check(b,n,w,1);}
static u32 capture_birth(BbBank*b,const VrWorld*w,u32 stage){
 PoHeader ph;BcView view;const BcRow*r;u32 i,j,n,e,matched=0;
 if(_gz_vws_validate(w,sizeof(*w))||!clocks(w)||w->count>BB_CAP)return 25;
 n=BB_HEADER+w->count*BB_ROW;
 if(!apart((u32)b,n,(u32)w,sizeof(*w))||!apart((u32)b,n,(u32)&_gz_po_header,sizeof(PoHeader))||
  !apart((u32)b,n,(u32)_gz_po_rows,sizeof(_gz_po_rows))||!apart((u32)b,n,(u32)&_gz_bc_catalog,sizeof(BcCatalog)))return 25;
 for(i=0;i<w->count;i++)if(!apart((u32)b,n,w->actors[i].actor,0x300))return 25;
 if(stage){e=_gz_roh_work_replaced();if(e)return e;}
 copy((u32)&ph,(u32)&_gz_po_header,sizeof(ph));
 if(ph.magic!=PO_MAGIC||ph.bytes!=sizeof(ph)||ph.version!=1||ph.capacity!=PO_CAP||ph.rows!=(u32)_gz_po_rows||ph.flags||ph.busy||!ph.serial)return 25;
 if(_gz_bc_view_open())return 25;
 copy((u32)&view,(u32)&_gz_bc_view_state,sizeof(view));
 if(view.frame!=w->frame){_gz_bc_view_close();return 25;}
 for(i=0;i<n/4;i++)((volatile u32*)b)[i]=0;
 for(i=0;i<w->count;i++){
  if(_gz_po_lookup(w->actors[i].actor)||!_gz_po_observation.valid||_gz_po_observation.flags||
   _gz_po_observation.session!=ph.session||!process_valid(&_gz_po_observation.entry,&w->actors[i],w->frame))goto bad;
  b->rows[i].index=i;b->rows[i].requested_profile=_gz_po_observation.entry.profile;
  b->rows[i].generation=_gz_po_observation.entry.generation;
  copy((u32)&b->rows[i].process,(u32)&_gz_po_observation.entry,sizeof(PoEntry));
 }
 /* One authenticated catalog traversal for the entire source world. */
 for(j=0;j<view.count;j++){
  r=_gz_bc_view_row(j);if(!r)goto bad;if(!r->valid)continue;
  for(i=0;i<w->count;i++)if(r->pid==b->rows[i].process.pid&&r->profile==b->rows[i].requested_profile){
   if(b->rows[i].valid||r->profile_entry!=b->rows[i].process.profile_entry)goto bad;
   copy((u32)&b->rows[i].request,(u32)r,sizeof(*r));b->rows[i].valid=1;matched++;
  }
 }
 if(matched!=w->count||_gz_bc_view_valid()==0||!same((u32)&ph,(u32)&_gz_po_header,sizeof(ph))||!clocks(w))goto bad;
 b->magic=BB_MAGIC;b->bytes=n;b->version=1;b->count=w->count;b->frame=w->frame;b->epoch=w->epoch;b->session=w->session;
 b->stage0=w->stage0;b->stage1=w->stage1;b->room=w->room;b->layer=w->layer;b->world_hash=w->checksum;b->world_sequence=w->sequence;
 b->catalog_version=BC_VERSION;b->catalog_count=view.count;b->catalog_attempts=view.attempts;b->catalog_completed=view.completed;
 b->catalog_seal=view.header_seal;b->process_serial=ph.serial;b->process_session=ph.session;
 b->checksum=hash(b,n);
 if(check(b,n,w,0))goto bad;
 _gz_bc_view_close();__asm__ volatile("sync":::"memory");b->valid=1;return 0;
bad:_gz_bc_view_close();return 25;
}
u32 _gz_bcb_stage_capture(void){u32 z;
 if(_gz_roh_user_guard())return 5;_gz_bcb_ready=0;
 z=capture_birth(&_gz_bcb_staging,&_gz_vws_stage,1);if(!z)_gz_bcb_ready=1;return z;
}
extern BbBank _gz_lgc_birth;
extern VrWorld _gz_lsp_live;
extern u32 _gz_srd_read_world(u32,u32),_gz_srd_read_bank(u32,u32),_gz_srd_read_layers(u32,u32);
u32 _gz_bcb_read_current(u32 token,u32 frame){u32 z;
 _gz_lgc_birth.valid=0;
 if(!_gz_srd_read_world(token,frame)||!_gz_srd_read_bank(token,frame)||!_gz_srd_read_layers(token,frame)||_gz_lsp_live.frame!=frame)return 42;
 z=capture_birth(&_gz_lgc_birth,&_gz_lsp_live,0);if(z)return z;
 if(!_gz_srd_read_world(token,frame)||!_gz_srd_read_bank(token,frame)||!_gz_srd_read_layers(token,frame)){
  _gz_lgc_birth.valid=0;return 42;
 }
 return 0;
}
u32 _gz_bcb_validate_ready(void){return !_gz_bcb_ready||!clocks(&_gz_vws_stage)?25:check(&_gz_bcb_staging,_gz_bcb_staging.bytes,&_gz_vws_stage,1);}
void _gz_bcb_commit(void){if(_gz_roh_user_guard())return;if(_gz_bcb_ready){copy((u32)&_gz_bcb_work,(u32)&_gz_bcb_staging,_gz_bcb_staging.bytes);_gz_bcb_ready=0;}}
void _gz_bcb_discard(void){if(_gz_roh_user_guard())return;_gz_bcb_ready=0;}
u32 _gz_bcb_validate_current(void){return _gz_bcb_validate(&_gz_bcb_work,_gz_bcb_work.bytes,&_gz_vws_source);}
u32 _gz_bcb_archive_validate(u32 p,u32 n,u32 installed){u32 b,bytes,world;
 if(!gz_arc_shape(p,n)||R32(p,172)!=sizeof(VrWorld))return 42;
 b=p+R32(p,192);bytes=R32(p,196);world=p+R32(p,168);
 if(_gz_bcb_validate((const BbBank*)b,bytes,(const VrWorld*)world))return 42;
 if(installed&&!same((u32)&_gz_bcb_work,b,bytes))return 42;
 return 0;
}
