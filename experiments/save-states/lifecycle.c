#include "lifecycle.h"
extern u32 _gz_vws_validate(const VrWorld*,u32);
LpPlan _gz_lsp_work;
static u32 used[VR_CAPACITY],matched[VR_CAPACITY],ordered[VR_CAPACITY],live_row[VR_CAPACITY],removed[LP_CAP];
static u32 span(u32 p,u32 n){return !(p&3)&&p>=0x01000000u&&p<0xc0000000u&&n&&n<=0xc0000000u-p;}
static u32 apart(u32 a,u32 n,u32 b,u32 m){return a+n<=b||b+m<=a;}
static u32 mix(u32 h,u32 v){u32 i;for(i=0;i<4;i++){h=(h^(v>>24))*16777619u;v<<=8;}return h;}
u32 _gz_lsp_hash(const LpPlan*p){u32 h=2166136261u,i,j;const u32*q=(const u32*)p;
 if(p!=&_gz_lsp_work||p->row_count>LP_CAP||p->create_order_count>VR_CAPACITY||p->remove_order_count>VR_CAPACITY)return 0;
 for(i=0;i<36;i++)h=mix(h,i==3?1:i==15?0:q[i]);
 for(i=0;i<p->row_count;i++){q=(const u32*)&p->rows[i];for(j=0;j<12;j++)h=mix(h,q[j]);}
 for(i=0;i<p->create_order_count;i++)h=mix(h,p->create_order[i]);
 for(i=0;i<p->remove_order_count;i++)h=mix(h,p->remove_order[i]);return h;
}
static u32 same_place(const VrActor*a,const VrActor*b){u32 i;
 if(a->profile!=b->profile||a->parameters!=b->parameters||a->set_id!=b->set_id)return 0;
 for(i=0;i<5;i++)if(a->home[i]!=b->home[i])return 0;return 1;
}
static u32 candidate(const VrWorld*s,const BbBank*b,const VrWorld*l,const PoEntry*f,u32 i,u32 j){
 return b->rows[i].requested_profile==f[j].profile&&b->rows[i].process.profile_entry==f[j].profile_entry&&same_place(&s->actors[i],&l->actors[j]);
}
static void bind(LpRow*r,const VrWorld*l,const PoEntry*f,u32 j,u32 kind){
 r->kind=kind;r->live_index=j;r->live_pid=l->actors[j].pid;r->live_actor=l->actors[j].actor;r->live_generation=f[j].generation;r->candidate_count=1;used[j]=1;
}
static u32 fail(u32 z){_gz_lsp_work.valid=0;_gz_lsp_work.result=z;_gz_lsp_work.row_count=0;return z;}
u32 _gz_lsp_plan(const VrWorld*s,const BbBank*b,const VrWorld*l,const PoEntry*f){
 LpPlan*p=&_gz_lsp_work;u32 i,j,k,n,choice,reverse,area,progress,parent;const PoEntry*a;LpRow*r;
 p->valid=0;
 if(!span((u32)s,sizeof(*s))||!span((u32)b,BB_HEADER)||!span((u32)l,sizeof(*l))||!span((u32)f,VR_CAPACITY*sizeof(*f))||
  !apart((u32)p,sizeof(*p),(u32)s,sizeof(*s))||!apart((u32)p,sizeof(*p),(u32)b,BB_BYTES)||
  !apart((u32)p,sizeof(*p),(u32)l,sizeof(*l))||!apart((u32)p,sizeof(*p),(u32)f,VR_CAPACITY*sizeof(*f)))return fail(1);
 if(_gz_vws_validate(s,sizeof(*s))||_gz_bcb_validate(b,b->bytes,s)||_gz_vws_validate(l,sizeof(*l)))return fail(2);
 if(s->session!=l->session||l->epoch<s->epoch||l->frame-s->frame>=0x80000000u)return fail(3);
 for(i=0;i<l->count;i++){
  a=&f[i];
  if(a->object!=l->actors[i].actor||a->pid!=l->actors[i].pid||a->process_name!=l->actors[i].profile||a->state!=PO_LIVE||
   !a->generation||a->flags||a->end_frame||a->profile>=484||!a->profile_entry||a->allocation_bytes<0x300||
   a->allocation_bytes!=a->base_bytes+a->other_bytes||a->allocation_bytes<a->base_bytes||l->frame-a->born_frame>=0x80000000u)return fail(4);
  for(j=0;j<i;j++)if(a->generation==f[j].generation)return fail(4);
  for(j=0;j<s->count;j++)if(a->generation==b->rows[j].generation&&
   (a->object!=b->rows[j].process.object||a->pid!=b->rows[j].process.pid||a->profile!=b->rows[j].requested_profile||
    a->profile_entry!=b->rows[j].process.profile_entry||a->process_name!=s->actors[j].profile))return fail(5);
 }
 for(i=0;i<sizeof(*p)/4;i++)((volatile u32*)p)[i]=0;
 for(i=0;i<VR_CAPACITY;i++){used[i]=ordered[i]=0;matched[i]=live_row[i]=LP_NO;}
 for(i=0;i<LP_CAP;i++)removed[i]=0;
 p->magic=LP_MAGIC;p->bytes=sizeof(*p);p->version=1;p->source_count=s->count;p->live_count=l->count;p->row_count=s->count;
 p->frame=l->frame;p->source_epoch=s->epoch;p->live_epoch=l->epoch;p->source_hash=s->checksum;p->live_hash=l->checksum;p->source_bank_hash=b->checksum;
 p->source_session=s->session;p->live_session=l->session;p->source_sequence=s->sequence;p->live_sequence=l->sequence;p->runtime_validation_required=1;
 area=s->stage0!=l->stage0||s->stage1!=l->stage1||s->room!=l->room||s->layer!=l->layer;if(area)p->flags|=LP_AREA;
 /* Preserve genuine surviving generation evidence before considering any
  * placement lookalikes. Mutable actor state is not an allocation identity. */
 for(i=0;i<s->count;i++){
  r=&p->rows[i];r->source_index=i;r->live_index=r->live_pid=r->parent_index=LP_NO;r->source_pid=s->actors[i].pid;r->source_generation=b->rows[i].generation;
  for(j=0;j<l->count;j++)if(b->rows[i].process.object==f[j].object&&b->rows[i].generation==f[j].generation&&b->rows[i].process.pid==f[j].pid){
   if(used[j]||b->rows[i].requested_profile!=f[j].profile||b->rows[i].process.profile_entry!=f[j].profile_entry||s->actors[i].profile!=l->actors[j].profile)return fail(5);
   bind(r,l,f,j,LP_SURVIVE);matched[i]=j;break;
  }
 }
 for(i=0;i<s->count;i++)if(matched[i]==LP_NO){
  r=&p->rows[i];choice=LP_NO;n=0;
  if(s->actors[i].profile==168){
   for(j=0;j<l->count;j++)if(!used[j]&&l->actors[j].profile==168){choice=j;n++;}
   if(n!=1)return fail(6);bind(r,l,f,choice,LP_PLAYER);matched[i]=choice;continue;
  }
  if(!area)for(j=0;j<l->count;j++)if(!used[j]&&candidate(s,b,l,f,i,j)){choice=j;n++;}
  r->candidate_count=n;
  if(!n){r->kind=LP_CREATE;continue;}
  reverse=0;for(k=0;k<s->count;k++)if(matched[k]==LP_NO&&s->actors[k].profile!=168&&candidate(s,b,l,f,k,choice))reverse++;
  if(n!=1||reverse!=1){r->kind=LP_AMBIGUOUS;p->flags|=LP_AMBIG;continue;}
  bind(r,l,f,choice,LP_REBIND);matched[i]=choice;
 }
 for(j=0;j<l->count;j++)if(!used[j]){
  if(p->row_count==LP_CAP)return fail(7);r=&p->rows[p->row_count++];r->source_index=LP_NO;r->live_index=j;r->source_pid=r->parent_index=LP_NO;
  r->live_pid=l->actors[j].pid;r->live_actor=l->actors[j].actor;r->live_generation=f[j].generation;r->kind=LP_REMOVE;
  if(!area)for(i=0;i<s->count;i++)if(p->rows[i].kind==LP_AMBIGUOUS&&candidate(s,b,l,f,i,j)){r->kind=LP_HOLD_AMBIGUOUS;break;}
 }
 for(i=0;i<s->count;i++){
  r=&p->rows[i];parent=s->actors[i].parent_pid;
  if(parent==LP_NO){r->parent_kind=LP_PARENT_NONE;if(r->live_index!=LP_NO&&l->actors[r->live_index].parent_pid!=LP_NO)r->flags|=LP_PARENT;}
  else{
   for(j=0;j<s->count;j++)if(s->actors[j].pid==parent)break;
   if(j<s->count){
    r->parent_kind=LP_PARENT_INTERNAL;r->parent_index=j;
    if(r->live_index!=LP_NO&&(p->rows[j].live_index==LP_NO||l->actors[r->live_index].parent_pid!=p->rows[j].live_pid))r->flags|=LP_PARENT;
    k=j;n=0;while(k!=LP_NO&&n++<=s->count){
     if(k==i){r->flags|=LP_CYCLE;p->flags|=LP_CYCLE;break;}
     parent=s->actors[k].parent_pid;for(k=0;k<s->count;k++)if(s->actors[k].pid==parent)break;
     if(k==s->count)k=LP_NO;
    }
   }else{r->parent_kind=parent?LP_PARENT_EXTERNAL:LP_PARENT_ZERO;p->external_parent_count++;p->flags|=LP_EXTERNAL;}
  }
  if(r->flags&LP_PARENT){p->parent_rebind_count++;p->flags|=LP_PARENT;}
 }
 for(i=0;i<p->row_count;i++)if(p->rows[i].live_index!=LP_NO){
  j=p->rows[i].live_index;if(live_row[j]!=LP_NO)return fail(8);live_row[j]=i;
 }
 for(i=s->count;i<p->row_count;i++){
  r=&p->rows[i];parent=l->actors[r->live_index].parent_pid;
  if(parent==LP_NO){r->parent_kind=LP_PARENT_NONE;continue;}
  for(j=0;j<l->count;j++)if(l->actors[j].pid==parent)break;
  if(j<l->count){r->parent_kind=LP_PARENT_INTERNAL;r->parent_index=live_row[j];}
  else{r->parent_kind=parent?LP_PARENT_EXTERNAL:LP_PARENT_ZERO;p->external_parent_count++;p->flags|=LP_EXTERNAL;}
 }
 for(i=0;i<p->row_count;i++)if(p->rows[i].kind==LP_REMOVE){
  parent=p->rows[i].live_pid;
  for(j=0;j<l->count;j++)if(l->actors[j].parent_pid==parent&&p->rows[live_row[j]].kind!=LP_REMOVE){
   p->rows[i].flags|=LP_RETAINED_CHILD;p->flags|=LP_RETAINED_CHILD;p->retained_child_count++;
  }
 }
 /* Parent creations precede child creations. A cycle never gets a fabricated
  * execution order; all work remains a diagnostic inventory difference. */
 do{progress=0;
  for(i=0;i<s->count;i++)if(p->rows[i].kind==LP_CREATE&&!ordered[i]&&!(p->rows[i].flags&LP_CYCLE)){
   j=p->rows[i].parent_index;
   if(j!=LP_NO&&p->rows[j].kind==LP_CREATE&&!ordered[j])continue;
   ordered[i]=1;p->create_order[p->create_order_count++]=i;progress=1;
  }
 }while(progress);
 do{progress=0;
  for(i=s->count;i<p->row_count;i++)if(p->rows[i].kind==LP_REMOVE&&!removed[i]){
   for(j=s->count;j<p->row_count;j++)if(p->rows[j].kind==LP_REMOVE&&!removed[j]&&p->rows[j].parent_kind==LP_PARENT_INTERNAL&&p->rows[j].parent_index==i)break;
   if(j<p->row_count)continue;
   removed[i]=1;p->remove_order[p->remove_order_count++]=i;progress=1;
  }
 }while(progress);
 for(i=0;i<p->row_count;i++)switch(p->rows[i].kind){
 case LP_SURVIVE:p->surviving_count++;break;case LP_REBIND:p->rebind_count++;break;case LP_CREATE:p->create_count++;break;
 case LP_REMOVE:p->remove_count++;break;case LP_AMBIGUOUS:p->ambiguous_count++;break;case LP_HOLD_AMBIGUOUS:p->held_count++;break;case LP_PLAYER:p->owned_player_count++;break;
 default:return fail(8);
 }
 if(p->remove_order_count!=p->remove_count)p->flags|=LP_CYCLE;
 if(_gz_vr_world_hash(s)!=p->source_hash||_gz_vr_world_hash(l)!=p->live_hash||b->checksum!=p->source_bank_hash)return fail(9);
 p->checksum=_gz_lsp_hash(p);__asm__ volatile("sync":::"memory");p->valid=1;return 0;
}
