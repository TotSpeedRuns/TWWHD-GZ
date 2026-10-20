/* Typed face before-image leaf. This is not a restore admission point.
 * Future caller must hold the independently certified process/heap/archive
 * lifetime and transaction recovery capability before invoking undo.
 * Hashes protect private integrity; they do not confer that authority. */
/* Contract already included by player_face_capture.c. */
typedef struct {
 u32 magic,bytes,valid,used,plan_hash,data_hash;
 u8 data[1700];
} FaceUndo;
typedef char FaceUndoSize[sizeof(FaceUndo)==1724?1:-1];
enum {FJ_PRIVATE=246,FJ_CHANGED=247};
static u32 face_hash(const void*data,u32 n){const volatile u8*p=data;u32 h=2166136261u,i;for(i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
static u32 face_equal(const void*a,const void*b,u32 n){u32 i;for(i=0;i<n;i++)if(((const volatile u8*)a)[i]!=((const volatile u8*)b)[i])return 0;return 1;}
static u32 face_disjoint(u32 a,u32 n,u32 b,u32 m){return a+n<=b||b+m<=a;}
static u32 face_private(const FaceBounds*b,const FacePlan*p,const FaceUndo*u){u32 a=(u32)u,n=sizeof(*u);
 if(a&3||!fpc_contract_whole(a,n)||!face_disjoint(a,n,b->actor,b->actor_bytes)||
 !face_disjoint(a,n,b->heap,b->end-b->heap)||!face_disjoint(a,n,b->animation,b->animation_bytes)||
 !face_disjoint(a,n,b->model_file,b->model_bytes)||!face_disjoint(a,n,(u32)p,sizeof(*p))||
 !face_disjoint(a,n,(u32)b,sizeof(*b)))return 0;
 return 1;
}
static u32 face_plan_current(const FaceBounds*b,const FacePlan*p){FacePlan current;u32 z;
 if(!p||p->magic!=0x46435031u||p->bytes!=sizeof(*p)||p->valid!=1||p->error)return FJ_PRIVATE;
 z=_gz_face_contract(b,p->object,p->resource,p->pattern,&current);if(z)return z;
 return face_equal(p,&current,sizeof(current))?0:FJ_CHANGED;
}
u32 _gz_face_journal_capture(const FaceBounds*b,const FacePlan*p,FaceUndo*u){u32 z,i,j,k=0;
 if(!b||!u)return FJ_PRIVATE;
 z=face_plan_current(b,p);if(z)return z;
 if(!face_private(b,p,u))return FJ_PRIVATE;
 for(i=0;i<sizeof(*u);i++)((volatile u8*)u)[i]=0;
 u->magic=0x464a5531u;u->bytes=sizeof(*u);u->plan_hash=face_hash(p,sizeof(*p));
 for(i=0;i<p->range_count;i++){
  const FaceRange*r=&p->ranges[i];
  if(r->count>(sizeof(u->data)-k)/r->width)return FJ_PRIVATE;
  for(j=0;j<r->count;j++){
   u32 q;for(q=0;q<r->width;q++)u->data[k++]=U8(r->base+j*r->stride,q);
  }
 }
 u->used=k;u->data_hash=face_hash(u->data,k);u->valid=1;return 0;
}
u32 _gz_face_journal_undo(const FaceBounds*b,const FacePlan*p,const FaceUndo*u){u32 z,i,j,k=0,n=0;
 if(!b||!u)return FJ_PRIVATE;
 z=face_plan_current(b,p);if(z)return z;
 if(!face_private(b,p,u)||u->magic!=0x464a5531u||u->bytes!=sizeof(*u)||u->valid!=1||
 u->used>sizeof(u->data)||u->plan_hash!=face_hash(p,sizeof(*p))||u->data_hash!=face_hash(u->data,u->used))return FJ_PRIVATE;
 for(i=0;i<p->range_count;i++)n+=p->ranges[i].count*p->ranges[i].width;
 if(n!=u->used)return FJ_PRIVATE;
 /* Every possible refusal precedes the first native store. Only certified
  * typed bytes are restored; cached pointer words and padding are untouched. */
 for(i=0;i<p->range_count;i++){
  const FaceRange*r=&p->ranges[i];
  for(j=0;j<r->count;j++){
   u32 q;for(q=0;q<r->width;q++)((volatile u8*)(r->base+j*r->stride))[q]=u->data[k++];
  }
 }
 return 0;
}
