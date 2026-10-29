/* Real split producer: park before typed writers; unpark only AFTER
 * their genuine inverse restores contact manager/mass before-images. The
 * R2 immediate roundtrip remains byteexact above, but is no longer called
 * by ordinary WGP forward. No Delete or parked-world commit is enabled. */
u32 _gz_ipk_native_park(u32 t,u32 f){u32 i,a,tag,z,end,count,all;
 if(!ipk_plan_valid(t)||!ipk_run_valid(t)||ipk_run.phase!=IPK_PREPARED||ipk_run.error||
  !_gz_jwf_atomic_cas(&ipk_entered,0,1))return IPK_PLAN_PHASE;
 z=IPK_PLAN_SOURCE;if(!ipk_source_current(t,f,2))goto done;
 z=_gz_ipk_admission_begin(t,f,2);if(z)goto done;
 z=IPK_PLAN_CHANGED;
 if(!ipk_lexical_current(t,f)||!ipk_queue(0)||
  !btr_equal(ipk_plan.root,(const void*)&_gz_pba_native_actor_queue,12)||
  !btr_equal(ipk_plan.prefix_tag,(const void*)ipk_plan.prefix_node,20))goto clean;
 for(i=0;i<ipk_plan.extra_count;i++){
  a=ipk_plan.extra_process[i].object;
  if(!ipk_owner_current(i,0)||!btr_equal(ipk_plan.tag[i],(const void*)(a+0xc8),20))goto clean;
 }
 z=_gz_ipk_admission_check();if(z)goto clean;
 all=(1u<<ipk_plan.extra_count)-1u;
 for(i=0;i<ipk_plan.extra_count;i++){
  if(!ipk_lexical_current(t,f)||!ipk_owner_current(i,0)){z=ipk_native_fail(t,IPK_PLAN_CHANGED);goto retained;}
  a=ipk_plan.extra_process[i].object;ipk_calling(1,i);
  ipk_run.pause_raw=ctp_native_step(t,f,CTP_PAUSE,i);ipk_run_seal();
  if(ipk_run.pause_raw!=1||!ipk_owner_current(i,1)||!ipk_lexical_current(t,f)){
   z=ipk_native_fail(t,IPK_PLAN_NATIVE);goto retained;}
  ipk_run.paused|=1u<<i;ipk_run_seal();
 }
 count=ipk_plan.current_count;
 for(i=ipk_plan.extra_count;i;i--){u32 k=i-1;
  a=ipk_plan.extra_process[k].object;tag=a+0xc8;
  if(!ipk_lexical_current(t,f)||!ipk_owner_current(k,1)||!ipk_queue_prefix(count)||
   R32((u32)&_gz_pba_native_actor_queue,4)!=tag){z=ipk_native_fail(t,IPK_PLAN_QUEUE);goto retained;}
  ipk_calling(2,k);ipk_run.cut_raw=ctp_native_step(t,f,CTP_CUT,k);ipk_run_seal();
  if(ipk_run.cut_raw!=1||R32(tag,0)||R32(tag,4)||R32(tag,8)||R32(tag,12)!=a||R8(tag,16)||
   !ipk_queue_prefix(count-1)||!ipk_lexical_current(t,f)){z=ipk_native_fail(t,IPK_PLAN_QUEUE);goto retained;}
  count--;ipk_run.cut|=1u<<k;ipk_run_seal();
 }
 if(ipk_run.paused!=all||ipk_run.cut!=all||!ipk_queue(1)){
  z=ipk_native_fail(t,IPK_PLAN_QUEUE);goto retained;}
 ipk_run.phase=IPK_PARKED;ipk_run_seal();
 z=0;
clean:end=_gz_ipk_admission_end();if(end){z=ipk_native_fail(t,end);goto retained;}
done:ipk_entered=0;return z;
retained:ipk_entered=0;return z;
}

u32 _gz_ipk_native_unpark(u32 t,u32 f){u32 i,a,tag,z,end,count,all;
 if(!ipk_plan_valid(t)||!ipk_run_valid(t)||ipk_run.error)return IPK_PLAN_PHASE;
 if(ipk_run.phase==IPK_PREPARED)return !ipk_run.paused&&!ipk_run.cut&&!ipk_run.added&&!ipk_run.unpaused?0:IPK_PLAN_PHASE;
 if(ipk_run.phase==IPK_RESTORED)return ctp_restored(t,f)?0:IPK_PLAN_PHASE;
 all=(1u<<ipk_plan.extra_count)-1u;
 if(ipk_run.phase!=IPK_PARKED||ipk_run.paused!=all||ipk_run.cut!=all||ipk_run.added||ipk_run.unpaused||
  !_gz_jwf_atomic_cas(&ipk_entered,0,1))return IPK_PLAN_PHASE;
 z=IPK_PLAN_SOURCE;if(!ipk_source_current(t,f,3))goto done;
 z=_gz_ipk_admission_begin(t,f,3);if(z)goto done;
 z=IPK_PLAN_CHANGED;
 if(!ipk_lexical_current(t,f)||!ipk_queue(1))goto clean;
 count=ipk_plan.source_count;
 for(i=0;i<ipk_plan.extra_count;i++){
  a=ipk_plan.extra_process[i].object;tag=a+0xc8;
  if(!ipk_lexical_current(t,f)||!ipk_owner_current(i,1)||!ipk_queue_prefix(count)||
   R32(tag,0)||R32(tag,4)||R32(tag,8)||R32(tag,12)!=a||R8(tag,16)){
   z=ipk_native_fail(t,IPK_PLAN_QUEUE);goto retained;}
  ipk_calling(3,i);ipk_run.add_raw=ctp_native_step(t,f,CTP_ADD,i);ipk_run_seal();
  if(!ipk_run.add_raw||!ipk_queue_prefix(count+1)||!ipk_lexical_current(t,f)){
   z=ipk_native_fail(t,IPK_PLAN_QUEUE);goto retained;}
  count++;ipk_run.added|=1u<<i;ipk_run_seal();
 }
 for(i=0;i<ipk_plan.extra_count;i++){
  a=ipk_plan.extra_process[i].object;
  if(!ipk_lexical_current(t,f)||!ipk_owner_current(i,1)){
   z=ipk_native_fail(t,IPK_PLAN_CHANGED);goto retained;}
  ipk_calling(4,i);ipk_run.disable_raw=ctp_native_step(t,f,CTP_UNPAUSE,i);ipk_run_seal();
  if(ipk_run.disable_raw!=1||!ipk_owner_current(i,0)||!ipk_lexical_current(t,f)){
   z=ipk_native_fail(t,IPK_PLAN_NATIVE);goto retained;}
  ipk_run.unpaused|=1u<<i;ipk_run_seal();
 }
 if(ipk_run.added!=all||ipk_run.unpaused!=all||!ipk_queue(0)||
  !btr_equal(ipk_plan.root,(const void*)&_gz_pba_native_actor_queue,12)||
  !btr_equal(ipk_plan.prefix_tag,(const void*)ipk_plan.prefix_node,20)||!ipk_source_current(t,f,3)){
  z=ipk_native_fail(t,IPK_PLAN_CHANGED);goto retained;}
 for(i=0;i<ipk_plan.extra_count;i++)if(!btr_equal(ipk_plan.tag[i],(const void*)(ipk_plan.extra_process[i].object+0xc8),20)){
  z=ipk_native_fail(t,IPK_PLAN_QUEUE);goto retained;}
 if(!ctp_restored(t,f)){z=ipk_native_fail(t,CTP_ERROR);goto retained;}
 ipk_run.phase=IPK_RESTORED;ipk_run.call_kind=ipk_run.call_index=0;ipk_run_seal();z=0;
clean:end=_gz_ipk_admission_end();if(end){z=ipk_native_fail(t,end);goto retained;}
done:ipk_entered=0;return z;
retained:ipk_entered=0;return z;
}
