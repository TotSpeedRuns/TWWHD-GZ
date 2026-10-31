/* Exact OIS-owned fixed pending-cut receiver; a copied image tuple cannot
 * authenticate itself. No output, caller buffer, ticket or hold is minted. */
U _gz_ghs_held_source_matches(U req,U f,const GhcSaveProducer*p){
 if(p!=&_gz_ois_held_record.producer||!_gz_ghs_scope(req,f,2)||
  ghs.command!=GHS_COMMAND_RESTORE||ghs.issued_hg!=ghs.hg)return 0;
 return p->magic==GHS_MAGIC&&p->bytes==64&&p->version==2&&
  p->request==req&&p->frame==f&&p->epoch==ghs.epoch&&p->slot==ghs.slot&&
  p->thread==ghs.thread&&p->original_return==ghs.original_return&&
  p->service_return==ghs.service_return&&p->save_return==ghs.admission_return&&
  p->hg==ghs.hg&&p->lease==ghs.lease&&p->job_generation==ghs.job_generation&&
  p->scene_hash==ghs.scene_hash&&p->reserved==GHS_COMMAND_RESTORE&&
  _gz_ghs_scope(req,f,2);}
