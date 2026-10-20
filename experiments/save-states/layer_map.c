#include "layer_map.h"
LmPlan _gz_lgm_work;
static u32 required[LG_CAP],used[LG_CAP];
static u32 mix(u32 h,u32 v){u32 i;for(i=0;i<4;i++){h=(h^(v>>24))*16777619u;v<<=8;}return h;}
u32 _gz_lgm_hash(const LmPlan*p){u32 i,j,h=2166136261u;const u32*q=(const u32*)p;
 if(p!=&_gz_lgm_work||p->source_count>LG_CAP)return 0;
 for(i=0;i<24;i++)h=mix(h,i==3?1:i==15?0:q[i]);
 for(i=0;i<p->source_count;i++){q=(const u32*)&p->rows[i];for(j=0;j<12;j++)h=mix(h,q[j]);}return h;
}
static u32 apart(u32 a,u32 n,u32 b,u32 m){return a+n>=a&&b+m>=b&&(a+n<=b||b+m<=a);}
static u32 key(const LgRow*a,const LgRow*b){
 return a->profile==b->profile&&a->process_name==b->process_name&&a->profile_entry==b->profile_entry&&
  a->parameters==b->parameters&&a->base_methods==b->base_methods&&a->node_methods==b->node_methods&&a->list_count==b->list_count;
}
static u32 identity(const LgRow*a,const LgRow*b){return a->generation&&a->generation==b->generation&&a->pid==b->pid&&a->owner==b->owner&&a->layer==b->layer;}
static void bind(u32 i,u32 j,const LgBank*l,u32 kind,u32 count){LmRow*r=&_gz_lgm_work.rows[i];const LgRow*b=&l->rows[j];
 r->current_index=j;r->current_layer=b->layer;r->current_owner=b->owner;r->current_generation=b->generation;r->kind=kind;r->candidates=count;used[j]=1;
}
static u32 fail(u32 z){_gz_lgm_work.valid=0;_gz_lgm_work.result=z;return z;}
u32 _gz_lgm_plan(const LgBank*s,const VrWorld*sw,const BbBank*sb,const LgBank*l,const VrWorld*lw,const BbBank*lb){
 LmPlan*p=&_gz_lgm_work;u32 i,j,k,depth,phase,n,exact,choice,reverse,remaining=0,progress,scene;
 p->valid=0;
 if(!apart((u32)p,sizeof(*p),(u32)s,LG_BYTES)||!apart((u32)p,sizeof(*p),(u32)sw,sizeof(*sw))||!apart((u32)p,sizeof(*p),(u32)sb,BB_BYTES)||
  !apart((u32)p,sizeof(*p),(u32)l,LG_BYTES)||!apart((u32)p,sizeof(*p),(u32)lw,sizeof(*lw))||!apart((u32)p,sizeof(*p),(u32)lb,BB_BYTES))return fail(1);
 if(_gz_lgb_validate(s,LG_BYTES,sw,sb)||_gz_lgb_validate(l,LG_BYTES,lw,lb))return fail(2);
 if(s->session!=l->session||l->epoch<s->epoch||l->frame-s->frame>=0x80000000u)return fail(3);
 /* Generation contradictions must not be disguised as placement matches. */
 for(i=0;i<s->count;i++)if(i!=s->root){
  for(j=0;j<i;j++)if(j!=s->root&&s->rows[i].generation==s->rows[j].generation)return fail(4);
  for(j=0;j<l->count;j++)if(j!=l->root&&s->rows[i].generation==l->rows[j].generation&&
   (!identity(&s->rows[i],&l->rows[j])||!key(&s->rows[i],&l->rows[j])))return fail(4);
 }
 for(i=0;i<l->count;i++)if(i!=l->root)for(j=0;j<i;j++)if(j!=l->root&&l->rows[i].generation==l->rows[j].generation)return fail(4);
 for(i=0;i<sizeof(*p)/4;i++)((volatile u32*)p)[i]=0;
 for(i=0;i<LG_CAP;i++)required[i]=used[i]=0;
 for(i=0;i<s->actor_count;i++){
  j=s->actor_layer[i];
  for(k=0;k<s->count;k++){required[j]=1;if(j==s->root)break;j=s->rows[j].parent;}
  if(j!=s->root)return fail(5);
 }
 p->magic=LM_MAGIC;p->bytes=sizeof(*p);p->version=1;p->source_count=s->count;p->current_count=l->count;
 p->source_hash=s->checksum;p->current_hash=l->checksum;p->source_frame=s->frame;p->current_frame=l->frame;
 p->source_epoch=s->epoch;p->current_epoch=l->epoch;p->session=s->session;p->runtime_validation_required=1;
 scene=s->stage0!=l->stage0||s->stage1!=l->stage1||s->layer!=l->layer;if(scene)p->flags|=LM_SCENE;
 for(i=0;i<s->count;i++){
  LmRow*r=&p->rows[i];const LgRow*a=&s->rows[i];r->source_index=i;r->current_index=LM_NO;r->source_layer=a->layer;
  r->source_owner=a->owner;r->source_generation=a->generation;r->required=required[i];r->parent_source=a->parent;
  if(i!=s->root)remaining++;
 }
 bind(s->root,l->root,l,LM_ROOT,1);
 /* Parents are resolved first. All surviving identities at a given depth
  * reserve their targets before any fresh semantic candidate is selected. */
 for(depth=0;depth<s->count&&remaining&&!scene;depth++){
  progress=0;
  for(phase=0;phase<2;phase++)for(i=0;i<s->count;i++){
   LmRow*r=&p->rows[i];const LgRow*a=&s->rows[i];u32 parent;
   if(i==s->root||r->kind||a->parent>=s->count)continue;
   parent=p->rows[a->parent].current_index;if(parent==LM_NO)continue;
   n=exact=0;choice=LM_NO;
   for(j=0;j<l->count;j++)if(!used[j]&&j!=l->root&&l->rows[j].parent==parent&&key(a,&l->rows[j])){
    n++;if(identity(a,&l->rows[j])){exact++;choice=j;}else if(!exact)choice=j;
   }
   if(phase==0){if(exact==1){bind(i,choice,l,LM_SURVIVE,n);remaining--;progress++;}continue;}
   r->candidates=n;
   if(!n){r->kind=LM_MISSING;p->flags|=LM_ABSENT;remaining--;progress++;continue;}
   reverse=0;
   for(k=0;k<s->count;k++)if(k!=s->root&&p->rows[k].current_index==LM_NO&&s->rows[k].parent==a->parent&&key(&s->rows[k],&l->rows[choice]))reverse++;
   if(n==1&&reverse==1){bind(i,choice,l,LM_SEMANTIC,n);remaining--;progress++;}
   else{r->kind=LM_AMBIGUOUS;p->flags|=LM_AMBIG;remaining--;progress++;}
  }
  if(!progress)break;
 }
 for(i=0;i<s->count;i++){
  LmRow*r=&p->rows[i];
  if(!r->kind){r->kind=scene?LM_SCENE_BLOCKED:LM_PARENT_BLOCKED;p->flags|=scene?LM_SCENE:LM_PARENT;}
  if(r->kind<=LM_SEMANTIC)p->bound_count++;
  else if(r->kind==LM_MISSING)p->missing_count++;
  else if(r->kind==LM_AMBIGUOUS)p->ambiguous_count++;
  else p->blocked_count++;
  if(r->required){p->required_count++;if(r->current_index==LM_NO)p->required_unbound++;}
 }
 p->checksum=_gz_lgm_hash(p);__asm__ volatile("sync":::"memory");p->valid=1;return 0;
}
