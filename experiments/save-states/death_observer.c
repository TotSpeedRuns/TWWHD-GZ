/* Private READ-ONLY transition capture at the real frame/root/HG boundary.
 * Current Link, the current creation queue and process-generation ledger are
 * rediscovered. No source pointer or caller-selected destination is followed.
 * Opaque addresses are audits, NOT future lifecycle/replay capabilities.
 * No health, actor, queue, camera, UI, audio or owner field is written here.
 */
#include "death_observer.h"
extern u32 _gz_dth_capture_access(void),_gz_dth_read_scope(u32),_gz_dth_health(void);
static DtoRecord _gz_dto_scratch;
DtoRecord _gz_dto_records[DTO_CAP];
volatile u32 _gz_dto_request,_gz_dto_ack,_gz_dto_stop;
u32 _gz_dto_report[32]={0x44544f31u,128};
static u32 _gz_dto_active,_gz_dto_entered,_gz_dto_frame,_gz_dto_start,_gz_dto_last,_gz_dto_count;
static u32 dto_quiet(void){return !_gz_pbac_pending&&!*(volatile u8*)&_gz_pbac_loading&&
 !_gz_pbac_tools&&!_gz_pbac_world_resume_pending&&_gz_pbac_menu<=1&&
 !_gz_roh_user_guard()&&!_gz_osa_pin_id()&&_gz_pbac_idle_for_capture();}
u32 _gz_dto_kernel_scope(u32 frame){return _gz_dto_entered==1&&_gz_dto_active&&
 _gz_dto_active==_gz_dto_request&&_gz_dto_request==_gz_dto_ack+1&&
 frame==_gz_dto_frame&&frame==_gz_pbac_frame&&_gz_dto_count<DTO_CAP&&dto_quiet();}
static u32 dto_native(u32 p,u32 n){return span(p,n)&&
 !(p<(u32)&_gz_dto_scratch+sizeof(_gz_dto_scratch)&&(u32)&_gz_dto_scratch<p+n)&&
 !(p<(u32)_gz_dto_records+sizeof(_gz_dto_records)&&(u32)_gz_dto_records<p+n)&&
 !(p<(u32)_gz_dto_report+128&&(u32)_gz_dto_report<p+n);}
static u32 dto_hash(const DtoRecord*p){u32 i,h=2166136261u;const u8*b=(const u8*)p;
 for(i=0;i<sizeof(*p)-4;i++)h=(h^((i>=8&&i<12)?0:b[i]))*16777619u;return h;}
static u32 dto_header(PoHeader*out){const PoHeader*h=&_gz_po_header;
 if(h->magic!=PO_MAGIC||h->bytes!=64||h->version!=1||h->capacity!=PO_CAP||h->rows!=(u32)_gz_po_rows||
  h->busy||h->flags||!h->serial)return DTO_PROCESS;
 btr_copy(out,h,sizeof(*h));return 0;}
static u32 dto_process(u32 a,u32 name,u32 bytes,PoEntry*out){const PoEntry*e;
 if(!dto_native(a,bytes)||_gz_po_lookup(a)||_gz_po_observation.valid!=1||_gz_po_observation.flags)return DTO_PROCESS;
 e=&_gz_po_observation.entry;
 if(e->object!=a||e->state!=PO_LIVE||!e->generation||e->process_name!=name||e->profile>=484||e->flags||e->end_frame||
  e->allocation_bytes<bytes||e->allocation_bytes>0x100000||e->base_bytes+e->other_bytes<e->base_bytes||
  e->base_bytes+e->other_bytes!=e->allocation_bytes||!dto_native(a,e->allocation_bytes))return DTO_PROCESS;
 if(R32(a,4)!=e->pid||R16(a,8)!=name||R16(a,14)!=e->profile||R32(a,16)!=e->profile_entry)return DTO_PROCESS;
 btr_copy(out,e,sizeof(*out));return 0;}
/* Exact current Exp/Solid allocation observation plus the independent HG
 * exclusion. The observation's missing31 remains; it grants no native call.
 */
static u32 dto_allocation(u32 p,u32 n,HlAllocation*a,HlHeap*h){const HlObservation*o=&_gz_hl_observation;
 /* Requests/appends are exact allocation bases. The exact hash lookup also
  * rejects ambiguous parent/child aliases, and avoids an interior-owner
  * census across every allocation during a death frame. */
 if(!dto_native(p,n)||_gz_hl_lookup(p)||o->valid!=1||o->flags)return DTO_HEAP;
 if(o->allocation.pointer!=p||!o->allocation.generation||o->allocation.state!=HL_LIVE||
  o->allocation.flags||o->allocation.end_frame||o->allocation.release_token||o->allocation.requested<n||
  o->allocation.capacity<n||o->allocation.missing!=HL_MISSING_ALL||o->allocation.reserved||
  !o->heap.generation||o->heap.state!=HL_LIVE||(o->heap.flags&~HL_REBORN)||o->heap.end_frame||
  (o->heap.kind!=HL_EXP&&o->heap.kind!=HL_SOLID)||o->allocation.heap!=o->heap.heap||
  o->allocation.heap_generation!=o->heap.generation||o->allocation.heap_epoch!=o->heap.epoch)return DTO_HEAP;
 btr_copy(a,&o->allocation,sizeof(*a));btr_copy(h,&o->heap,sizeof(*h));return 0;}
static u32 dto_request(u32 head,u32 pid,DtoRequest*out){u32 q=head,r,next,n=0,z;HlAllocation a;HlHeap h;
 while(q){
  if(++n>4096)return DTO_REQUEST;
  z=dto_allocation(q,0x48,&a,&h);if(z)return z;
  r=R32(q,12);next=R32(q,8);if(r!=q||next==q||R8(q,0x10)!=1)return DTO_REQUEST;
  if(R32(r,0x3c)==pid){
   if(out->pointer||a.requested!=0x60||a.capacity<0x60||R32(r,0x34)!=(u32)_gz_pba_native_standard_methods||
    R16(r,0x50)!=483||R8(r,0x14)>1||R8(r,0x15)||R32(r,0x58)||R32(r,0x5c))return DTO_REQUEST;
   out->pointer=r;out->append=R32(r,0x54);out->layer=R32(r,0x44);out->phase=R32(r,0x48);
   btr_copy(&out->allocation,&a,sizeof(a));btr_copy(&out->heap,&h,sizeof(h));
   if(!dto_native(out->layer,4))return DTO_REQUEST;
   z=dto_allocation(out->append,28,&out->append_allocation,&out->append_heap);if(z)return z;
   if(out->append_allocation.requested!=28||R32(out->append,0)||R32(out->append,8)||
    R32(out->append,12)||R32(out->append,16)||R32(out->append,24)!=BAD)return DTO_REQUEST;
   btr_copy(out->request_bytes,(const void*)r,96);btr_copy(out->append_bytes,(const void*)out->append,28);
  }
  q=next;
 }return 0;}
/* The GameOver process is not sought in the actor queue. A saved ID is never
 * treated as an address; only a fresh unique LIVE ledger row selects a read.
 */
static u32 dto_gameover(u32 pid,PoEntry*out){u32 i,a=0,z;
 for(i=0;i<PO_CAP;i++){const PoEntry*e=_gz_po_rows+i;
  if(e->object&&e->pid==pid&&e->state==PO_LIVE){if(a)return DTO_PROCESS;a=e->object;}}
 if(!a)return 0;z=dto_process(a,483,0x114,out);if(z)return z;
 return out->pid==pid&&out->profile==483&&out->base_bytes==0x114&&!out->other_bytes?0:DTO_PROCESS;}
#include "death_ui_binding.c"
u32 _gz_dto_read_under_root(u32 frame){DtoRecord*p=&_gz_dto_scratch;PoHeader h;PoEntry last;
 u32 a,id,z,callback,proc,i;
 if(!_gz_dto_kernel_scope(frame)||!_gz_dth_read_scope(frame))return DTO_CONTEXT;
 eks_zero(p,sizeof(*p));z=dto_header(&h);if(z)return z;
 a=_gz_pbac_player;z=dto_process(a,168,0x8284,&p->link);if(z)return z;
 if(R8(a,12)!=2)return DTO_PROCESS;
 proc=R32(a,0x65f0);callback=R32(a,0x65f8);
 if(proc!=178)return 0;
 if(R32(a,0x65f4)!=65535||R32((u32)_gz_lpc_native_procedure_table+178*12,0)!=65535||
  callback!=0x02422108u||R32((u32)_gz_lpc_native_procedure_table+178*12,4)!=callback)return DTO_LAYOUT;
 p->magic=DTO_MAGIC;p->bytes=sizeof(*p);p->frame=frame;p->request_nonce=_gz_dto_active;p->epoch=_gz_pbac_epoch;
 p->stage0=_gz_pbac_stage[0];p->stage1=_gz_pbac_stage[1];p->room=_gz_pbac_room;p->layer=_gz_pbac_layer;
 p->process_serial=h.serial;p->create_head=_gz_pba_native_create_queue;p->delete_head=_gz_pba_native_delete_queue;p->link_audit=a;
 p->values[0]=proc;p->values[1]=65535;p->values[2]=callback;p->values[3]=_gz_dth_health();
 p->values[4]=R32(a,0x69f8);p->values[5]=id=R32(a,0x6a80);p->values[6]=R16(a,0x420);p->values[7]=_gz_pbac_event;
 p->values[8]=R32(a,0x6a70);p->values[9]=R32(0x10470350u,0x5cd8);p->values[10]=R8(0x10470350u,0x5be3);
 p->values[11]=R32(0x101f835cu,0); /* Opaque UI-root audit only. */
 if(p->values[3]>65535||!finite(p->values[4])||p->values[4]>0x3f800000u)return DTO_LAYOUT;
 if(id==BAD)p->kind=DTO_BEFORE_REQUEST;
 else{
  z=dto_request(p->create_head,id,&p->request);if(z)return z;
  z=dto_gameover(id,&p->gameover);if(z)return z;
  if(p->gameover.object){u32 g=p->gameover.object;
   p->process_values[0]=R8(g,12);p->process_values[1]=R16(g,0x10c);p->process_values[2]=R16(g,0x10e);
   p->process_values[3]=R8(g,0x110);p->process_values[4]=R8(g,0x111);
   if(p->process_values[0]>2)return DTO_LAYOUT;
   if(p->request.pointer){
    if(R32(p->request.request_bytes,0x40)!=g)return DTO_REQUEST;p->kind=DTO_CONSTRUCTING;
   }else{
    if(p->process_values[0]!=2)return DTO_PROCESS;
    if(p->process_values[4]>1)return DTO_LAYOUT;
    i=p->process_values[3];if(i!=0&&i!=3&&i!=4&&i!=5&&i!=6)return DTO_LAYOUT;
    p->kind=i>=5?DTO_FINISHING:i>=3?DTO_WINDOW:p->process_values[4]?DTO_FADING:DTO_READY;
   }
  }else if(p->request.pointer){if(R32(p->request.request_bytes,0x40))return DTO_REQUEST;p->kind=DTO_REQUESTED;}
  else return DTO_MISSING;
 }
 /* A genuine frame/root/HG read remains nonyielding. The complete native
  * binding and process registry are checked again before private publication.
  */
 z=dto_process(a,168,0x8284,&last);if(z||!btr_equal(&last,&p->link,sizeof(last)))return DTO_CHANGED;
 if(p->gameover.object){
  z=dto_process(p->gameover.object,483,0x114,&last);if(z||!btr_equal(&last,&p->gameover,sizeof(last)))return DTO_CHANGED;
  if(R8(last.object,12)!=p->process_values[0]||R16(last.object,0x10c)!=p->process_values[1]||
   R16(last.object,0x10e)!=p->process_values[2]||R8(last.object,0x110)!=p->process_values[3]||R8(last.object,0x111)!=p->process_values[4])return DTO_CHANGED;
 }
 if(p->request.pointer&&(!btr_equal((const void*)p->request.pointer,p->request.request_bytes,96)||
  !btr_equal((const void*)p->request.append,p->request.append_bytes,28)))return DTO_CHANGED;
 if(!btr_equal(&h,&_gz_po_header,sizeof(h))||_gz_pbac_player!=a||R32(a,0x65f0)!=proc||R32(a,0x65f8)!=callback||
  R32(a,0x6a80)!=id||R32(a,0x69f8)!=p->values[4]||p->create_head!=_gz_pba_native_create_queue||p->delete_head!=_gz_pba_native_delete_queue||
  p->epoch!=_gz_pbac_epoch||p->stage0!=_gz_pbac_stage[0]||p->stage1!=_gz_pbac_stage[1]||p->room!=_gz_pbac_room||p->layer!=_gz_pbac_layer||
  p->values[7]!=_gz_pbac_event||p->values[3]!=_gz_dth_health()||!_gz_dth_read_scope(frame))return DTO_CHANGED;
 dui_observe(frame);return 0;}
static void dto_finish(u32 error){_gz_dto_report[6]=error;_gz_dto_report[9]=_gz_pbac_frame;_gz_dto_report[4]=0;
 __asm__ volatile("sync":::"memory");_gz_dto_ack=_gz_dto_active;_gz_dto_report[3]=_gz_dto_ack;_gz_dto_active=0;}
void _gz_dto_step(void){u32 req=_gz_dto_request,frame=_gz_pbac_frame,z,i;DtoRecord*d;
 if(_gz_dto_entered||!req||req==_gz_dto_ack)return;
 if(req!=_gz_dto_ack+1){_gz_dto_report[6]=DTO_CONTEXT;return;}
 if(!_gz_dto_active){
  _gz_dto_active=req;_gz_dto_start=frame;_gz_dto_last=frame-1;_gz_dto_count=0;
  for(i=2;i<32;i++)_gz_dto_report[i]=0;
  _gz_dto_report[2]=req;_gz_dto_report[4]=1;_gz_dto_report[8]=frame;
 }
 if(_gz_dto_active!=req)return;
 if(_gz_dto_count>=DTO_CAP){dto_finish(_gz_dto_count==DTO_CAP?0:DTO_CONTEXT);return;}
 if(_gz_dto_stop==req){dto_finish(0);return;}
 if(frame-_gz_dto_start>900){dto_finish(DTO_TIMEOUT);return;}
 if(frame==_gz_dto_last)return;_gz_dto_last=frame;_gz_dto_report[10]=frame;_gz_dto_report[12]++;
 if(!dto_quiet()){_gz_dto_report[11]++;return;}
 _gz_dto_frame=frame;_gz_dto_entered=1;z=_gz_dth_capture_access();_gz_dto_entered=0;
 if(z){_gz_dto_report[6]=z;_gz_dto_report[11]++;return;}
 if(!_gz_dto_scratch.kind)return;
 _gz_dto_scratch.sequence=_gz_dto_count+1;_gz_dto_scratch.checksum=dto_hash(&_gz_dto_scratch);
 d=_gz_dto_records+_gz_dto_count;btr_copy(d,&_gz_dto_scratch,sizeof(*d));__asm__ volatile("sync":::"memory");d->valid=1;
 _gz_dto_count++;_gz_dto_report[5]=_gz_dto_count;_gz_dto_report[7]|=1u<<d->kind;_gz_dto_report[13]=d->kind;
 if(_gz_dto_count==DTO_CAP)dto_finish(0);
}
