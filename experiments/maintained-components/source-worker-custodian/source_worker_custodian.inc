/* Same actual SO/OSA TU. One real acquisition; never a copied/adopted fake pin. */
extern u32 _gz_hg_platform_thread(void);
extern u32 _gz_wcm_admission_held(u32,u32,u32);
static u32 swc_install_held(u32),swc_roh_begin_held(u32,u32);

static void swc_seal(void){swc.seal=hash(&swc,sizeof(swc)-4);__asm__ volatile("sync":::"memory");}
static u32 swc_pending(void){const u32*p=(const u32*)&swc;u32 i;
 for(i=0;i<sizeof(swc)/4;i++)if(p[i])return 1;return 0;}
u32 _gz_swc_pending(void){return swc_pending();}
static u32 swc_valid(void){return swc.phase>=SWC_READY&&swc.phase<=SWC_HALTED&&
 swc.key.request&&swc.key.frame&&swc.key.slot<10&&swc.key.thread&&swc.key.parent&&
 swc.pin&&swc.source&&swc.used&&swc.capacity&&swc.identity&&swc.generation&&swc.revision&&
 swc.work_count<=SWC_WORK_BFLOWER&&swc.seal==hash(&swc,sizeof(swc)-4);}
static u32 swc_fail(void){if(!swc.error)swc.error=SWC_RETAINED;
 swc.phase=SWC_HALTED;swc_seal();return SWC_RETAINED;}
static void swc_release_refused(void){(void)swc_fail();}
static u32 swc_context(u32 request,u32 frame,u32 slot){GzSwcContext now;
 now.request=request;now.frame=frame;now.slot=slot;now.thread=_gz_hg_platform_thread();
 now.parent=_gz_wct_source_worker_context(request,frame,slot);
 return gz_swc_context_same(&swc.key,&now);
}
/* Full actual OSA/SO metadata, raw capacity and sealed Row digest stay fresh.
 * The private snapshot cannot make a missing/changed real lease valid. */
static u32 swc_view(void){const OsaPin*p=&_gz_osa_state.pin;
 return swc_valid()&&!swc.error&&swc.phase!=SWC_HALTED&&
  !_gz_osa_validate(swc.pin)&&p->active==1&&p->serial==swc.pin&&p->slot==swc.key.slot&&
  p->allocation==swc.source&&p->bytes==swc.used&&p->identity==swc.identity&&
  p->generation==swc.generation&&so_capacity(swc.pin)==swc.capacity&&
  so_raw_hash(swc.pin)==swc.raw_hash&&owner.rows[swc.key.slot].seal==swc.row_seal&&so_raw_match(swc.pin,swc.key.slot,swc.source,swc.used);
}
static u32 swc_current(u32 request,u32 frame,u32 slot){return swc_view()&&
 swc_context(request,frame,slot)&&swc.revision==_gz_roh_registry_revision;}
static u32 swc_idle(void){return !bsr_blocked()&&!_gz_som_active&&!_gz_som_fatal&&
 som_idle()&&!_gz_som_guard_previous();}
static u32 swc_stage(u32 request,u32 frame,u32 slot,u32 stage){return
 swc_current(request,frame,slot)&&gz_swc_stage(swc.phase,swc.borrowed,stage)&&swc_idle();}
u32 _gz_swc_admission_pin(u32 request,u32 frame,u32 slot){return
 swc_current(request,frame,slot)&&swc.phase==SWC_READY&&swc.borrowed==SWC_ADMISSION&&swc_idle()?swc.pin:0;}
u32 _gz_swc_begin(u32 request,u32 frame,u32 slot){GzSwcContext key;u32 z,pin;
 if(swc_pending()||!swc_idle()||_gz_osa_pin_id()||owner.entered||owner.writer||owner.lease.active)return 5;
 key.request=request;key.frame=frame;key.slot=slot;key.thread=_gz_hg_platform_thread();
 key.parent=_gz_wct_source_worker_context(request,frame,slot);
 if(!gz_swc_context_same(&key,&key)||!_gz_roh_registry_revision)return 42;
 z=_gz_osa_acquire(slot);if(z)return z;pin=_gz_osa_pin_id();
 swc.key=key;swc.phase=SWC_READY;swc.pin=pin;swc.source=_gz_osa_source(pin);
 swc.used=_gz_osa_bytes(pin);swc.capacity=so_capacity(pin);swc.identity=_gz_osa_pin_identity(pin);
 swc.generation=_gz_osa_pin_generation(pin);swc.raw_hash=so_raw_hash(pin);swc.row_seal=owner.rows[slot].seal;swc.revision=_gz_roh_registry_revision;swc_seal();
 return swc_stage(request,frame,slot,SWC_ADMISSION)?0:swc_fail();
}
u32 _gz_swc_admission(u32 request,u32 frame,u32 slot){u32 z;
 if(!swc_stage(request,frame,slot,SWC_ADMISSION))return swc_fail();
 swc.borrowed=SWC_ADMISSION;swc_seal();
 z=_gz_wcm_admission_held(request,frame,slot);
 if(!swc_current(request,frame,slot)||swc.borrowed!=SWC_ADMISSION||swc.phase!=SWC_READY||!swc_idle())return swc_fail();
 swc.borrowed=0;if(!z)swc.phase=SWC_ADMITTED;swc_seal();return z;
}
/* The retained slot, MCA and bflower wrappers each invoke the original
 * callback. Fixed entry purposes authenticate their order; every real call
 * must witness its own exact registry increment under the same custody. */
static u32 swc_work_replaced(u32 purpose){u32 previous,count,z;
 if(!swc_current(swc.key.request,swc.key.frame,swc.key.slot)||!swc_idle()||
  swc.phase!=SWC_ADMITTED||swc.borrowed!=SWC_INSTALL||
  !gz_swc_work_next(swc.work_count,purpose)||swc.revision==0xffffffffu)return SWC_RETAINED;
 previous=swc.revision;count=swc.work_count;
 som_uninstall();z=_gz_som_work_previous();
 /* The revision is deliberately still the old sealed value at this point.
  * Recheck all other actual lease/context fields before accepting a change. */
 if(!swc_view()||!swc_context(swc.key.request,swc.key.frame,swc.key.slot)||!swc_idle()||
  swc.phase!=SWC_ADMITTED||swc.borrowed!=SWC_INSTALL||
  swc.revision!=previous||swc.work_count!=count)return swc_fail();
 if(z){if(_gz_roh_registry_revision!=previous)return swc_fail();return z;}
 if(_gz_roh_registry_revision!=previous+1)return swc_fail();
 swc.revision=_gz_roh_registry_revision;swc.work_count=purpose;swc_seal();return 0;
}
/* Normal C ABI, no supplied context, pointer, revision or native result.
 * Outside custody each entry preserves the existing generic operation. */
u32 _gz_swc_slot_work_replaced(void){return swc_pending()?swc_work_replaced(SWC_WORK_SLOT):_gz_roh_work_replaced();}
u32 _gz_swc_mca_work_replaced(void){return swc_pending()?swc_work_replaced(SWC_WORK_MCA):_gz_roh_work_replaced();}
u32 _gz_swc_bflower_work_replaced(void){return swc_pending()?swc_work_replaced(SWC_WORK_BFLOWER):_gz_roh_work_replaced();}
u32 _gz_swc_install(u32 request,u32 frame,u32 slot){u32 z;
 if(!swc_stage(request,frame,slot,SWC_INSTALL)||swc.work_count)return swc_fail();
 swc.borrowed=SWC_INSTALL;swc_seal();z=swc_install_held(slot);
 if(!swc_current(request,frame,slot)||swc.borrowed!=SWC_INSTALL||swc.phase!=SWC_ADMITTED||!swc_idle())return swc_fail();
 if(!z&&(swc.work_count!=SWC_WORK_BFLOWER||!som_installed(slot,swc.source,swc.used)))return swc_fail();
 swc.borrowed=0;if(!z)swc.phase=SWC_INSTALLED;swc_seal();return z;
}
u32 _gz_swc_roh_begin(u32 request,u32 frame,u32 slot,u32 frames){u32 z;
 if(!swc_stage(request,frame,slot,SWC_ADOPT)||swc.work_count!=SWC_WORK_BFLOWER)return swc_fail();
 swc.borrowed=SWC_ADOPT;swc_seal();z=swc_roh_begin_held(slot,frames);
 if(z){
  if(!swc_current(request,frame,slot)||!swc_idle()||swc.borrowed!=SWC_ADOPT||swc.phase!=SWC_INSTALLED)return swc_fail();
  swc.borrowed=0;swc_seal();return z;
 }
 if(!swc_current(request,frame,slot)||swc.borrowed!=SWC_ADOPT||swc.phase!=SWC_INSTALLED||
  !som_live(slot)||_gz_som_active!=1||_gz_som_pin!=swc.pin||_gz_som_slot!=slot||
  !_gz_som_token||_gz_som_token!=_gz_roh_owner[5]||_gz_som_token!=_gz_roh_serial||
  _gz_roh_owner[3]!=1||_gz_roh_owner[4]!=1||_gz_roh_owner[16]!=slot||
  _gz_roh_owner[17]!=swc.source||_gz_roh_owner[18]!=swc.used)return swc_fail();
 /* Exact actual pin custody transfers to original SOM. No release/reacquire,
  * pin copy/renewal or source access remains in this custodian afterward. */
 zero(&swc,sizeof(swc));return 0;
}
u32 _gz_swc_cancel(u32 request,u32 frame,u32 slot){
 if(!swc_pending())return 0;
 if(!swc_current(request,frame,slot)||swc.borrowed||!swc_idle())return swc_fail();
 /* The old leaf is private, nonfallible and does no body/free/callback work.
  * Only this checked clean cancellation may end a preadoption custody pin. */
 osa_release_owned_leaf();zero(&swc,sizeof(swc));return 0;
}

static u32 swc_install_held(u32 slot){u32 z,pin;
 if(bsr_blocked()||_gz_som_active||_gz_som_fatal||_gz_som_guard_previous())return 5;
 if(!swc_current(swc.key.request,swc.key.frame,slot)||swc.phase!=SWC_ADMITTED||swc.borrowed!=SWC_INSTALL)return SWC_RETAINED;
 pin=swc.pin;
 z=_gz_pfb_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),0);
 if(!z)z=_gz_bcb_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),0);
 if(!z)z=_gz_lgb_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),0);
 if(!z)z=_gz_b294m_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),0);
 if(!z)z=_gz_ncb_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),0);
 if(!z)z=_gz_som_install_previous(slot);
 if(!z)z=_gz_ncb_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),1);
 if(_gz_osa_pin_id()!=pin){_gz_som_fatal=1;return 42;}
 if(!z&&_gz_osa_validate(pin))z=42;
 if(!z){u32 p=_gz_osa_source(pin),n=_gz_osa_bytes(pin);
  if(!p||n<216||!som_descriptor(p)||_gz_osa_verify_source_final(pin,p,n,
       _gz_osa_state.pin.identity,_gz_osa_state.pin.generation,_gz_roh_registry_revision))z=42;
  else{_gz_som_installed.slot=slot;_gz_som_installed.source=p;_gz_som_installed.bytes=n;
   _gz_som_installed.identity=_gz_osa_state.pin.identity;_gz_som_installed.generation=_gz_osa_state.pin.generation;
   _gz_som_installed.hash=_gz_osa_pin_hash(pin);
   _gz_som_installed.revision=_gz_roh_registry_revision;
   __asm__ volatile("sync":::"memory");_gz_som_installed.valid=1;
  }
 }
 if(z)som_uninstall();
 /* The previous installer copied to the separate main working buffers. No
  * source pointer escapes this pin as a new permission to read later. */
 return z;
}

static u32 swc_roh_begin_held(u32 slot,u32 frames){u32 z;
 if(bsr_blocked()||_gz_som_active||_gz_som_fatal||_gz_som_guard_previous())return 5;
 if(!swc_current(swc.key.request,swc.key.frame,slot)||swc.phase!=SWC_INSTALLED||swc.borrowed!=SWC_ADOPT)return SWC_RETAINED;
 if(!som_logical_installed(slot)||_gz_som_installed.identity!=_gz_osa_pin_identity(_gz_osa_pin_id())||
  _gz_som_installed.generation!=_gz_osa_pin_generation(_gz_osa_pin_id())||
  _gz_som_installed.bytes!=_gz_osa_bytes(_gz_osa_pin_id())){return 42;}
 _gz_som_installed.source=_gz_osa_source(_gz_osa_pin_id());
 _gz_som_active=1;_gz_som_slot=slot;_gz_som_pin=_gz_osa_pin_id();_gz_som_token=0;
 z=_gz_som_begin_previous(slot,frames);
 if(z){if(som_idle()&&_gz_osa_pin_id()==_gz_som_pin){_gz_som_pin=_gz_som_slot=_gz_som_token=0;_gz_som_active=0;}else _gz_som_fatal=1;return z;}
 if(!som_live(slot)||_gz_roh_owner[3]!=1||_gz_roh_owner[4]!=1||!_gz_roh_owner[5]||_gz_roh_owner[5]!=_gz_roh_serial||
  _gz_roh_owner[16]!=slot||_gz_roh_owner[17]!=_gz_osa_source(_gz_som_pin)||_gz_roh_owner[18]!=_gz_osa_bytes(_gz_som_pin)){
  _gz_som_fatal=1;_gz_roh_fail(_gz_roh_owner[5],42);return 42;
 }
 _gz_som_token=_gz_roh_owner[5];return 0;
}
