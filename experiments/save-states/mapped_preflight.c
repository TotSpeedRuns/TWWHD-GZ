/* Use the existing registry/identity/bomb validators on the sealed mapped
 * inputs. This is a read-only participant preflight, not world publication.
 * The native constructor and its original source/undo receipts stay intact. */
extern u32 _gz_v104_actor_registry_validate(u32);
extern u32 _gz_v104_actor_registry_validate_view(u32,const u32*,const u32*);
extern u32 _gz_identity_record_safe(u32),_gz_identity_resolve(u32,u32,u32);
extern u32 _gz_vws_source_matches(const VrWorld*,const u32*,const u32*,const u32*);
extern u32 _gz_b294m_source_matches(u32,u32,const VrWorld*);
extern volatile u32 _gz_bflower_retire_plan,_gz_item_retire_probe;
static u8 _gz_bmc_b294[16480] __attribute__((aligned(4)));
static u32 _gz_bmc_entered;
u32 _gz_bmc_report[32]={0x424d4331u,128};

static u32 bmc_b294_hash(void){u32 i,h=2166136261u;
 for(i=0;i<sizeof(_gz_bmc_b294);i++)h=(h^((i>=16&&i<20)?0:_gz_bmc_b294[i]))*16777619u;
 return h;
}

u32 _gz_bmc_preflight(u32 token){BmvViews*v;u32 z=28,i,p,scene,bank,frame=_gz_pbac_frame;
 if(_gz_bmc_entered)return 5;
 p=_gz_bmv_read(token,frame,BMV_HEADER);if(!p)return 28;
 v=(BmvViews*)p;
 _gz_bmc_entered=1;
 for(i=2;i<32;i++)_gz_bmc_report[i]=0;
 _gz_bmc_report[2]=token;_gz_bmc_report[4]=frame;_gz_bmc_report[13]=BAD;
 _gz_bmc_report[15]=v->count;
 /* Existing registry exceptions belong to different lifetime participants.
  * Never clear them or let them silently exclude a mapped actor here. */
 if(_gz_bflower_retire_plan||_gz_item_retire_probe)goto done;
 scene=bmv_component(v->source,v->source_bytes,0,20);
 bank=bmv_component(v->source,v->source_bytes,24,16480);
 if(!scene||!bank)goto done;
 z=_gz_vws_source_matches(&v->world,v->meta,v->ids,(const u32*)scene);
 _gz_bmc_report[6]=z;if(z)goto done;
 z=_gz_identity_record_safe((u32)v->graph);_gz_bmc_report[7]=z;if(z)goto done;
 /* Keep the installed source registry untouched. The old entry is sampled
  * for evidence, then the same registry algorithm runs on explicit inputs. */
 _gz_bmc_report[16]=_gz_v104_actor_registry_validate(BAD);
 z=_gz_v104_actor_registry_validate_view(BAD,v->meta,v->ids);
 _gz_bmc_report[5]=z;if(z!=1){z=27;goto done;}
 for(i=0;i<v->count;i++){
  const VrActor*w=&v->world.actors[i];
  p=_gz_identity_resolve((u32)v->graph,w->pid,0);
  _gz_bmc_report[13]=i;_gz_bmc_report[14]=p;
  if(!p||p!=w->actor||!span(p,0x300)||R32(p,4)!=w->pid||R16(p,8)!=w->profile){z=27;goto done;}
  _gz_bmc_report[8]++;
 }
 /* Retain the actual B294 archive format, checksum and World/typed-source
  * crosscheck; a naked remapped row is not a substitute for those guards. */
 btr_copy(_gz_bmc_b294,(const void*)bank,64);
 btr_copy(_gz_bmc_b294+64,&v->bombs,sizeof(PbBank));
 *(u32*)(_gz_bmc_b294+16)=bmc_b294_hash();
 z=_gz_b294m_source_matches((u32)_gz_bmc_b294,sizeof(_gz_bmc_b294),&v->world);
 _gz_bmc_report[9]=z;if(z)goto done;
 z=_gz_ibi_bank_preflight((const PbBank*)(_gz_bmc_b294+64),_gz_pba_native_actor_queue,&v->bombs.rows[0].context);
 _gz_bmc_report[10]=z;if(z)goto done;
 z=_gz_btr_final_check(token);_gz_bmc_report[11]=z;if(z)goto done;
 z=_gz_bmv_check(token);_gz_bmc_report[12]=z;if(z)goto done;
 if(frame!=_gz_pbac_frame||_gz_bflower_retire_plan||_gz_item_retire_probe){z=28;goto done;}
 _gz_bmc_report[17]=bmc_b294_hash();
done:
 _gz_bmc_report[3]=z;_gz_bmc_report[18]=_gz_pbac_frame;
 _gz_bmc_entered=0;
 return z;
}
