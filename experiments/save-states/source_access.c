/* A source view is either a standalone immutable READ or the actual installed
 * source held by a CLEAN/OBSERVE operation. Never borrow/release the other's
 * pin. Kind plus service serial prevents a view surviving service boundaries
 * even when the two independent token counters happen to have equal values. */
u32 _gz_srd_kind(u32 token,u32 frame){
 if(bsr_live(token,frame))return 1;
 if(bsr_blocked())return 0;
 return pfb_readable(token,frame)?2:0;
}
u32 _gz_srd_serial(u32 token,u32 frame){u32 k=_gz_srd_kind(token,frame);
 return k==1?_gz_bsr_state.pin:k==2?_gz_roh_owner[13]:0;
}
static u32 srd_component(u32 token,u32 frame,u32 index){u32 kind,serial,p,n,z;
 kind=_gz_srd_kind(token,frame);if(!kind)return 0;
 if(kind==1)return bsr_component(token,frame,index);
 serial=_gz_srd_serial(token,frame);if(!serial)return 0;
 p=_gz_osa_source(_gz_som_pin);n=_gz_osa_bytes(_gz_som_pin);
 if(!p||n<216)return 0;
 /* Validate saved directory/World/typed data and its installed working copy.
  * All saved pointers remain data. These readers grant no actor writes. */
 z=index==26?_gz_ckb_archive_validate(p,n,1):index==25?_gz_hrp_archive_validate(p,n,1):index==24?_gz_b294m_archive_validate(p,n,1):index==23?_gz_lgb_archive_validate(p,n,1):_gz_bcb_archive_validate(p,n,1);
 if(z||index>=((volatile const u32*)p)[3]||_gz_srd_kind(token,frame)!=2||_gz_srd_serial(token,frame)!=serial)return 0;
 return p+((const volatile u32*)p)[4+index*2];
}
u32 _gz_srd_read_world(u32 token,u32 frame){return srd_component(token,frame,19);}
u32 _gz_srd_read_bank(u32 token,u32 frame){return srd_component(token,frame,22);}
u32 _gz_srd_read_layers(u32 token,u32 frame){return srd_component(token,frame,23);}

u32 _gz_srd_read_bombs(u32 token,u32 frame){return srd_component(token,frame,24);}

u32 _gz_srd_read_player(u32 token,u32 frame){return srd_component(token,frame,3);}

u32 _gz_srd_read_grapple(u32 token,u32 frame){return srd_component(token,frame,25);}

u32 _gz_srd_read_clock(u32 token,u32 frame){return srd_component(token,frame,26);}
