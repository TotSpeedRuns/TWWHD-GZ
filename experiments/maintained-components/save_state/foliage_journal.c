/* Numeric foliage rewind. Saved packet/actor pointers are comparison audits,
 * never write destinations. Native allocation generations + literal current
 * roots qualify each destination; link/room pointers stay current. */
#include "object_binding.h"
#include "foliage_manager_topology.c"
extern u32 _gz_foliage_capture(void*),_gz_foliage_record_safe(const void*),_gz_foliage_preflight(const void*);
extern u32 _gz_hit_world_forward_scope(u32,u32,u32),_gz_hit_recovery_scope(u32,u32);
enum {FGP_PREPARED=1,FGP_APPLYING=2,FGP_APPLIED=3,FGP_UNDOING=4,FGP_ROLLED=5};
typedef struct {u32 slot,pointer,bytes,vtable,hash;ObBinding allocation;} FgpRoot;
typedef struct {u32 actor,pid,hash;ObBinding allocation;PoEntry process;} FgpPlant;
typedef struct {u32 magic,bytes,valid,token,frame,phase,source,used,pin,row,row_hash,source_frame,before,before_hash,plants;
 u32 relation,source_epoch,current_epoch;FgtReport topology;
 FgpRoot roots[4];FgpPlant actors[64];} FgpPlan;
static FgpPlan fgp;static u32 fgp_seal,fgp_entered;
u32 _gz_fgp_report[64]={0x46475031u,256,1};
static const u32 fgp_slots[4]={GZ_R313_NATIVE_10474B68,GZ_R313_NATIVE_10474B6C,GZ_R313_NATIVE_10474B74,GZ_R313_NATIVE_10474B70};
static const u32 fgp_sizes[4]={0x66408u,0x2de90u,0x1c654u,0x7d3d0u};
static const u32 fgp_rows[4]={32,108128,145552,165504};
static const u32 fgp_counts[4]={1500,64,200,200},fgp_stride[4]={68,168,68,584};
static const u32 fgp_anim[4]={102156,10908,13756,116952},fgp_anim_n[4]={104,72,72,72};
static void fgp_seal_now(void){fgp_seal=btr_hash(&fgp,sizeof(fgp));}
static u32 fgp_empty(void){return !fgp.valid&&!fgp.phase&&!fgp_seal&&!fgp_entered&&!jp_lease.valid&&!jp_lease.seal;}
static u32 fgp_sealed(u32 t){return t&&fgp.magic==0x46475050u&&fgp.bytes==sizeof(fgp)&&fgp.valid==1&&fgp.token==t&&
 fgp.phase>=1&&fgp.phase<=5&&fgp.plants<=64&&(fgp.relation==1||fgp.relation==2)&&
 fgp.current_epoch==_gz_pbac_epoch&&fgp.topology.magic==0x46475431u&&fgp.topology.bytes==316&&
 (fgp.topology.status==0||fgp.topology.status==FGT_ORDER_REPLAY)&&fgp_seal==btr_hash(&fgp,sizeof(fgp))&&
 jp_view(t)==fgp.before&&fgp.before&&btr_hash((const void*)fgp.before,JP_FOLIAGE_BYTES)==fgp.before_hash;}
static u32 fgp_receipt(u32 t){return fgp_sealed(t)&&!fgp_entered?fgp_seal:0;}
static u32 fgp_forward_scope(u32 t,u32 f){
 if(tx_core_has(t))return tx_core_forward(t,f,8);
 if(!fgp_sealed(t)||_gz_roh_owner[12]!=8||!_gz_bmh_scope(t,f)||
  _gz_roh_replay_common(t,f,1)||_gz_boh_service_validate(t)||!_gz_hbp_service_read(t,f))return 0;
 return fgp.source==_gz_hbp_recipe.source&&fgp.used==_gz_hbp_recipe.used&&fgp.pin==_gz_hbp_recipe.pin;
}
static u32 fgp_recovery_scope(u32 t,u32 f){return tx_core_has(t)?tx_core_recovery(t,f):
 _gz_boh_recovery_authority(t,_gz_boh_journal.actor,_gz_boh_plan.link,f);}
static u32 fgp_bind(u32 before_source){u32 i,a,z;PoEntry p;
 if(!fgp_sealed(fgp.token))return 1100;
 for(i=0;i<4;i++){
  const FgpRoot*r=fgp.roots+i;a=R32(r->slot,0);if(a!=r->pointer)return 1101;if(!a)continue;
  z=before_source&&fgp.relation==1?_gz_ob_validate_before(&r->allocation,fgp.source_frame):_gz_ob_validate(&r->allocation);
  if(z||R32(a,12)!=r->vtable)return 1102;
 }
 for(i=0;i<fgp.plants;i++){
  const FgpPlant*r=fgp.actors+i;
  if(_gz_po_lookup(r->actor)||!_gz_po_observation.valid)return 1103;
  btr_copy(&p,&_gz_po_observation.entry,sizeof(p));
  if(!btr_equal(&p,&r->process,sizeof(p))||p.state!=PO_LIVE||p.pid!=r->pid||R16(r->actor,8)!=274||R8(r->actor,12)!=2||
   (before_source?_gz_ob_validate_before(&r->allocation,fgp.source_frame):_gz_ob_validate(&r->allocation)))return 1103;
 }
 return 0;
}
/* Same stage/room/layer source and an already authenticated current Link
 * participant qualify a scene-role relation. This is not a saved manager
 * pointer lease: every current root is independently allocation-bound below.
 * Separate plant actors still require their original observed lifetimes. */
static u32 fgp_source_context(u32 t,u32 f,u32 s,u32 n,u32 pin,u32*source_epoch){const VrWorld*w;u32 o;
 if(R32(s,172)!=sizeof(VrWorld))return 0;o=R32(s,168);
 if(o<216||(o&3)||o>n||sizeof(VrWorld)>n-o)return 0;w=(const VrWorld*)(s+o);
 if(_gz_vws_validate(w,sizeof(*w))||!lpc_sealed()||_gz_lpc_run.phase!=LPC_PREPARED||
  _gz_lpc_plan.token!=t||_gz_lpc_plan.frame!=f||_gz_lpc_plan.source!=s||_gz_lpc_plan.used!=n||_gz_lpc_plan.pin!=pin||
  _gz_lpc_plan.actor!=_gz_roh_owner[29]||_gz_lpc_plan.pid!=_gz_roh_owner[30]||
  R32(_gz_lpc_plan.saved,4)!=w->link_pid||R32(_gz_lpc_plan.saved,8)!=w->link_actor||
  w->stage0!=_gz_roh_owner[41]||w->stage1!=_gz_roh_owner[42]||w->room!=_gz_roh_owner[43]||w->layer!=_gz_roh_owner[44]||
  _gz_roh_owner[45]!=_gz_pbac_epoch||!w->epoch||w->epoch>_gz_pbac_epoch)return 0;
 *source_epoch=w->epoch;
 return w->epoch==_gz_pbac_epoch&&w->link_pid==_gz_lpc_plan.pid&&w->link_actor==_gz_lpc_plan.actor?1:w->epoch<_gz_pbac_epoch?2:0;
}
static u32 fgp_plants_match(u32 row){u32 i,p=row+163456,a;
 for(i=0;i<fgp.plants;i++,p+=32){a=fgp.actors[i].actor;
  if(R32(p,0)!=fgp.actors[i].pid||R32(p,4)!=a||R32(p,8)!=R32(a,0x3b4)||
   R32(p,12)!=R8(a,0x326)||R32(p,16)!=R32(a,0xb0))return 1116;
 }return 0;
}
/* Only active inline room members are writable pointers, and every target
 * is generated from a sealed CURRENT allocation plus a checked logical slot.
 * Source/before pointer words are integer index evidence, never destinations.
 * Linked room heads/next pointers must already have the same logical graph.
 * The inverse uses the separately captured current before-image order. */
static u32 fgp_topology_values(u32 row,u32 write){u32 g,i,j,p,a,src,idx,word,dst,z,n,off,stride,cap;
 for(g=0;g<4;g++){
  a=fgp.roots[g].pointer;if(!a)continue;p=row+fgp_rows[g];src=R32(p,0);
  if(g==0||g==2){
   off=g==0?107840:17648;dst=a+(g==0?107980:17788);
   for(i=0;i<64;i++){
    z=_gz_fgt_relocate_pointer(R32(p,off+i*4),src,a,g,&word);if(z)return z;
    if(R32(dst,i*4)!=word)return 1124;
   }
   for(i=0;i<fgp_counts[g];i++)if(fgp.topology.registered[fgt_masks[g]+(i>>5)]&(1u<<(i&31))){
    z=_gz_fgt_relocate_pointer(R32(p,16+i*68+64),src,a,g,&word);if(z)return z;
    if(R32(a,156+i*68+64)!=word)return 1124;
   }
  }else{
   off=g==1?20272:34096;stride=g==1?268:812;cap=g==1?64:200;
   for(i=0;i<64;i++){
    dst=a+(g==1?20412:127032)+i*stride;n=R32(p,off+i*stride);
    if(n>cap||R32(dst,0)!=n||R32(dst,4)!=cap||R32(dst,8)!=dst+12)return 1124;
    for(j=0;j<n;j++){
     word=R32(p,off+i*stride+12+j*4);
     if(!word||!fgt_index(word,src,g,&idx)||idx==BAD||!(fgp.topology.registered[fgt_masks[g]+(idx>>5)]&(1u<<(idx&31))))return 1124;
     z=_gz_fgt_relocate_pointer(word,src,a,g,&word);if(z)return z;
     if(write)*(volatile u32*)(dst+12+j*4)=word;else if(R32(dst,12+j*4)!=word)return 1124;
    }
   }
  }
 }return 0;
}
static u32 fgp_prepare(u32 t,u32 f){u32 z,i,q,a,count=0,s,n,row,before,source_frame,relation,source_epoch;const TxObserveSource*v;
 if(!fgp_empty()||_gz_roh_gl_context(t,f,1)!=1||!(v=tx_observe_source(t,f)))return 1104;
 s=v->source;n=v->used;if(R32(s,156)!=0x60000u)return 1105;row=s+R32(s,152)+24608;
 if(row<s||row-s>n||JP_FOLIAGE_BYTES>n-(row-s)||_gz_foliage_record_safe((const void*)row))return 1105;
 source_frame=R32(s+R32(s,168),20);if(!source_frame||f-source_frame>=0x80000000u)return 1105;
 relation=fgp_source_context(t,f,s,n,v->pin,&source_epoch);if(!relation)return 1125;
 before=jp_acquire(t,f,s);if(!before)return 1106;
 for(i=8;i<24;i++)_gz_fgp_report[i]=0;
 bcl_zero(&fgp,sizeof(fgp));fgp.magic=0x46475050u;fgp.bytes=sizeof(fgp);fgp.token=t;fgp.frame=f;fgp.phase=FGP_PREPARED;
 fgp.relation=relation;fgp.source_epoch=source_epoch;fgp.current_epoch=_gz_pbac_epoch;
 fgp.source=s;fgp.used=n;fgp.pin=v->pin;fgp.row=row;fgp.row_hash=btr_hash((const void*)row,JP_FOLIAGE_BYTES);fgp.source_frame=source_frame;fgp.before=before;
 for(i=0;i<4;i++){
  FgpRoot*r=fgp.roots+i;r->slot=fgp_slots[i];r->bytes=fgp_sizes[i];r->pointer=a=R32(r->slot,0);
  if(relation==1&&R32(row,fgp_rows[i])!=a){z=1107;goto fail;}if(!a){if(R32(row,fgp_rows[i])){z=1107;goto fail;}continue;}
  z=_gz_ob_capture(a,r->bytes,&r->allocation);_gz_fgp_report[8+i]=z;if(z){z=1108;goto fail;}
  z=relation==1?_gz_ob_validate_before(&r->allocation,source_frame):_gz_ob_validate(&r->allocation);if(z){z=1109;goto fail;}
  r->vtable=R32(a,12);if(R32(row,fgp_rows[i]+4)!=r->vtable){z=1110;goto fail;}
  r->hash=btr_hash((const void*)a,r->bytes);_gz_fgp_report[16+i]=a;_gz_fgp_report[20+i]=r->allocation.generation;
 }
 q=_gz_pba_native_actor_queue;
 while(q){if(++count>4096||!span(q,16)){z=1111;goto fail;}a=R32(q,12);
  if(a&&R16(a,8)==274){FgpPlant*r;if(fgp.plants==64||_gz_po_lookup(a)||!_gz_po_observation.valid){z=1111;goto fail;}
   r=fgp.actors+fgp.plants;btr_copy(&r->process,&_gz_po_observation.entry,sizeof(PoEntry));
   if(r->process.state!=PO_LIVE||r->process.object!=a||r->process.process_name!=274||r->process.flags||
    !r->process.generation||r->process.end_frame||r->process.pid!=R32(a,4)||r->process.profile!=R16(a,14)||
    r->process.profile_entry!=R32(a,16)||R8(a,12)!=2||r->process.allocation_bytes<0x52d||r->process.allocation_bytes>0x100000u){z=1111;goto fail;}
   r->actor=a;r->pid=R32(a,4);z=_gz_ob_capture(a,r->process.allocation_bytes,&r->allocation);if(z){z=1111;goto fail;}
   if(_gz_ob_validate_before(&r->allocation,source_frame)){z=1111;goto fail;}
   r->hash=btr_hash((const void*)a,r->process.allocation_bytes);fgp.plants++;
  }q=R32(q,8);
 }
 if(R32(row,4)!=fgp.plants){z=1112;goto fail;}
 bcl_zero((void*)before,JP_FOLIAGE_BYTES);z=_gz_foliage_capture((void*)before);if(z){z=1113;goto fail;}
 z=_gz_foliage_preflight((const void*)before);if(z)goto fail;
 z=_gz_fgt_manager_topology((const u8*)row,(const u8*)before,JP_FOLIAGE_BYTES,&fgp.topology);
 _gz_fgp_report[28]=z;_gz_fgp_report[29]=fgp.topology.roots;_gz_fgp_report[30]=fgp.topology.placements;
 _gz_fgp_report[31]=fgp.topology.order_rooms;_gz_fgp_report[32]=fgp.topology.order_members;
 _gz_fgp_report[33]=relation;_gz_fgp_report[34]=source_epoch;_gz_fgp_report[35]=fgp.current_epoch;
 if(z&&z!=FGT_ORDER_REPLAY)goto fail;
 z=fgp_plants_match(row);if(z)goto fail;
 fgp.before_hash=btr_hash((const void*)before,JP_FOLIAGE_BYTES);fgp.valid=1;fgp_seal_now();
 z=fgp_bind(1);if(!z)z=fgp_topology_values(before,0);if(z)goto fail;
 if(_gz_roh_gl_context(t,f,1)!=1||!tx_observe_matches(t,f,s,n,fgp.pin)){z=1114;goto fail;}
 _gz_fgp_report[2]=t;_gz_fgp_report[3]=FGP_PREPARED;_gz_fgp_report[4]=0;_gz_fgp_report[5]=f;
 _gz_fgp_report[6]=fgp.plants;_gz_fgp_report[7]=before;_gz_fgp_report[24]=fgp.before_hash;return 0;
fail:_gz_fgp_report[4]=z;return z;
}
static void fgp_piece(u32 dst,u32 src,u32 bytes,u32 write,u32*bad){if(write)btr_copy((void*)dst,(const void*)src,bytes);else if(!btr_equal((const void*)dst,(const void*)src,bytes))*bad=1115;}
static u32 fgp_values(u32 row,u32 write,u32 partial){u32 g,i,a,p,pos,stride,unit,n,bad=0,anim,animstride;
 for(g=0;g<4;g++){
  a=fgp.roots[g].pointer;if(!a)continue;p=row+fgp_rows[g];stride=fgp_stride[g];unit=g==3?120:g==1?168:64;
  pos=p+16;
  for(i=0;i<fgp_counts[g];i++){
   u32 dst=a+(g==3?0x98:0x9c)+i*stride;
   if(fgp.topology.registered[fgt_masks[g]+(i>>5)]&(1u<<(i&31))){
    if(g==3){fgp_piece(dst,pos,14,write,&bad);fgp_piece(dst+16,pos+16,104,write,&bad);}
    else fgp_piece(dst,pos,unit,write,&bad);
   }
   pos+=g==3?120:stride;
   if(write&&partial&&g==0&&i==7)return 1119;
  }
  anim=a+fgp_anim[g];animstride=g==1?132:g==3?140:56;n=fgp_anim_n[g];
  for(i=0;i<n;i++){
   if(g==3)fgp_piece(anim,pos,139,write,&bad);
   else{fgp_piece(anim,pos,1,write,&bad);fgp_piece(anim+2,pos+2,g==1?130:4,write,&bad);if(g!=1)fgp_piece(anim+8,pos+8,48,write,&bad);}
   anim+=animstride;pos+=animstride;
  }
 }
 p=row+163456;
 for(i=0;i<fgp.plants;i++,p+=32){a=fgp.actors[i].actor;
  if(R32(p,0)!=fgp.actors[i].pid||R32(p,4)!=a)return 1116;
  fgp_piece(a+0x524,p+20,9,write,&bad);
 }
 if(bad)return bad;
 return fgp_topology_values(row,write);
}
static u32 fgp_apply(u32 t,u32 f,u32 partial){u32 z;
 if(fgp_entered||!fgp_sealed(t)||fgp.phase!=FGP_PREPARED||!fgp_forward_scope(t,f))return 1117;
 fgp_entered=1;z=fgp_bind(1);if(z)goto done;
 if(!fgp_forward_scope(t,f)||btr_hash((const void*)fgp.row,JP_FOLIAGE_BYTES)!=fgp.row_hash){z=1117;goto done;}
 z=fgp_values(fgp.before,0,0);if(z)goto done;
 fgp.phase=FGP_APPLYING;fgp_seal_now();z=fgp_values(fgp.row,1,partial);if(z)goto done;
 z=fgp_bind(0);if(!z)z=fgp_values(fgp.row,0,0);if(!z&&!fgp_forward_scope(t,f))z=1117;
 if(!z){fgp.phase=FGP_APPLIED;fgp_seal_now();_gz_fgp_report[3]=FGP_APPLIED;_gz_fgp_report[25]=f;}
done:_gz_fgp_report[4]=z;fgp_entered=0;return z;
}
static u32 fgp_undo(u32 t,u32 f){u32 z;
 if(fgp_entered||!fgp_sealed(t)||fgp.phase==FGP_ROLLED||!fgp_recovery_scope(t,f))return 1118;
 fgp_entered=1;z=fgp_bind(0);if(z)goto done;
 if(fgp.phase!=FGP_PREPARED){fgp.phase=FGP_UNDOING;fgp_seal_now();z=fgp_values(fgp.before,1,0);if(z)goto done;}
 z=fgp_values(fgp.before,0,0);if(!z)z=fgp_bind(0);if(!z&&!fgp_recovery_scope(t,f))z=1118;
 if(!z){fgp.phase=FGP_ROLLED;fgp_seal_now();_gz_fgp_report[3]=FGP_ROLLED;_gz_fgp_report[26]=f;}
done:_gz_fgp_report[4]=z;fgp_entered=0;return z;
}
static u32 fgp_before(u32 t){u32 i,z=fgp_bind(0);if(z)return z;
 if(fgp.token!=t)return 1118;
 if(fgp.phase!=FGP_ROLLED)return 1118;z=fgp_values(fgp.before,0,0);if(z)return z;
 for(i=0;i<4;i++)if(fgp.roots[i].pointer&&btr_hash((const void*)fgp.roots[i].pointer,fgp.roots[i].bytes)!=fgp.roots[i].hash)return 1120;
 for(i=0;i<fgp.plants;i++)if(btr_hash((const void*)fgp.actors[i].actor,fgp.actors[i].process.allocation_bytes)!=fgp.actors[i].hash)return 1120;
 _gz_fgp_report[27]=1;return 0;
}
static u32 fgp_saved_check(u32 t,u32 f){u32 z;
 if(!fgp_receipt(t)||fgp.phase!=FGP_APPLIED||!_gz_bci_read_scope(t,f)||
  fgp.source!=_gz_hbp_recipe.source||fgp.used!=_gz_hbp_recipe.used||fgp.pin!=_gz_hbp_recipe.pin||
  btr_hash((const void*)fgp.row,JP_FOLIAGE_BYTES)!=fgp.row_hash)return 1123;
 z=fgp_bind(0);if(!z)z=fgp_values(fgp.row,0,0);return z;
}
static void fgp_finish_leaf(u32 t){if(fgp.token==t){jp_finish_leaf(t);bcl_zero(&fgp,sizeof(fgp));fgp_seal=0;}}
static u32 fgp_forget(u32 t){const volatile u32*o=_gz_roh_owner;
 if(o[3]||o[4]||o[5]||o[6]||o[12]||_gz_osa_pin_id()||fgp_entered)return 1122;
 if(fgp_empty())return 0;
 if(!t||fgp.token!=t||fgp.phase>FGP_PREPARED||jp_lease.token!=t)return 1122;
 fgp_finish_leaf(t);return fgp_empty()?0:1122;
}

static u32 fgp_core_target(u32 t,u32 f){u32 z;
 if(fgp_entered||!fgp_sealed(t)||fgp.phase!=FGP_APPLIED||!_gz_wct_target_scope(t,f)||
  btr_hash((const void*)fgp.row,JP_FOLIAGE_BYTES)!=fgp.row_hash)return 1123;
 z=fgp_bind(0);if(!z)z=fgp_values(fgp.row,0,0);return z;
}
