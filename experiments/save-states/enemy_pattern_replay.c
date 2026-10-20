/* Existing-allocation Keese pattern replay. Before-images for the complete
 * bank must already be sealed. This does not publish a saved world: body,
 * numeric state, hidden-model attachment and actor reconstruction still need
 * their complete saved-state contracts before ordinary restore is admitted. */
enum {EKR_CONTEXT=530,EKR_SOURCE=531,EKR_NATIVE=532,EKR_CHANGED=533};
typedef struct {u32 magic,bytes,valid,token,frame,phase,attempted,applied,undone,error,rollback_error,changed;} EkrRun;
typedef char EkrRunSize[sizeof(EkrRun)==48?1:-1];
static EkrRun _gz_ekr_run;
static u32 _gz_ekr_seal,_gz_ekr_entered;
u32 _gz_ekr_report[32]={0x454b5231u,128};
extern u32 _gz_ept_native_reuse(u32,u32,u32,u32);
extern const u8 _gz_ext_keese_face_ids[];
static void ekr_seal(void){_gz_ekr_seal=btr_hash(&_gz_ekr_run,sizeof(_gz_ekr_run));}
static u32 ekr_empty(void){u32 i;const u32*p=(const u32*)&_gz_ekr_run;
 if(_gz_ekr_seal||_gz_ekr_entered)return 0;
 for(i=0;i<sizeof(_gz_ekr_run)/4;i++)if(p[i])return 0;return 1;
}
static u32 ekr_sealed(void){const EkrRun*r=&_gz_ekr_run;
 return r->magic==0x454b5231u&&r->bytes==sizeof(*r)&&r->valid==1&&r->token&&
  (r->phase==2||r->phase==3||r->phase==4||r->phase==5)&&r->attempted<=_gz_eks_plan.count&&
  r->applied<=r->attempted&&r->undone<=r->attempted&&_gz_ekr_seal==btr_hash(r,sizeof(*r));
}
u32 _gz_ekr_private_apart(u32 p,u32 n){return
 eks_apart(p,n,(u32)&_gz_ekr_run,sizeof(_gz_ekr_run))&&
 eks_apart(p,n,(u32)&_gz_ekr_seal,4)&&eks_apart(p,n,(u32)&_gz_ekr_entered,4)&&
 eks_apart(p,n,(u32)_gz_ekr_report,sizeof(_gz_ekr_report));}
static void ekr_report(void){const EkrRun*r=&_gz_ekr_run;
 _gz_ekr_report[2]=r->token;_gz_ekr_report[3]=r->phase;_gz_ekr_report[4]=r->error;
 _gz_ekr_report[5]=r->frame;_gz_ekr_report[6]=r->attempted;_gz_ekr_report[7]=r->applied;
 _gz_ekr_report[8]=r->undone;_gz_ekr_report[9]=_gz_ekr_seal;_gz_ekr_report[10]=r->changed;
 _gz_ekr_report[11]=r->rollback_error;
}
/* Native Keese render frames are exact integer floats from an unsigned byte.
 * Decode that saved scalar without reading a saved pointer or using soft-float
 * helper libraries. Negative, fractional, NaN and out-of-range values refuse. */
static u32 ekr_frame(u32 bits,u32 maximum,u32*out){u32 exponent,mantissa,shift,value;
 if(!(bits&0x7fffffffu)){*out=0;return maximum!=0;}
 if(bits&0x80000000u)return 0;exponent=(bits>>23)&255;
 if(exponent<127||exponent>134)return 0;
 shift=150-exponent;mantissa=(bits&0x7fffffu)|0x800000u;
 if(mantissa&((1u<<shift)-1))return 0;value=mantissa>>shift;
 if(value>=maximum||value>255)return 0;*out=value;return 1;
}
static u32 ekr_float(u32 value){u32 exponent=0,n=value;
 if(!value)return 0;while(n>1){n>>=1;exponent++;}
 return ((127+exponent)<<23)|((value-(1u<<exponent))<<(23-exponent));
}
static u32 ekr_target(const EksRow*r,u32*frame,u32*logical){u32 p=r->source_row,id,z;
 if(!eks_external(p,10240)||btr_hash((const void*)p,10240)!=r->source_hash||
  R32(p,0)!=r->pid||R32(p,52)!=215||R32(p,4800)!=1||R32(p,4816)!=r->target_resource.index||
  R32(p,4820)>1||!epj_finite(R32(p,4880))||!epj_finite(R32(p,4884))||
  !ekr_frame(R32(p,4884),r->target.animations[1].max_frame,frame))return EKR_SOURCE;
 if(!R32(p,4820)){if(R32(p,4856)!=2)return EKR_SOURCE;*logical=0;return 0;}
 if(R32(p,4856)!=1||R32(p,4852)>3)return EKR_SOURCE;
 id=R16((u32)_gz_ext_keese_face_ids,(R32(p,4852)+(R8(p,5248)?4:0))*2);
 z=_gz_ern_resolve(r->target_resource.token,215,id,ERN_PATTERN);if(z)return z;
 if(!_gz_ekr_private_apart(_gz_ern_resolved.wrapper,16))return EKS_ALIAS;
 if(_gz_ern_resolved.header!=r->target_resource.header||_gz_ern_resolved.table!=r->target_resource.table)return EKR_CHANGED;
 *logical=_gz_ern_resolved.wrapper;
 return 0;
}
static u32 ekr_match(const EksRow*r){const EpjBounds*b=&r->journal.layout.bounds;u32 p=r->source_row,frame,logical,z;
 z=ekr_target(r,&frame,&logical);if(z)return z;
 return R32(b->face,16)==r->target_resource.wrapper&&
  R32(b->face+0x1c,0x44)==r->target_resource.data&&
  R32(b->actor,0x4e0)==logical&&
  btr_equal((const void*)b->face,(const void*)(p+4880),16)&&
  btr_equal((const void*)(b->actor+0x4e8),(const void*)(p+5240),6)?0:EKR_CHANGED;
}
u32 _gz_ekr_apply(u32 token){EkrRun*run=&_gz_ekr_run;const EksPlan*p=&_gz_eks_plan;
 u32 frame=_gz_pbac_frame,i,j,z=0,target_frame,logical,status;
 if(!ekr_empty()||_gz_eks_entered||!eks_sealed()||p->token!=token||_gz_eks_run.phase!=1||
  p->frame!=frame||!_gz_ern_scope(token,frame))return EKR_CONTEXT;
 /* Validate all source scalars and mutable ownership before the first SDK
  * call. Shared resource wrappers may alias; per-actor heaps may not. */
 for(i=0;i<p->count;i++){
  const EksRow*r=p->rows+i;
  z=eks_binding(r);if(z)return z;z=_gz_epj_before_matches(&r->journal);if(z)return z;
  z=ekr_target(r,&target_frame,&logical);if(z)return z;
  for(j=0;j<i;j++)if(!eks_apart(r->heap.heap,r->heap.bytes,p->rows[j].heap.heap,p->rows[j].heap.bytes)||
   !eks_apart(r->actor,0x10bc,p->rows[j].actor,0x10bc))return EKS_ALIAS;
 }
 if(btr_hash((const void*)p->bank,p->bank_bytes)!=p->bank_hash||!_gz_ern_scope(token,frame))return EKR_SOURCE;
 eks_zero(run,sizeof(*run));run->magic=0x454b5231u;run->bytes=sizeof(*run);run->valid=1;
 run->token=token;run->frame=frame;run->phase=2;ekr_seal();_gz_ekr_entered=1;
 for(i=0;i<p->count;i++){
  const EksRow*r=p->rows+i;const EpjBounds*b=&r->journal.layout.bounds;
  z=eks_binding(r);if(z)goto done;z=ekr_target(r,&target_frame,&logical);if(z)goto done;
  if(!_gz_ern_scope(token,frame)){z=EKR_CONTEXT;goto done;}
  /* Mark attempted BEFORE even the shared frame store, so an SDK refusal
   * after partial writes cannot discard this row's native before-image. */
  run->attempted=i+1;ekr_seal();_gz_ekr_report[13]=i;
  *(volatile u32*)r->target_resource.wrapper=ekr_float(target_frame);
  status=_gz_ept_native_reuse(b->face,r->target.resources.model_wrapper,r->target_resource.wrapper,target_frame);
  _gz_ekr_report[12]=status;
  if(!status){z=EKR_NATIVE;goto done;}
  *(volatile u32*)(b->actor+0x4e0)=logical;
  btr_copy((void*)(b->actor+0x4e8),(const void*)(r->source_row+5240),6);
  btr_copy((void*)b->face,(const void*)(r->source_row+4880),16);
  z=eks_binding(r);if(z)goto done;z=ekr_match(r);if(z)goto done;
  if(R32(r->target.resources.model_wrapper,0x38)!=b->face+0x1c){z=EKR_CHANGED;goto done;}
  run->applied++;if(_gz_epj_before_matches(&r->journal))run->changed++;ekr_seal();
  /* Existing private fault point4 now proves undo after a real partial bank.
   * The outer point remains for zero-enemy banks. Strip BIF before delivery. */
  if(!i){z=_gz_bif_after_part(token,3,4);if(z)goto done;}
 }
 for(i=0;i<p->count;i++){z=eks_binding(p->rows+i);if(z)goto done;z=ekr_match(p->rows+i);if(z)goto done;}
 /* Attachment is shared by actors using the same model wrapper. Its final
  * value belongs to the last actual SDK writer, not to every row at once. */
 for(i=0;i<p->count;i++){
  for(j=i+1;j<p->count;j++)if(p->rows[j].target.resources.model_wrapper==p->rows[i].target.resources.model_wrapper)break;
  if(j==p->count&&R32(p->rows[i].target.resources.model_wrapper,0x38)!=p->rows[i].journal.layout.bounds.face+0x1c){z=EKR_CHANGED;goto done;}
 }
 if(!_gz_ern_scope(token,frame)||btr_hash((const void*)p->bank,p->bank_bytes)!=p->bank_hash){z=EKR_CHANGED;goto done;}
 run->phase=3;
done:run->error=z;ekr_seal();ekr_report();_gz_ekr_entered=0;return z;
}
u32 _gz_ekr_recovery_undo(u32 token){EkrRun*r=&_gz_ekr_run;u32 i,z;
 if(ekr_empty())return 0;
 if(_gz_ekr_entered||!token||!bor_original()||_gz_boh_recovery.phase!=BOR_UNDO||
  _gz_boh_recovery.token!=token||!eks_sealed()||!ekr_sealed()||r->token!=token||
  (r->phase!=2&&r->phase!=3&&r->phase!=5)||r->rollback_error)return EKR_CONTEXT;
 z=_gz_roh_recovery_common(token,_gz_pbac_frame);if(z)return z;
 /* All current generation/allocation/resource bindings are checked before
  * any native undo store. No saved source body is read on this path. */
 for(i=0;i<_gz_eks_plan.count;i++){z=eks_binding(_gz_eks_plan.rows+i);if(z)return z;}
 if(r->phase==5)return 0;
 _gz_ekr_entered=1;r->phase=4;ekr_seal();
 for(i=r->attempted;i;i--){z=_gz_epj_undo(&_gz_eks_plan.rows[i-1].journal);if(z)goto done;r->undone++;ekr_seal();}
 for(i=0;i<_gz_eks_plan.count;i++){z=_gz_epj_before_matches(&_gz_eks_plan.rows[i].journal);if(z)goto done;}
 z=_gz_roh_recovery_common(token,_gz_pbac_frame);if(!z)r->phase=5;
done:r->rollback_error=z;ekr_seal();ekr_report();_gz_ekr_entered=0;return z;
}
u32 _gz_ekr_recovery_check(u32 token,u32 frame){u32 z;
 if(ekr_empty())return 0;
 if(_gz_ekr_entered||frame!=_gz_pbac_frame||!ekr_sealed()||_gz_ekr_run.token!=token||
  _gz_ekr_run.phase!=5||_gz_ekr_run.rollback_error||_gz_ekr_run.undone!=_gz_ekr_run.attempted)return EKR_CHANGED;
 z=_gz_roh_recovery_common(token,frame);return z;
}
static void ekr_clear(void){eks_zero(&_gz_ekr_run,sizeof(_gz_ekr_run));_gz_ekr_seal=_gz_ekr_entered=0;}
