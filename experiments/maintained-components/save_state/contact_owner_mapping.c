/* WGP-only current owner correspondence. This uses unused bytes inside the
 * original 512-PoEntry CGR owner area; no new native arena or global table.
 * The bounded rebirth lane has at most128 owners including parent/active-edge
 * dependencies. Ordinary survivor/factory storage and limits are unchanged.
 * Recipes, PO, CO and ObBinding remain observations. Actual permission comes
 * only from the existing genuine WGP source/HG/native-world protocol. */
#include "object_binding.h"
#include "contact_owner_key.h"
enum {CGR_REBOUND_MAGIC=0x43475242u,CGR_REBOUND_CAP=128,CGR_REBOUND_DEPTH=8};
typedef struct {ObBinding allocation;u32 source_index,source_pid,source_generation,source_actor,source_parent,current_parent,recipe_hash,relation,state;} CgrMappedOwner;
typedef char CgrMapFits[CGR_REBOUND_CAP*(sizeof(PoEntry)+sizeof(CgrMappedOwner))<=CGR_OWNERS*sizeof(PoEntry)?1:-1];
static u32 cgr_rebound(const CgrPlan*p){return p->reserved[0]==CGR_REBOUND_MAGIC;}
static CgrMappedOwner*cgr_maps(CgrPlan*p){return (CgrMappedOwner*)(cgr_owners(p)+CGR_REBOUND_CAP);}
/* The allocation observer rejects metadata arguments inside any tracked
 * native heap. Both the game-thread stack and the counted CGW arena can be
 * in those heaps. Keep the retained identities there, but give every observer
 * API only this fixed image buffer. Never retain the scratch as an identity.
 * The guard covers copy/API/copy; no callback or yield occurs in that span. */
static ObBinding cgr_binding_scratch;
static u32 cgr_binding_entered;

static u32 cgr_binding_enter(void)
{
 CgrPlan *plan;
 if(cgr_binding_entered){
  _gz_cgr_report[19]=0x10000001u;
  return 0;
 }
 cgr_binding_entered=1;
 plan=cgr_plan();
 if(!plan||!cgr_rebound(plan)||plan->frame!=_gz_pbac_frame||
    plan->reserved[5]!=_gz_pbac_epoch||
    !(_gz_wgp_contact_scope(plan->token,plan->frame,1)||
      _gz_wgp_contact_scope(plan->token,plan->frame,2)||
      _gz_wgp_contact_scope(plan->token,plan->frame,3)||
      _gz_wgp_contact_scope(plan->token,plan->frame,4))){
  _gz_cgr_report[19]=0x10000002u;
  cgr_binding_entered=0;
  return 0;
 }
 return 1;
}

static u32 cgr_binding_capture(u32 actor,u32 bytes,ObBinding *binding)
{
 u32 result;
 if(!cgr_binding_enter())return OB_OBSERVER;
 result=_gz_ob_capture(actor,bytes,&cgr_binding_scratch);
 _gz_cgr_report[19]=0x01000000u|result;
 if(!result){
  result=_gz_ob_validate(&cgr_binding_scratch);
  _gz_cgr_report[19]=0x02000000u|result;
 }
 if(!result)btr_copy(binding,&cgr_binding_scratch,sizeof(*binding));
 cgr_binding_entered=0;
 return result;
}

static u32 cgr_binding_validate(const ObBinding *binding)
{
 u32 result;
 if(!cgr_binding_enter())return OB_OBSERVER;
 btr_copy(&cgr_binding_scratch,binding,sizeof(*binding));
 result=_gz_ob_validate(&cgr_binding_scratch);
 _gz_cgr_report[19]=0x03000000u|result;
 cgr_binding_entered=0;
 return result;
}

static u32 cgr_binding_validate_before(const ObBinding *binding,u32 source_frame)
{
 u32 result;
 if(!cgr_binding_enter())return OB_OBSERVER;
 btr_copy(&cgr_binding_scratch,binding,sizeof(*binding));
 result=_gz_ob_validate_before(&cgr_binding_scratch,source_frame);
 _gz_cgr_report[19]=0x04000000u|result;
 cgr_binding_entered=0;
 return result;
}

static const BbEntry*cgr_map_source(const CgrPlan*p,const PoEntry*s){const VrWorld*w=(const VrWorld*)(p->source+R32(p->source,168));
 const BbBank*b=(const BbBank*)(p->source+R32(p->source,192));const BbEntry*found=0;u32 i;
 for(i=0;i<w->count;i++)if(w->actors[i].pid==s->pid){if(found||b->rows[i].index!=i||!btr_equal(&b->rows[i].process,s,sizeof(*s)))return 0;found=b->rows+i;}
 return found;
}
static const BbEntry*cgr_map_pid_source(const CgrPlan*p,u32 pid){const VrWorld*w=(const VrWorld*)(p->source+R32(p->source,168));
 const BbBank*b=(const BbBank*)(p->source+R32(p->source,192));const BbEntry*found=0;u32 i;
 for(i=0;i<w->count;i++)if(w->actors[i].pid==pid){if(found)return 0;found=b->rows+i;}return found;
}
static u32 cgr_map_scope(const CgrPlan*p){return cgr_rebound(p)&&p->frame==_gz_pbac_frame&&p->reserved[5]==_gz_pbac_epoch&&
 _gz_wgp_contact_scope(p->token,p->frame,1)&&_gz_roh_gl_context(p->token,p->frame,1)==1;}
/* Physical identity validation runs before any actor-header or collider read.
 * Its missing/session tags remain exactly as emitted by the real observer. */
static u32 cgr_map_current(const PoEntry*e,const ObBinding*b){PoEntry again;
 if(b->query!=e->object||b->query_bytes!=e->allocation_bytes||cgr_binding_validate(b)||
  _gz_po_lookup(e->object)||!_gz_po_observation.valid||_gz_po_observation.flags)return CGR_BIND;
 btr_copy(&again,&_gz_po_observation.entry,sizeof(again));
 if(!btr_equal(e,&again,sizeof(again))||e->state!=PO_LIVE||e->flags||e->end_frame||!e->generation||e->profile>=484||
  e->process_name!=e->profile||e->profile_entry!=_gz_po_profiles[e->profile]||!epj_ram(e->object,e->allocation_bytes)||
  R32(e->object,4)!=e->pid||R16(e->object,8)!=e->profile||R16(e->object,14)!=e->process_name||
  R32(e->object,16)!=e->profile_entry||R8(e->object,12)!=2)return CGR_BIND;
 return cgr_binding_validate(b)?CGR_BIND:0;
}
static u32 cgr_map_capture(u32 a,PoEntry*out,ObBinding*b){PoEntry e;
 if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return CGR_BIND;
 btr_copy(&e,&_gz_po_observation.entry,sizeof(e));
 if(e.object!=a||e.state!=PO_LIVE||e.flags||e.end_frame||!e.generation||e.allocation_bytes<0x300||e.allocation_bytes>0x100000u||
  e.base_bytes+e.other_bytes<e.base_bytes||e.base_bytes+e.other_bytes!=e.allocation_bytes||
  cgr_binding_capture(a,e.allocation_bytes,b)||cgr_map_current(&e,b))return CGR_BIND;
 btr_copy(out,&e,sizeof(*out));return 0;
}
static u32 cgr_map_resolve(CgrPlan*p,const PoEntry*s,u32 depth,u32*out){const BbEntry*old;const VrWorld*w;
 PoEntry e,chosen;ObBinding physical,chosen_physical;BcRow recipe;u32 i,q,next,a,head,left=4096,found=0,relation=0,k,parent=BAD,z,link;
 if(depth>=CGR_REBOUND_DEPTH||!cgr_map_scope(p)||(old=cgr_map_source(p,s))==0||!_gz_bc_row_valid(&old->request)||!cok_role(&old->request))return CGR_BIND;
 for(i=0;i<p->owner_count;i++)if(cgr_maps(p)[i].source_index==old->index){CgrMappedOwner*m=cgr_maps(p)+i;
  if(m->state!=2||m->source_pid!=s->pid||m->source_generation!=s->generation||m->source_actor!=s->object||cgr_map_current(cgr_owners(p)+i,&m->allocation))return CGR_BIND;*out=i;return 0;}
 /* An issued local source pin is the enclosing provenance boundary. These
  * extra exact current-catalog receipts reject foreign/recycled recipes; the
  * documented session0/World.session1 values are not session certificates. */
 if(_gz_bc_lookup(s->pid,s->profile)||!_gz_bc_result.valid||!btr_equal(&_gz_bc_result,&old->request,sizeof(BcRow)))return CGR_BIND;
 w=(const VrWorld*)(p->source+R32(p->source,168));link=s->profile==168;
 if(link&&(!lpc_sealed()||_gz_lpc_run.phase!=LPC_PREPARED||_gz_lpc_plan.token!=p->token||_gz_lpc_plan.frame!=p->frame||
  _gz_lpc_plan.source!=p->source||_gz_lpc_plan.used!=p->used||_gz_lpc_plan.pin!=p->pin||
  s->pid!=w->link_pid||s->object!=w->link_actor))return CGR_BIND;
 head=q=_gz_pba_native_actor_queue;
 while(q){
  if(!left--||!epj_ram(q,16))return CGR_BIND;next=R32(q,8);a=R32(q,12);
  if(a){
   if(_gz_po_lookup(a)||!_gz_po_observation.valid||_gz_po_observation.flags)return CGR_BIND;
   btr_copy(&e,&_gz_po_observation.entry,sizeof(e));
   if(e.profile==s->profile){
    if(cgr_map_capture(a,&e,&physical))return CGR_BIND;
    relation=cok_lifetime(s,&e,w->frame,p->frame,w->epoch,_gz_pbac_epoch);
    if(relation){
     if(_gz_bc_lookup(e.pid,e.profile)||!_gz_bc_result.valid||!_gz_bc_row_valid(&_gz_bc_result))return CGR_BIND;
     btr_copy(&recipe,&_gz_bc_result,sizeof(recipe));
     if(recipe.pid!=e.pid||recipe.profile_entry!=e.profile_entry||e.born_frame-recipe.frame_after>=0x80000000u||
      p->frame-recipe.frame_after>=0x80000000u)return CGR_BIND;
     if(link?btr_equal(&e,&_gz_lpc_plan.process,sizeof(e)):cok_recipe(&old->request,&recipe)){
      if(link&&lpc_binding(0))return CGR_BIND;
      if(!cok_role(&recipe)||R32(a,0x2e8)!=cok_word(recipe.append,28)||++found!=1||
       (relation==1&&cgr_binding_validate_before(&physical,w->frame)))return CGR_BIND;
      btr_copy(&chosen,&e,sizeof(e));btr_copy(&chosen_physical,&physical,sizeof(physical));parent=cok_word(recipe.append,28);
     }
    }
   }
  }q=next;
 }
 if(found!=1||head!=_gz_pba_native_actor_queue||p->owner_count==CGR_REBOUND_CAP||cgr_map_current(&chosen,&chosen_physical))return CGR_BIND;
 relation=cok_lifetime(s,&chosen,w->frame,p->frame,w->epoch,_gz_pbac_epoch);if(!relation)return CGR_BIND;
 for(i=0;i<p->owner_count;i++)if(cgr_owners(p)[i].object==chosen.object||cgr_owners(p)[i].pid==chosen.pid)return CGR_BIND;
 k=p->owner_count++;btr_copy(cgr_owners(p)+k,&chosen,sizeof(chosen));{
  CgrMappedOwner*m=cgr_maps(p)+k;bcl_zero(m,sizeof(*m));btr_copy(&m->allocation,&chosen_physical,sizeof(chosen_physical));
  m->source_index=old->index;m->source_pid=s->pid;m->source_generation=s->generation;m->source_actor=s->object;m->source_parent=cok_word(old->request.append,28);
  m->current_parent=parent;m->recipe_hash=cok_hash(&old->request);m->relation=relation;m->state=1;
  if(w->actors[old->index].parent_pid!=m->source_parent)return CGR_BIND;
  if(m->source_parent==BAD){if(parent!=BAD)return CGR_BIND;}
  else{const BbEntry*parent_old=cgr_map_pid_source(p,m->source_parent);if(!parent_old)return CGR_BIND;
   z=cgr_map_resolve(p,&parent_old->process,depth+1,&i);if(z||cgr_owners(p)[i].pid!=parent)return CGR_BIND;}
  if(cgr_map_current(cgr_owners(p)+k,&m->allocation)||R32(chosen.object,0x2e8)!=parent||!cgr_map_scope(p))return CGR_BIND;m->state=2;
 }*out=k;return 0;
}
static u32 cgr_map_observed_survivor(const PoEntry*s){return !_gz_po_lookup(s->object)&&_gz_po_observation.valid&&!_gz_po_observation.flags&&
 btr_equal(s,&_gz_po_observation.entry,sizeof(*s));}
static u32 cgr_map_prepare(CgrPlan*p){const VrWorld*w=(const VrWorld*)(p->source+R32(p->source,168));const BbBank*b;
 u32 i,j,k,z,needed=0;const CgpRow*s;
 if(!_gz_wgp_contact_scope(p->token,p->frame,1))return 0;
 if(_gz_bcb_archive_validate(p->source,p->used,0)||_gz_vws_validate(w,sizeof(*w)))return CGR_SOURCE;
 if(w->epoch==_gz_pbac_epoch)return 0;
 /* An advanced scene counter alone does not replace a process lifetime.
  * Preserve the original survivor admission, including its full512-owner
  * limit and supported layouts, whenever every recorded owner still matches.
  * These observer queries follow no historical/native actor address. */
 for(i=0;i<p->count;i++){
  s=cgr_source_row(p,i);if(!cgr_map_observed_survivor(&s->owner)||
   (s->info[5]&&!cgr_map_observed_survivor(&s->status_owner))){needed=1;break;}
 }
 if(!needed)return 0;
 if(!w->epoch||w->epoch>=_gz_pbac_epoch||_gz_pbac_epoch>0xffffffu||w->frame==p->frame||p->frame-w->frame>=0x80000000u||
  w->stage0!=_gz_roh_owner[41]||w->stage1!=_gz_roh_owner[42]||w->room!=_gz_roh_owner[43]||w->layer!=_gz_roh_owner[44]||
  _gz_roh_owner[45]!=_gz_pbac_epoch||!lpc_sealed()||_gz_lpc_run.phase!=LPC_PREPARED||_gz_lpc_plan.token!=p->token||
  _gz_lpc_plan.frame!=p->frame||_gz_lpc_plan.source!=p->source||_gz_lpc_plan.used!=p->used||_gz_lpc_plan.pin!=p->pin||
  _gz_lpc_plan.actor!=_gz_roh_owner[29]||_gz_lpc_plan.pid!=_gz_roh_owner[30])return CGR_BIND;
 b=(const BbBank*)(p->source+R32(p->source,192));if(b->process_session!=_gz_po_header.session)return CGR_BIND;
 bcl_zero(cgr_owners(p),CGR_OWNERS*sizeof(PoEntry));p->reserved[0]=CGR_REBOUND_MAGIC;p->reserved[1]=w->checksum;
 p->reserved[2]=b->checksum;p->reserved[3]=w->frame;p->reserved[4]=w->epoch;p->reserved[5]=_gz_pbac_epoch;
 for(i=0;i<p->count;i++){
  s=cgr_source_row(p,i);
  if(s->info[2]!=1||s->info[3]||s->info[4]||s->owner_heap_generation||
   (s->info[5]&&(s->info[13]!=1||s->info[14]||s->info[15]||s->status_heap_generation||!btr_equal(&s->owner,&s->status_owner,sizeof(PoEntry)))))return CGR_BIND;
  z=cgr_map_resolve(p,&s->owner,0,&k);if(z)return z;
  if(s->info[11]){const BbEntry*peer=cgr_map_pid_source(p,s->info[12]);if(!peer||peer->process.object!=s->info[11])return CGR_SOURCE;
   z=cgr_map_resolve(p,&peer->process,0,&k);if(z)return z;}
  for(j=0;j<3;j++)if(s->edges[j][3]!=BAD&&(s->edges[j][3]||(s->edges[j][0]&1)||s->edges[j][1]||s->edges[j][4])){
   const BbEntry*peer=cgr_map_pid_source(p,s->edges[j][3]);if(!peer)return CGR_SOURCE;
   z=cgr_map_resolve(p,&peer->process,0,&k);if(z)return z;
  }
 }
 return _gz_bcb_archive_validate(p->source,p->used,0)||!tx_observe_matches(p->token,p->frame,p->source,p->used,p->pin)?CGR_SOURCE:0;
}
static u32 cgr_map_lookup(CgrPlan*p,const PoEntry*s,PoEntry*out){u32 i;const BbEntry*old;if(!cgr_map_scope(p))return CGR_CONTEXT;old=cgr_map_source(p,s);if(!old)return CGR_SOURCE;
 for(i=0;i<p->owner_count;i++)if(cgr_maps(p)[i].source_index==old->index){CgrMappedOwner*m=cgr_maps(p)+i;
  if(m->state!=2||m->source_pid!=s->pid||m->source_generation!=s->generation||m->source_actor!=s->object||cgr_map_current(cgr_owners(p)+i,&m->allocation))return CGR_BIND;
  btr_copy(out,cgr_owners(p)+i,sizeof(*out));return 0;}return CGR_BIND;
}
static u32 cgr_map_physical(CgrPlan*p,const PoEntry*e){u32 i;if(!cgr_rebound(p))return 0;
 if(p->frame!=_gz_pbac_frame||p->reserved[5]!=_gz_pbac_epoch||!(_gz_wgp_contact_scope(p->token,p->frame,1)||
  _gz_wgp_contact_scope(p->token,p->frame,2)||_gz_wgp_contact_scope(p->token,p->frame,3)||_gz_wgp_contact_scope(p->token,p->frame,4)))return CGR_CONTEXT;
 for(i=0;i<p->owner_count;i++)if(btr_equal(e,cgr_owners(p)+i,sizeof(*e)))return cgr_maps(p)[i].state==2?cgr_map_current(e,&cgr_maps(p)[i].allocation):CGR_BIND;
 return CGR_BIND;
}
/* Parent/active-edge-only dependencies may own no collider row. Validate the
 * entire retained map before every real forward/target/recovery phase. This
 * follows no source pointer and never edits a historical observer identity. */
static u32 cgr_map_all(CgrPlan*p){u32 i,z;if(!cgr_rebound(p))return 0;
 for(i=0;i<p->owner_count;i++){
  z=cgr_map_physical(p,cgr_owners(p)+i);if(z)return z;
  if(R32(cgr_owners(p)[i].object,0x2e8)!=cgr_maps(p)[i].current_parent)return CGR_BIND;
 }return 0;
}
static u32 cgr_map_history(CgrPlan*p,u32 id,const PoEntry*old){u32 i,found=0;PoEntry e;
 if(id==BAD||!id)return 0;
 if(_gz_po_header.busy||_gz_po_header.flags||!btr_equal(&p->po,&_gz_po_header,sizeof(PoHeader)))return CGR_CHANGED;
 for(i=0;i<PO_CAP;i++){
  btr_copy(&e,_gz_po_rows+i,sizeof(e));if(e.state!=PO_LIVE||e.pid!=id)continue;
  if(++found!=1||!old||!btr_equal(&e,old,sizeof(e)))return CGR_BIND;
 }
 return btr_equal(&p->po,&_gz_po_header,sizeof(PoHeader))?0:CGR_CHANGED;
}
static u32 cgr_map_extent(CgrPlan*p,const PoEntry*s,const PoEntry*l,u32 audit,u32 n,u32 kind,u32 heap,u32 gen,u32 epoch,u32*out){u32 off;
 if(!cgr_rebound(p))return cgr_extent(l,audit,n,kind,heap,gen,epoch,out);
 if(kind!=1||heap||gen||epoch||audit<s->object||(off=audit-s->object)>s->allocation_bytes||n>s->allocation_bytes-off||
  off>l->allocation_bytes||n>l->allocation_bytes-off)return CGR_BIND;*out=l->object+off;return 0;
}
static u32 cgr_map_collider(CgrPlan*p,const CgpRow*s,const CgpRow*d){const VrWorld*w;u32 i,relation=0;
 if(!cgr_rebound(p))return d->collider.generation==s->collider.generation&&d->collider.born_frame==s->collider.born_frame?0:CGR_BIND;
 for(i=0;i<p->owner_count;i++)if(btr_equal(&d->owner,cgr_owners(p)+i,sizeof(PoEntry)))relation=cgr_maps(p)[i].relation;
 if(relation==1)return d->collider.generation==s->collider.generation&&d->collider.born_frame==s->collider.born_frame?0:CGR_BIND;
 w=(const VrWorld*)(p->source+R32(p->source,168));
 return relation==2&&d->collider.generation>s->collider.generation&&d->collider.born_frame!=w->frame&&
  d->collider.born_frame-w->frame<0x80000000u&&d->collider.born_frame-d->owner.born_frame<0x80000000u&&
  p->frame-d->collider.born_frame<0x80000000u&&s->collider.ctor_site&&d->collider.ctor_site==s->collider.ctor_site&&
  d->collider.roles==s->collider.roles&&(!d->collider.object_vtable||d->collider.object_vtable==s->info[6])?0:CGR_BIND;
}
/* Sealing and recovery inspect only retained private metadata. The source is
 * consulted in PREPARE only; no source pointer is followed by this validator. */
static u32 cgr_map_valid(CgrPlan*p){u32 i,j;CgrMappedOwner*m;
 if(!cgr_rebound(p)||!p->owner_count||p->owner_count>CGR_REBOUND_CAP||!p->reserved[1]||!p->reserved[2]||
  !p->reserved[3]||p->frame-p->reserved[3]>=0x80000000u||!p->reserved[4]||p->reserved[4]>=p->reserved[5]||p->reserved[5]>0xffffffu)return 0;
 for(i=6;i<11;i++)if(p->reserved[i])return 0;
 for(i=0;i<p->owner_count;i++){
  m=cgr_maps(p)+i;if(m->state!=2||m->source_index>=VR_CAPACITY||!m->source_pid||!m->source_generation||!m->source_actor||(m->source_actor&3u)||
   !m->recipe_hash||(m->relation!=1&&m->relation!=2)||m->allocation.query!=cgr_owners(p)[i].object||
   m->allocation.query_bytes!=cgr_owners(p)[i].allocation_bytes)return 0;
  for(j=0;j<i;j++)if(cgr_maps(p)[j].source_index==m->source_index||cgr_maps(p)[j].source_pid==m->source_pid||cgr_maps(p)[j].source_actor==m->source_actor||
   cgr_owners(p)[j].object==cgr_owners(p)[i].object||cgr_owners(p)[j].pid==cgr_owners(p)[i].pid)return 0;
  if(m->source_parent==BAD){if(m->current_parent!=BAD)return 0;}
  else{for(j=0;j<p->owner_count;j++)if(cgr_maps(p)[j].source_pid==m->source_parent)break;
   if(j==p->owner_count||cgr_owners(p)[j].pid!=m->current_parent)return 0;}
 }return 1;
}
static u32 cgr_owner_hash(CgrPlan*p){u32 h=cgr_hash(cgr_owners(p),p->owner_count*sizeof(PoEntry),BAD);
 return cgr_rebound(p)?(h^cgr_hash(cgr_maps(p),p->owner_count*sizeof(CgrMappedOwner),BAD))*16777619u:h;
}
