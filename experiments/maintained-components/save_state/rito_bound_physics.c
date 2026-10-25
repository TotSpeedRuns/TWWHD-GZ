/* A private projected packet refers only to freshly qualified native BGs.
 * This does not require the actor's mutable last-query cache to equal the
 * target. Before/target links are journaled separately and written only under
 * the existing retained whole-world execution/heap/resource hold.
 */
static u32 rph_bg_ref(u32 off){u32 i;
 if(off==0x4c4||off==0x4c8||off==0x4cc)return 1;
 for(i=0;i<4;i++)if(off==nph_poly[i]+4u||off==nph_poly[i]+8u)return 1;return 0;}
static void rph_set_ref(u32 p,u32 off,u32 v){u32 i;for(i=0;i<nph_ref_count();i++)if(nph_refs[i]==off){*(u32*)(p+RPH_HEADER+4*i)=v;return;}}
static u32 rph_bound_links(u32 a,u32 p){u32 i,w,index,object,pid;BgEntry bg;const RwmMeta*m;
 for(i=0;i<nph_ref_count();i++)if(!rph_bg_ref(nph_refs[i])&&R32(a,nph_refs[i])!=R32(p,RPH_HEADER+4*i))return RPH_CHANGED;
 for(i=0;i<4;i++){
  w=R32(p,RPH_META+16*i);object=rph_ref(p,nph_poly[i]+4);pid=rph_ref(p,nph_poly[i]+8);
  if(w==0xffff0100u){if(object||pid!=0xffffffffu)return RPH_CHANGED;continue;}
  index=w&65535;
  if(_gz_bg_lookup(index)||!_gz_bg_observation.valid)return RPH_CHANGED;btr_copy(&bg,&_gz_bg_observation.entry,sizeof(bg));
  if(bg.object!=object||bg.pid!=pid||bg.generation!=R32(p,RPH_META+16*i+4)||bg.owner_generation!=R32(p,RPH_META+16*i+8)||
   bg.resource_signature!=R32(p,RPH_META+16*i+12)||!epj_ram(bg.resource,0x30)||(w>>16)>=R32(bg.resource,8))return RPH_CHANGED;
 }
 index=rph_ref(p,0x4c4);object=rph_ref(p,0x4c8);pid=rph_ref(p,0x4cc);m=(const RwmMeta*)(p+RPH_USED);
 if(!index&&!object&&!pid)return rph_wall_record(p)?0:RPH_CHANGED;
 if(_gz_bg_lookup(index)||!_gz_bg_observation.valid)return RPH_CHANGED;btr_copy(&bg,&_gz_bg_observation.entry,sizeof(bg));
 return bg.object==object&&bg.pid==pid&&bg.generation==m->generation&&bg.owner_generation==m->owner_generation&&bg.epoch==m->epoch&&
  bg.bgw_vtable==m->vtable&&bg.resource==m->resource&&bg.resource_signature==m->signature&&epj_ram(bg.resource,0x30)&&R32(bg.resource,8)==m->triangles?0:RPH_CHANGED;
}
static u32 rph_bound_join(const PoEntry*process,u32 p){u32 z=rph_identity(process,p,1);
 return z?z:R32(p,8)==1?(rph_refs_match(process->object,p)?0:RPH_CHANGED):rph_bound_links(process->object,p);}
static u32 rpj_values(u32 a,u32 p,u32 write){u32 i,j,w,tuple[3],z=rph_values(a,p,write);if(z)return z;
 for(i=0;i<4;i++){
  tuple[0]=R32(p,RPH_META+16*i);tuple[1]=rph_ref(p,nph_poly[i]+4);tuple[2]=rph_ref(p,nph_poly[i]+8);
  for(j=0;j<3;j++){w=a+nph_poly[i]+4*j;if(write)*(volatile u32*)w=tuple[j];else if(R32(w,0)!=tuple[j])return RPH_CHANGED;}
 }
 tuple[0]=rph_ref(p,0x4c4);tuple[1]=rph_ref(p,0x4c8);tuple[2]=rph_ref(p,0x4cc);
 for(j=0;j<3;j++){w=a+0x4c4+4*j;if(write)*(volatile u32*)w=tuple[j];else if(R32(w,0)!=tuple[j])return RPH_CHANGED;}
 return 0;
}
