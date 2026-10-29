/* Draft native suffix park/unpark participant; include after park_context.inc.
 * No WCT/CGR hook is installed by this file. PREPARE is CLEAN/read-only.
 * The real native roundtrip entry below requires MUTATING WGP service8 and
 * retains HG plus the same jobGate for the ENTIRE reversible operation.
 * Resource destruction and parked-world publication are separate protocols.
 * R2 borrows the actual WHL transaction interval; no independent admission.
 */
#include "generic_partition.h"
#include "birth_bank.h"
#include "park_scalar.h"
#include "archive_layout.h"
extern u32 _gz_ipk_native_pause_enable(u32,u32),_gz_ipk_native_pause_disable(u32,u32);
extern u32 _gz_ipk_native_is_node(u32,u32);
/* Preserve original DATA-base + addend semantics, not a sampled pointer. */
#define IPK_NODE_TYPE R32(GZ_OB_DATA_BEGIN,0x001f3d60u)
extern u32 _gz_ipk_native_actor_cut(u32),_gz_ipk_native_actor_add(u32);
enum {IPK_PLAN_MAGIC=0x49505031u,IPK_PLAN_SOURCE=1820,IPK_PLAN_BIND=1821,
 IPK_PLAN_QUEUE=1822,IPK_PLAN_COHORT=1823,IPK_PLAN_PHASE=1824,
 IPK_PLAN_NATIVE=1825,IPK_PLAN_CHANGED=1826,
 IPK_PREPARED=1,IPK_CALLING=2,IPK_PARKED=3,IPK_RESTORED=4,IPK_FAILED_DIRTY=5};
typedef struct {
 u32 magic,bytes,valid,token,frame,source,used,pin,identity,generation,revision,source_hash,node_type,lease_identity;
 u32 source_count,current_count,extra_count,source_co_count,extra_co_count;
 PoHeader po;CoHeader co;CsHeader cs;
 u32 root[3],prefix_node;u8 prefix_tag[20],mass[416],divide[64];
 PoEntry source_process[IPR_SOURCE_CAP],current_process[IPR_SOURCE_CAP],extra_process[IPR_PROCESS_CAP];
 IprKey source_co[IPR_SOURCE_CAP];IprExtraCo extra_co[IPR_EXTRA_CO_CAP];
 u8 tag[IPR_PROCESS_CAP][20],pause[IPR_PROCESS_CAP];
 IprPartition partition;u32 seal;
} IpkPlan;
typedef struct {u32 magic,token,phase,error,paused,cut,added,unpaused,
 call_kind,call_index,pause_raw,cut_raw,add_raw,disable_raw,end_checked,seal;} IpkRun;
typedef char IpkPlanSize[(sizeof(IpkPlan)==73780)?1:-1];
typedef char IpkRunSize[(sizeof(IpkRun)==64)?1:-1];
static IpkPlan ipk_plan __attribute__((section(".bss.gz_ipk_plan")));
static IpkRun ipk_run __attribute__((section(".bss.gz_ipk_plan")));
static IprScratch ipk_scratch __attribute__((section(".bss.gz_ipk_plan")));
static u32 ipk_entered __attribute__((section(".bss.gz_ipk_plan")));
/* Both pure-kernel callers own this fixed DATA-domain workspace only after
 * their successful ipk_entered CAS. Native stacks are not DATA-domain input.
 * No descriptor/output survives either call; the sealed plan is separate. */
typedef struct {IprInput input;IprPartition current;} IpkPartitionWork;
typedef char IpkPartitionWorkSize[(sizeof(IpkPartitionWork)==112)?1:-1];
static IpkPartitionWork ipk_partition_work __attribute__((section(".bss.gz_ipk_plan")));
static void ipk_run_seal(void){ipk_run.seal=btr_hash(&ipk_run,sizeof(ipk_run)-4);}
static u32 ipk_plan_valid(u32 t){return t&&ipk_plan.magic==IPK_PLAN_MAGIC&&ipk_plan.bytes==sizeof(ipk_plan)&&
 ipk_plan.valid==1&&ipk_plan.token==t&&ipk_plan.seal==btr_hash(&ipk_plan,sizeof(ipk_plan)-4);}
static u32 ipk_headers(void){return _gz_po_header.magic==PO_MAGIC&&_gz_po_header.bytes==64&&_gz_po_header.version==1&&
 _gz_po_header.capacity==PO_CAP&&_gz_po_header.rows==(u32)_gz_po_rows&&_gz_po_header.active<=PO_CAP&&
 _gz_po_header.serial&&_gz_po_header.serial!=BAD&&_gz_co_header.magic==CO_MAGIC&&_gz_co_header.bytes==64&&
 _gz_co_header.version==1&&_gz_co_header.capacity==CO_CAP&&_gz_co_header.rows==(u32)_gz_co_rows&&_gz_co_header.active<=CO_CAP&&
 _gz_co_header.serial&&_gz_co_header.serial!=BAD&&_gz_cs_header.magic==0x43535031u&&_gz_cs_header.bytes==128&&
 _gz_cs_header.version==1&&_gz_cs_header.root==CS_ROOT&&!_gz_cs_header.depth&&!_gz_cs_header.flags&&
 _gz_cs_header.epoch&&_gz_cs_header.serial&&!_gz_po_header.busy&&!_gz_po_header.flags&&!_gz_co_header.busy&&!_gz_co_header.flags&&
 btr_equal(&ipk_plan.po,&_gz_po_header,sizeof(PoHeader))&&btr_equal(&ipk_plan.co,&_gz_co_header,sizeof(CoHeader))&&
 btr_equal(&ipk_plan.cs,&_gz_cs_header,sizeof(CsHeader));}
static u32 ipk_current_process(u32 object,PoEntry*out){
 if(_gz_po_lookup(object)||!_gz_po_observation.valid||_gz_po_observation.flags)return 0;
 btr_copy(out,&_gz_po_observation.entry,sizeof(*out));
 return out->object==object&&out->state==PO_LIVE&&!out->flags&&!out->end_frame&&out->generation&&
 out->allocation_bytes>=0x300&&span(object,out->allocation_bytes)&&R32(object,4)==out->pid&&
 R16(object,8)==out->process_name&&R8(object,12)==2;
}
static u32 ipk_in_extra(u32 p,u32 bytes){u32 i;
 for(i=0;i<ipk_plan.extra_count;i++){const PoEntry*o=ipk_plan.extra_process+i;
  if(p>=o->object&&p-o->object<=o->allocation_bytes&&bytes<=o->allocation_bytes-(p-o->object))return i;}
 return BAD;
}
/* Exactly the existing CGP object's fixed getter/code contract; no virtual
 * function is invoked or caller-selected extent accepted. Inline extras only. */
static u32 ipk_extra_collider(const CoEntry*c,IprExtraCo*out){u32 a=c->object,owner,vt,get,n,status,so;
 owner=ipk_in_extra(a,0x50);if(owner==BAD)return IPK_PLAN_BIND;
 vt=R32(a,0x3c);if((vt&3)||vt<GZ_OB_DATA_BEGIN||vt>GZ_OB_OBJECT_VTABLE_HIGH)return IPK_PLAN_BIND;
 get=R32(vt,0x1c);
 if(get==GZ_OB_OBJECT_GET_GAME&&R32(get,0)==0x4e800020u)n=0xf8;
 else if(get==GZ_OB_OBJECT_GET_BASE&&R32(get,0)==0x38600000u&&R32(get,4)==0x4e800020u)n=0x50;
 else return IPK_PLAN_BIND;
 if(ipk_in_extra(a,n)!=owner||(c->object_vtable&&c->object_vtable!=vt))return IPK_PLAN_BIND;
 btr_copy(&out->collider,c,sizeof(*c));out->owner_index=owner;out->object_bytes=n;
 status=R32(a,0x44);if(status!=c->status)return IPK_PLAN_BIND;
 out->status_owner_index=BAD;out->status_bytes=0;if(!status)return 0;
 so=ipk_in_extra(status,0x1c);if(so==BAD)return IPK_PLAN_BIND;
 vt=R32(status,0x18);if((vt&3)||vt<GZ_OB_DATA_BEGIN||vt>GZ_OB_STATUS_VTABLE_HIGH)return IPK_PLAN_BIND;
 get=R32(vt,0x1c);
 if(get==GZ_OB_STATUS_GET_GAME&&R32(get,0)==0x3863001cu&&R32(get,4)==0x4e800020u)n=0x3c;
 else if(get==GZ_OB_STATUS_GET_BASE&&R32(get,0)==0x38600000u&&R32(get,4)==0x4e800020u)n=0x1c;
 else return IPK_PLAN_BIND;
 if(ipk_in_extra(status,n)!=so)return IPK_PLAN_BIND;
 out->status_owner_index=so;out->status_bytes=n;return 0;
}
static u32 ipk_native_quiet(void){u32 group,i,n,p,base;
 if(!_gz_csb_quiet()||_gz_pba_native_create_queue||_gz_pba_native_delete_queue||_gz_rll_priority_head)return 0;
 if(R32((u32)&_gz_pba_native_create_queue,4)||R32((u32)&_gz_pba_native_create_queue,8)||
  R32((u32)&_gz_pba_native_delete_queue,4)||R32((u32)&_gz_pba_native_delete_queue,8)||
  R32((u32)&_gz_rll_priority_head,4)||R32((u32)&_gz_rll_priority_head,8))return 0;
 for(group=0;group<2;group++){
  n=R32(CS_ROOT+0x2854,group?0xbc:0x40);if(n>(group?2u:5u))return 0;
  base=group?0xc0:0x44;
  for(i=0;i<n;i++){p=R32(CS_ROOT+0x2854,base+24*i);if(!p||ipk_in_extra(p,4)!=BAD)return 0;}
 }
 return 1;
}
static u32 ipk_source_current(u32 t,u32 f,u32 mode){
 if(mode==1){const TxObserveSource*v=tx_observe_source(t,f);
  return v&&v->source==ipk_plan.source&&v->used==ipk_plan.used&&v->pin==ipk_plan.pin&&
   wsp_source.identity==ipk_plan.identity&&wsp_source.generation==ipk_plan.generation&&
   wsp_source.revision==ipk_plan.revision&&wsp_source.source_hash==ipk_plan.source_hash;}
 return ((mode==2&&_gz_wct_forward_scope(t,f,8))||(mode==3&&_gz_wct_recovery_scope(t,f)))&&wct.source==ipk_plan.source&&wct.used==ipk_plan.used&&
  wct.pin==ipk_plan.pin&&wct.identity==ipk_plan.identity&&wct.generation==ipk_plan.generation&&
  wct.revision==ipk_plan.revision&&wct.source_hash==ipk_plan.source_hash;
}
static u32 ipk_queue_prefix(u32 n){u32 h=(u32)&_gz_pba_native_actor_queue,q=R32(h,0),prev=0,i;
 if(R32(h,8)!=n)return 0;
 for(i=0;i<n;i++){
  const PoEntry*want=ipk_plan.current_process+i;PoEntry now;
  if(q!=want->object+0xc8||!ipk_current_process(want->object,&now)||!btr_equal(&now,want,sizeof(now))||
   R32(q,0)!=prev||R32(q,4)!=h||R32(q,12)!=want->object||R8(q,16)!=1)return 0;
  prev=q;q=R32(q,8);
 }
 return !q&&prev==R32(h,4);
}
static u32 ipk_queue(u32 parked){return ipk_queue_prefix(parked?ipk_plan.source_count:ipk_plan.current_count);}
/* CLEAN plan owns its private arrays. No caller descriptor, native pointer,
 * profile mask or supplied result can replace the actual source/current cut. */
u32 _gz_ipk_prepare(u32 t,u32 f){const TxObserveSource*v;const VrWorld*w;const BbBank*b;u32 h,q,prev=0,i,j,source,co,stride,components,co_offset,co_bytes,z=IPK_PLAN_SOURCE,end;
 if(ipk_plan.valid||ipk_run.phase||!_gz_jwf_atomic_cas(&ipk_entered,0,1))return IPK_PLAN_PHASE;
 bcl_zero(&ipk_partition_work,sizeof(ipk_partition_work));
 z=_gz_ipk_admission_begin(t,f,1);if(z)goto done;z=IPK_PLAN_SOURCE;
 v=tx_observe_source(t,f);if(!v)goto release;
 source=v->source;components=gz_arc_shape(source,v->used);
 /* The inherited CGA validator accepts legacy missing-contact directories.
  * This participant needs a real complete contact extent, never empty data
  * inferred from a legacy success. Authenticate slot28 BEFORE following it. */
 if(components<29)goto release;
 co_offset=R32(source,240);co_bytes=R32(source,244);
 if(!ipk_contact_extent(v->used,components,co_offset,co_bytes)||
  _gz_bcb_archive_validate(source,v->used,0)||_gz_cga_archive_validate(source,v->used,0))goto release;
 bcl_zero(&ipk_plan,sizeof(ipk_plan));ipk_plan.magic=IPK_PLAN_MAGIC;ipk_plan.bytes=sizeof(ipk_plan);ipk_plan.token=t;ipk_plan.frame=f;
 ipk_plan.source=source;ipk_plan.used=v->used;ipk_plan.pin=v->pin;ipk_plan.identity=wsp_source.identity;
 ipk_plan.generation=wsp_source.generation;ipk_plan.revision=wsp_source.revision;ipk_plan.source_hash=wsp_source.source_hash;
 ipk_plan.lease_identity=_gz_wct_heap_identity(t,f);
 if(!ipk_plan.lease_identity)goto release;ipk_plan.node_type=IPK_NODE_TYPE;
 btr_copy(&ipk_plan.po,&_gz_po_header,sizeof(PoHeader));btr_copy(&ipk_plan.co,&_gz_co_header,sizeof(CoHeader));btr_copy(&ipk_plan.cs,&_gz_cs_header,sizeof(CsHeader));
 if(!ipk_headers())goto release;
 w=(const VrWorld*)(source+R32(source,168));b=(const BbBank*)(source+R32(source,192));
 if(!w->count||w->count>IPR_SOURCE_CAP||b->count!=w->count)goto release;
 ipk_plan.source_count=w->count;
 for(i=0;i<w->count;i++){
  if(b->rows[i].index!=i||b->rows[i].process.object!=w->actors[i].actor||b->rows[i].process.pid!=w->actors[i].pid)goto release;
  btr_copy(ipk_plan.source_process+i,&b->rows[i].process,sizeof(PoEntry));
 }
 h=(u32)&_gz_pba_native_actor_queue;btr_copy(ipk_plan.root,(const void*)h,12);q=R32(h,0);
 while(q){PoEntry now;
  i=ipk_plan.current_count;if(i==IPR_SOURCE_CAP||_gz_po_owner_of(q,20)||!_gz_po_observation.valid||_gz_po_observation.flags)goto release;
  btr_copy(&now,&_gz_po_observation.entry,sizeof(now));
  if(q!=now.object+0xc8||!ipk_current_process(now.object,&now)||R32(q,0)!=prev||R32(q,4)!=h||R32(q,12)!=now.object||R8(q,16)!=1)goto release;
  btr_copy(ipk_plan.current_process+i,&now,sizeof(now));ipk_plan.current_count++;
  for(j=0;j<w->count;j++)if(ipk_plan.source_process[j].object==now.object)break;
  if(j==w->count){j=ipk_plan.extra_count;if(j==IPR_PROCESS_CAP)goto release;btr_copy(ipk_plan.extra_process+j,&now,sizeof(now));ipk_plan.extra_count++;}
  prev=q;q=R32(q,8);
 }
 if(ipk_plan.current_count!=ipk_plan.root[2]||prev!=ipk_plan.root[1]||
  !ipk_suffix_shape(ipk_plan.source_count,ipk_plan.current_count,ipk_plan.extra_count))goto release;
 for(i=0;i<ipk_plan.extra_count;i++){
  const PoEntry*p=ipk_plan.extra_process+i;
  if(!btr_equal(p,ipk_plan.current_process+ipk_plan.source_count+i,sizeof(*p))||
   IPK_NODE_TYPE!=ipk_plan.node_type||_gz_ipk_native_is_node(ipk_plan.node_type,R32(p->object,0xb8)))goto release;
  btr_copy(ipk_plan.tag[i],(const void*)(p->object+0xc8),20);ipk_plan.pause[i]=R8(p->object,11);
 }
 ipk_plan.prefix_node=ipk_plan.source_process[ipk_plan.source_count-1].object+0xc8;
 btr_copy(ipk_plan.prefix_tag,(const void*)ipk_plan.prefix_node,20);
 co=source+co_offset;ipk_plan.source_co_count=R32(co,48);stride=R32(co,8)==1?396:472;
 if(!ipk_plan.source_co_count||ipk_plan.source_co_count>IPR_SOURCE_CAP)goto release;
 for(i=0;i<ipk_plan.source_co_count;i++){ipk_plan.source_co[i].object=R32(co+128+i*stride,0);ipk_plan.source_co[i].generation=R32(co+128+i*stride,4);}
 for(i=0;i<CO_CAP;i++){const CoEntry*c=_gz_co_rows+i;IprExtraCo*x;
  if(c->state!=CO_LIVE||ipk_in_extra(c->object,4)==BAD)continue;
  j=ipk_plan.extra_co_count;if(j==IPR_EXTRA_CO_CAP)goto release;x=ipk_plan.extra_co+j;
  z=ipk_extra_collider(c,x);if(z)goto release;ipk_plan.extra_co_count++;
 }
 ipk_partition_work.input.source_processes=ipk_plan.source_process;ipk_partition_work.input.source_process_count=ipk_plan.source_count;
 ipk_partition_work.input.current_actor_processes=ipk_plan.current_process;ipk_partition_work.input.current_actor_count=ipk_plan.current_count;
 ipk_partition_work.input.extra_processes=ipk_plan.extra_process;ipk_partition_work.input.extra_process_count=ipk_plan.extra_count;
 ipk_partition_work.input.source_co_keys=ipk_plan.source_co;ipk_partition_work.input.source_co_count=ipk_plan.source_co_count;
 ipk_partition_work.input.current_co_rows=_gz_co_rows;ipk_partition_work.input.current_co_capacity=CO_CAP;ipk_partition_work.input.extra_co=ipk_plan.extra_co;
 ipk_partition_work.input.extra_co_count=ipk_plan.extra_co_count;ipk_partition_work.input.source_frame=w->frame;ipk_partition_work.input.frame=f;
 z=_gz_ipr_partition(&ipk_partition_work.input,&ipk_scratch,&ipk_plan.partition);if(z)goto release;
 z=IPK_PLAN_CHANGED;if(!ipk_native_quiet()||!ipk_headers()||!ipk_source_current(t,f,1)||!ipk_queue(0))goto release;
 btr_copy(ipk_plan.mass,(const void*)(CS_ROOT+0x2854),416);btr_copy(ipk_plan.divide,(const void*)(CS_ROOT+0x2810),64);
 z=_gz_ipk_admission_check();if(z)goto release;
 ipk_plan.valid=1;ipk_plan.seal=btr_hash(&ipk_plan,sizeof(ipk_plan)-4);
 ipk_run.magic=IPK_PLAN_MAGIC;ipk_run.token=t;ipk_run.phase=IPK_PREPARED;
 ipk_run.pause_raw=ipk_run.cut_raw=ipk_run.add_raw=ipk_run.disable_raw=BAD;ipk_run_seal();
 z=_gz_ctp_prepare(t,f);if(z)ipk_plan.valid=0;
release:if(!ipk_plan.valid&&!z)z=IPK_PLAN_SOURCE;end=_gz_ipk_admission_end();if(end){z=end;goto done;}if(z){bcl_zero(&ipk_plan,sizeof(ipk_plan));bcl_zero(&ipk_run,sizeof(ipk_run));}
done:bcl_zero(&ipk_partition_work,sizeof(ipk_partition_work));ipk_entered=0;return z;
}
u32 _gz_ipk_private_apart(u32 p,u32 n){return _gz_lrr_private_apart(p,n)&&epj_apart(p,n,(u32)&ipk_context,sizeof(ipk_context))&&
 epj_apart(p,n,(u32)&ipk_plan,sizeof(ipk_plan))&&epj_apart(p,n,(u32)&ipk_run,sizeof(ipk_run))&&
 epj_apart(p,n,(u32)&ipk_scratch,sizeof(ipk_scratch))&&epj_apart(p,n,(u32)&ipk_entered,sizeof(ipk_entered))&&
 epj_apart(p,n,(u32)&ipk_partition_work,sizeof(ipk_partition_work))&&
 epj_apart(p,n,(u32)_gz_ipk_admission_report,sizeof(_gz_ipk_admission_report));}
#include "native_suffix_roundtrip.inc"
