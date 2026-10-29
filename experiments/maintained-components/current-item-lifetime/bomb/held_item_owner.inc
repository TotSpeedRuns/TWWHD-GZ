/* A returned private item operation can release only after native retirement,
 * exact original topology, before-images and the unchanged real pin agree. */
extern u32 _gz_hif_finish_scope(u32,u32),_gz_som_item_finish_check(u32);
extern void _gz_hif_finish_leaf(void);
u32 _gz_hif_owner_retired[64];
u32 _gz_roh_item_roundtrip_finish(u32 t,u32 f){u32 i,h,pin,committed;
 if(!owned(t)||!_gz_roh_owner.frame_open||_gz_roh_owner.hold!=1||_gz_roh_owner.mutated!=1||
  f!=_gz_roh_live_frame||_gz_roh_owner.service_calls<2||_gz_roh_owner.service_calls>60||!all_quiet()||
  !((_gz_roh_owner.phase==ROH_MUTATING&&!_gz_roh_owner.error&&!_gz_roh_owner.inflight&&f==_gz_roh_owner.last_frame)||
    (_gz_roh_owner.phase==ROH_FAILED_DIRTY&&(_gz_roh_owner.error==990||_gz_roh_owner.error==991||_gz_roh_owner.error==992)&&(_gz_roh_owner.inflight==9))||
    (_gz_roh_owner.phase==ROH_FAILED_DIRTY&&_gz_hit_retired_service_scope(t)))||
  (_gz_roh_owner.error?_gz_roh_recovery_baseline(t,f)!=0:!recovery_context())||_gz_hif_finish_scope(t,f)!=1)return ROH_OWNER;
 h=hash((u32)&_gz_roh_owner,sizeof(RohOwner));pin=_gz_osa_pin_id();
 if(!pin||_gz_som_item_finish_check(t)||(_gz_roh_owner.error?_gz_roh_recovery_baseline(t,f)!=0:!recovery_context())||_gz_hif_finish_scope(t,f)!=1||
  h!=hash((u32)&_gz_roh_owner,sizeof(RohOwner))||pin!=_gz_osa_pin_id())return ROH_OWNER;
 _gz_hif_owner_retired[0]=0x48494f31u;_gz_hif_owner_retired[1]=256;
 _gz_hif_owner_retired[2]=t;_gz_hif_owner_retired[3]=h;
 for(i=0;i<60;i++)_gz_hif_owner_retired[4+i]=((const u32*)&_gz_roh_owner)[i];
 committed=_gz_roh_owner.committed_token;
 _gz_som_recovery_finish_leaf();_gz_hif_finish_leaf();_gz_rll_recovery_finish_leaf();
 for(i=0;i<sizeof(roh_recovery)/4;i++)((u32*)&roh_recovery)[i]=0;roh_recovery_seal=0;
 for(i=3;i<60;i++)if(i!=4)((u32*)&_gz_roh_owner)[i]=0;
 _gz_roh_owner.committed_token=committed;__asm__ volatile("sync":::"memory");_gz_roh_owner.hold=0;return 0;
}
