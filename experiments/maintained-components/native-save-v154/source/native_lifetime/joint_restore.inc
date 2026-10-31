typedef char AjRequestSize[sizeof(AjRestoreRequest)==144?1:-1];
typedef char AjResultSize[sizeof(AjRestoreResult)==64?1:-1];
static U32 jr_entry_allowed(const ArDomain*d){return !jr.busy||(jr.active&&jr.thread==(U32)OSGetCurrentThread()&&
 jr.request&&d==&jr.request->domain);}
static const U8*jr_directory(const void*b,const AjHeader*h,U32 i,U32*bytes){const AjIndex*ix=(const AjIndex*)((const U8*)b+h->index);
 *bytes=ix[i].bytes;return(const U8*)b+ix[i].offset;
}
static U32 jr_total(const void*b,const AjHeader*h){U32 i,n=0,bytes;const U8*p;
 for(i=0;i<h->count;i++){p=jr_directory(b,h,i,&bytes);n+=R32(p,28);}return n;
}
static void jr_retain(U32 input,U32 output){
 jr.active=0;jr.input=input;jr.result_address=output;jr.recovery_stage=_gz_ar_stage;
 copy(&jr.recovery_journal,&journal,sizeof(journal));
 /* A poisoned/inflight observer is deliberately not repaired here. Its
  * failed stamp can never authenticate a resume; ownership remains held. */
 if(_gz_hr_stamp(jr.recovery_stamp))zero(jr.recovery_stamp,sizeof(jr.recovery_stamp));
 __sync_synchronize();jr.recovery=1;
}
static void jr_clear(void){zero((U8*)&jr+4,sizeof(jr)-4);__sync_synchronize();jr.busy=0;}
static U32 jr_before_native(const ArDomain*d,const void*source,U32 bytes,const void*undo,U32 n){U32 stamp[4],id[2];
 if(!jr.active)return jr.busy?AR_BUSY:0;
 if(!jr_entry_allowed(d)||source!=jr.source_af||bytes!=jr.source.afs_bytes||n!=jr.undo.afs_bytes||
  undo!=(const U8*)jr.request->undo+64)return AR_CHANGED;
 if(_gz_hr_stamp(stamp)||!equal(stamp,jr.stamp,sizeof(stamp)))return AR_CHANGED;
 if(_gz_hr_heap_identity(d->current.graph.root,id)||id[0]!=jr.root_generation)return AR_CHANGED;
 jr.ar_undo=undo;jr.ar_undo_bytes=n;return 0;
}
static U32 jr_after_apply(const AfDomain*d,const void*p,U32 bytes,U32 i){
 const AjHeader*h;const void*b;const U8*dir;U32 role,n,id[2],stamp[4],code;HrReceipt r;
 if(!jr.active)return jr.busy?AR_BUSY:0;
 if(jr_foreign_thread())return AR_RECOVERY;
 if(p==jr.source_af&&bytes==jr.source.afs_bytes&&d==&jr.request->domain.source){role=1;h=&jr.source;b=(const void*)jr.request->source;}
 else if(p==jr.ar_undo&&bytes==jr.ar_undo_bytes&&d==&jr.request->domain.current){role=2;h=&jr.undo;b=(const void*)jr.request->undo;}
 else return AR_RECOVERY;
 if(i>=h->count)return AR_RECOVERY;
 if(!i){if(_gz_hr_stamp(stamp))return AR_RECOVERY;jr.birth_floor=stamp[0];jr.phase=role;jr.next=0;jr.mapped=0;}
 if(jr.phase!=role||i!=jr.next)return AR_RECOVERY;
 dir=jr_directory(b,h,i,&n);
 if(_gz_hr_heap_identity(R32(dir,12),id)){jr.hr_error=HR_OBSERVER;return AR_RECOVERY;}
 /* The root is retained. Every child MUST have an actual observed birth
  * after this rebuild began, even when its address and kind match an old
  * incarnation. Never repair a missing native callback by inventing a gen. */
 if((!i&&id[0]!=jr.root_generation)||(i&&id[0]<=jr.birth_floor)){jr.hr_error=HR_OBSERVER;return AR_RECOVERY;}
 zero(&r,sizeof(r));code=_gz_hr_rebind(dir,n,id[0],id[1],(HrMap*)(jr.request->map+jr.mapped*sizeof(HrMap)),
  jr.request->map_capacity-jr.mapped*sizeof(HrMap),&r);
 jr.hr_error=code;
 if(code){
  if(!r.mutated&&(code==HR_LAYOUT||code==HR_CHANGED)&&!_gz_hr_stamp(stamp))return AR_CHANGED;
  return AR_RECOVERY;
 }
 if(!r.valid||r.mapped!=R32(dir,28)||r.missing!=31)return AR_RECOVERY;
 jr.mapped+=r.mapped;jr.next++;return 0;
}
AgU32 _gz_aj_restore(const AjRestoreRequest*q,AjRestoreResult*out){
 U32 z=AR_ARGUMENT,i,j,ok=0,n=0,id[2],stamp[4],sizes[6],starts[6];AjRestoreResult result;AjHeader sh,uh;const U8*dir;const AjRestoreRequest*input=q;
 if(!_gz_image_write_allowed((U32)out,sizeof(*out)))return AR_ARGUMENT;
 if(!__sync_bool_compare_and_swap(&jr.busy,0,1))return AR_BUSY;
 zero((U8*)&jr+4,sizeof(jr)-4);jr.thread=(U32)OSGetCurrentThread();zero(&result,sizeof(result));result.missing=31;
 if(!aj_meta((U32)q,sizeof(*q)))goto done;
 /* Consume a private prepared request throughout capture, native callbacks
  * and rollback. A caller's next UI request must not retarget this operation.
  * The real owner still provides source/buffer allocation leases. */
 copy(&jr.prepared,q,sizeof(*q));if(!equal(&jr.prepared,q,sizeof(*q)))goto done;q=&jr.prepared;
 if(!_gz_image_write_allowed(q->undo,q->undo_capacity)||!_gz_image_write_allowed(q->map,q->map_capacity))goto done;
 if(q->reserved[0]||q->reserved[1])goto done;
 starts[0]=q->source;sizes[0]=q->source_bytes;starts[1]=q->undo;sizes[1]=q->undo_capacity;
 starts[2]=q->scratch;sizes[2]=q->scratch_capacity;starts[3]=q->map;sizes[3]=q->map_capacity;
 starts[4]=(U32)out;sizes[4]=sizeof(*out);starts[5]=(U32)input;sizes[5]=sizeof(*input);
 if((q->scratch==0)!=(q->scratch_capacity==0))goto done;
 for(i=0;i<5;i++){
  if(i==2&&!sizes[i])continue;
  if(!aj_private(starts[i],sizes[i],&q->domain.source)||!apart(starts[i],sizes[i],(U32)input,sizeof(*input)))goto done;
  for(j=0;j<i;j++)if(!(j==2&&!sizes[j])&&!apart(starts[i],sizes[i],starts[j],sizes[j]))goto done;
 }
 z=_gz_aj_validate(&q->domain.source,(const void*)q->source,q->source_bytes);if(z)goto done;copy(&sh,(const void*)q->source,64);
 for(i=0;i<6;i++)if(!(i==2&&!sizes[i])&&!aj_graph_private(starts[i],sizes[i],graph_of((const U8*)q->source+64),sh.count)){z=AR_ARGUMENT;goto done;}
 /* Before touching native memory, freeze a complete independent current
  * bundle. Ar directly compares its embedded AFS against every current live
  * byte before mutation, retaining this same immutable image for rollback. */
 z=_gz_aj_capture(&q->domain.current,(void*)q->undo,q->undo_capacity,&n);if(z)goto done;copy(&uh,(const void*)q->undo,64);
 for(i=0;i<6;i++)if(!(i==2&&!sizes[i])&&!aj_graph_private(starts[i],sizes[i],graph_of((const U8*)q->undo+64),uh.count)){z=AR_ARGUMENT;goto done;}
 ok=1;result.undo_bundle_bytes=n;
 if(jr_total((const void*)q->source,&sh)>q->map_capacity/sizeof(HrMap)||jr_total((const void*)q->undo,&uh)>q->map_capacity/sizeof(HrMap)){z=AR_SPACE;goto done;}
 if(_gz_hr_stamp(stamp)||stamp[0]!=uh.serial||stamp[1]!=uh.release_serial){z=AR_CHANGED;goto done;}
 dir=jr_directory((const void*)q->undo,&uh,0,&n);jr.root_generation=R32(dir,20);
 if(_gz_hr_heap_identity(q->domain.current.graph.root,id)||id[0]!=jr.root_generation||id[1]!=R32(dir,24)){z=AR_CHANGED;goto done;}
 jr.request=q;jr.source_af=(const U8*)q->source+64;copy(&jr.source,&sh,64);copy(&jr.undo,&uh,64);copy(jr.stamp,stamp,sizeof(stamp));jr.active=1;
 z=_gz_ar_restore_prepared(&q->domain,jr.source_af,sh.afs_bytes,(const U8*)q->undo+64,uh.afs_bytes,&result.arena);
 result.phase=jr.phase;result.hr_error=jr.hr_error;
 if(result.arena.mutation_started){
  const AjHeader*expected=z==AR_ROLLED_BACK?&uh:&sh;
  if((z==AR_OK||z==AR_ROLLED_BACK)&&jr.next==expected->count&&jr.phase==(z==AR_ROLLED_BACK?2u:1u)&&
   ((const AjHeader*)q->source)->checksum==sh.checksum&&((const AjHeader*)q->undo)->checksum==uh.checksum&&
   aj_hash((const void*)q->source,q->source_bytes)==sh.checksum&&aj_hash((const void*)q->undo,uh.bytes)==uh.checksum){
   result.mapped=jr.mapped;result.valid=1;
  }else{z=AR_RECOVERY;result.arena.code=z;result.arena.owner_action=AR_OWNER_RECOVER;result.arena.pending_epoch=0;}
 }
done:if(ok){result.arena.code=z;copy(out,&result,sizeof(result));}
 if(ok&&result.arena.mutation_started&&z!=AR_OK&&z!=AR_ROLLED_BACK){jr_retain((U32)input,(U32)out);return z;}
 jr_clear();return z;
}
AgU32 _gz_aj_recovery_pending(void){return jr.busy&&jr.recovery?1:0;}
AgU32 _gz_aj_recover(const AjRestoreRequest*input,AjRestoreResult*out){
 AjRestoreResult result;const AjRestoreRequest*q;U32 z=AR_RECOVERY,stamp[4],id[2],native_started=0;
 if(!jr.busy||jr.recovery!=1||jr_foreign_thread())return AR_BUSY;
 if((U32)input!=jr.input||(U32)out!=jr.result_address||!aj_meta((U32)input,sizeof(*input))||
  !aj_meta((U32)out,sizeof(*out))||!_gz_image_write_allowed((U32)out,sizeof(*out)))return AR_ARGUMENT;
 if(!equal(input,&jr.prepared,sizeof(*input)))return AR_CHANGED;
 if(!__sync_bool_compare_and_swap(&jr.recovery,1,2))return AR_BUSY;
 zero(&result,sizeof(result));result.missing=31;result.undo_bundle_bytes=jr.undo.bytes;
 result.arena.mutation_started=1;result.arena.owner_action=AR_OWNER_RECOVER;
 q=&jr.prepared;
 if(!_gz_image_write_allowed(q->map,q->map_capacity)){z=AR_ARGUMENT;goto done;}
 if(!jr.recovery_stamp[0]||_gz_hr_stamp(stamp)||!equal(stamp,jr.recovery_stamp,sizeof(stamp))||
  _gz_hr_heap_identity(q->domain.current.graph.root,id)||id[0]!=jr.root_generation){z=AR_CHANGED;goto done;}
 if(_gz_ar_stage!=jr.recovery_stage||!equal(&journal,&jr.recovery_journal,sizeof(journal))){z=AR_CHANGED;goto done;}
 /* The original source may be corrupt. Undo admission depends only on the
  * retained independent before-image, never that source or current payload. */
 z=_gz_aj_validate(&q->domain.current,(void*)q->undo,jr.undo.bytes);if(z)goto done;
 if(((const AjHeader*)q->undo)->checksum!=jr.undo.checksum||aj_hash((void*)q->undo,jr.undo.bytes)!=jr.undo.checksum){z=AR_CORRUPT;goto done;}
 jr.active=1;z=recover_undo(&q->domain,(const U8*)q->undo+64,jr.undo.afs_bytes,&result.arena,&native_started);jr.active=0;
 result.phase=jr.phase;result.hr_error=jr.hr_error;
 if(z==AR_ROLLED_BACK&&result.arena.pending_epoch==q->domain.rollback_epoch&&jr.phase==2&&jr.next==jr.undo.count&&
  ((const AjHeader*)q->undo)->checksum==jr.undo.checksum&&aj_hash((void*)q->undo,jr.undo.bytes)==jr.undo.checksum){result.mapped=jr.mapped;result.valid=1;}
 else if(z==AR_ROLLED_BACK){z=AR_RECOVERY;result.arena.pending_epoch=0;result.arena.owner_action=AR_OWNER_RECOVER;}
done:result.arena.code=z;copy(out,&result,sizeof(result));
 if(z==AR_ROLLED_BACK&&result.valid){jr_clear();return z;}
 if(native_started)jr_retain((U32)input,(U32)out);
 else {__sync_synchronize();jr.recovery=1;} /* refusal cannot ratchet the original stamp */
 return z;
}
