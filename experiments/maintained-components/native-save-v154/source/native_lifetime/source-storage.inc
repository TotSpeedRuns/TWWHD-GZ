/* Included after the directory and payload implementations. Allocation is
 * outside the common metadata lock; no native actor/heap bytes are restored.
 * The real SDK allocation remains owned until process exit. */
extern u32 MEMGetBaseHeapHandle(u32),MEMAllocFromExpHeapEx(u32,u32,int);
extern u32 _gz_roh_user_guard(void),_gz_ois_recovery_pending(void),_gz_aj_recovery_pending(void);
/* Exact owned candidate only. Other writers remain excluded even before
 * publication. This check uses the same metadata lock and exclusions as the
 * general private-span query, without rejecting the candidate against itself. */
static u32 scs_candidate_private(u32 p){u32 i,ok=0;HlHeap*h;
 if(!sc_static_private(p,sizeof(ScDirectory))||!hr_private_base(p,sizeof(ScDirectory),0)||
  !hr_apart(p,sizeof(ScDirectory),(u32)&_gz_rd_report,sizeof(_gz_rd_report))||!lock())return 0;
 if(!hr_healthy())goto done;
 for(i=0;i<HL_HEAPS;i++){h=_gz_hl_heaps+i;
  if(h->heap&&(h->state==HL_LIVE||h->state==HL_MUTATING||h->state==HL_ENDING)&&
   !hr_private_base(p,sizeof(ScDirectory),h))goto done;
 }
 ok=1;
done:unlock();return ok;
}
void _gz_sc_storage_tick(void){u32 p,h;ScHeader*header;
 if(_gz_sc_storage||_gz_scs_report[3]>=2)return;
 if(!__sync_bool_compare_and_swap(&scs_busy,0,1))return;
 if(_gz_roh_user_guard()||_gz_ois_recovery_pending()||_gz_aj_recovery_pending())goto done;
 if(!_gz_scs_report[3]){
  h=MEMGetBaseHeapHandle(1);_gz_scs_report[7]=h;
  if(!h){_gz_scs_report[4]=1;_gz_scs_report[3]=2;goto done;}
  _gz_scs_report[5]=sizeof(ScDirectory);_gz_scs_report[8]++;
  p=MEMAllocFromExpHeapEx(h,sizeof(ScDirectory),256);_gz_scs_report[6]=p;
  if(!p||!span(p,sizeof(ScDirectory))||(p&255)){
   _gz_scs_report[4]=2;_gz_scs_report[3]=2;goto done;
  }
  scs_seal=scs_hash();_gz_scs_report[3]=1;
 }
 if(scs_seal!=scs_hash()||_gz_scs_report[5]!=sizeof(ScDirectory)||_gz_scs_report[8]!=1){
  _gz_scs_report[4]=3;_gz_scs_report[3]=2;goto done;
 }
 p=_gz_scs_report[6];
 /* Contention is a retry, not another allocation. Private-span validation
  * scans the actual observed native heaps under the shared metadata lock. */
 if(!_gz_image_write_allowed(p,sizeof(ScDirectory))||!scs_candidate_private(p))goto done;
 header=(ScHeader*)p;zero(header,sizeof(*header));
 header->magic=0x53434431u;header->bytes=sizeof(ScDirectory);header->version=1;header->missing=31;
 /* Each capture clears its header and writes every referenced row before
  * publication. Unused capacity is never inspected or serialized. */
 _gz_scs_report[9]=_gz_hl_frame;_gz_scs_report[10]=1;_gz_scs_report[3]=3;
 __sync_synchronize();_gz_sc_storage=(ScDirectory*)p;
done:
 __sync_synchronize();scs_busy=0;
}
