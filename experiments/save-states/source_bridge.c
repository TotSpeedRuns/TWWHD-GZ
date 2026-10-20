/* Bind the actual current main owner/installer to native observed slot memory.
 * Source ownership is not native actor/resource or whole-world authority. */
typedef unsigned int u32;
extern volatile u32 _gz_roh_owner[60],_gz_roh_serial,_gz_slots_ptrs[10];
u32 _gz_osa_acquire(u32),_gz_osa_validate(u32),_gz_osa_pin_id(void),_gz_osa_source(u32),_gz_osa_bytes(u32);
void _gz_osa_release_leaf(void);
extern u32 _gz_som_begin_previous(u32,u32),_gz_som_sample_previous(u32,u32,u32);
extern u32 _gz_som_abort_previous(u32),_gz_som_guard_previous(void),_gz_som_work_previous(void),_gz_som_install_previous(u32);
extern u32 _gz_roh_fail(u32,u32);
extern u32 _gz_pfb_archive_validate(u32,u32,u32);
extern u32 _gz_bcb_archive_validate(u32,u32,u32);
extern u32 _gz_lgb_archive_validate(u32,u32,u32);
extern u32 _gz_b294m_archive_validate(u32,u32,u32);
static u32 _gz_som_active,_gz_som_pin,_gz_som_slot,_gz_som_token,_gz_som_fatal;
static u32 bsr_blocked(void);
extern volatile u32 _gz_route_records[],_gz_roh_registry_revision;
typedef struct {u32 valid,slot,source,identity,generation,bytes,revision;} SomInstalled;
static SomInstalled _gz_som_installed;
static void som_uninstall(void){_gz_som_installed.valid=0;}
static u32 som_descriptor(u32 p){u32 i,o=((const volatile u32*)p)[4];
 if(((const volatile u32*)p)[5]!=20)return 0;
 for(i=0;i<4;i++)if(_gz_route_records[680*5+i]!=((const volatile u32*)(p+o))[i])return 0;
 return 1;
}
static u32 som_installed(u32 slot,u32 p,u32 n){const SomInstalled*s=&_gz_som_installed;const OsaPin*k=&_gz_osa_state.pin;
 return s->valid==1&&s->slot==slot&&s->source==p&&s->bytes==n&&s->identity==k->identity&&s->generation==k->generation&&
  s->revision==_gz_roh_registry_revision&&som_descriptor(p);
}
static u32 som_idle(void){const volatile u32*o=_gz_roh_owner;
 return o[0]==0x524f4831u&&o[1]==240&&o[2]==2&&!o[3]&&!o[4]&&!o[5]&&!o[6]&&!o[12];}
static u32 som_live(u32 slot){u32 p,n;
 if(_gz_som_active!=1||_gz_som_fatal||slot!=_gz_som_slot||slot>9||_gz_osa_validate(_gz_som_pin))return 0;
 if(_gz_som_token&&(_gz_roh_owner[5]!=_gz_som_token||_gz_roh_serial!=_gz_som_token||_gz_roh_owner[4]!=1))return 0;
 p=_gz_osa_source(_gz_som_pin);n=_gz_osa_bytes(_gz_som_pin);
 /* Only now is even this header size word allowed to be read. The complete
  * used extent was issued from the actual allocator/writer metadata first. */
 return p&&n>=216&&_gz_slots_ptrs[slot]==p&&((volatile const u32*)p)[1]==n&&som_installed(slot,p,n);
}
static void som_release(void){_gz_osa_release_leaf();_gz_som_pin=_gz_som_slot=_gz_som_token=0;_gz_som_active=0;}
u32 _gz_roh_user_guard(void){return bsr_blocked()||_gz_som_active||_gz_som_fatal?5:_gz_som_guard_previous();}
u32 _gz_roh_work_replaced(void){
 if(bsr_blocked()||_gz_som_active||_gz_som_fatal)return 5;
 som_uninstall();return _gz_som_work_previous();
}
u32 _gz_romain_owner_sample(u32 slot,u32 out,u32 deep){u32 z,p,n;
 if(!som_live(slot))return 42;
 p=_gz_osa_source(_gz_som_pin);n=_gz_osa_bytes(_gz_som_pin);
 z=_gz_pfb_archive_validate(p,n,deep);if(z)return z;
 z=_gz_bcb_archive_validate(p,n,deep);if(z)return z;
 z=_gz_lgb_archive_validate(p,n,deep);if(z)return z;
 z=_gz_b294m_archive_validate(p,n,deep);if(z)return z;
 z=_gz_som_sample_previous(slot,out,deep);
 if(!z&&(!som_live(slot)||((volatile const u32*)out)[0]!=slot||((volatile const u32*)out)[1]!=p||((volatile const u32*)out)[2]!=n))return 42;
 return z;
}
u32 _gz_roh_begin(u32 slot,u32 frames){u32 z;
 if(bsr_blocked()||_gz_som_active||_gz_som_fatal||_gz_som_guard_previous())return 5;
 z=_gz_osa_acquire(slot);if(z)return z;
 _gz_som_active=1;_gz_som_slot=slot;_gz_som_pin=_gz_osa_pin_id();_gz_som_token=0;
 z=_gz_som_begin_previous(slot,frames);
 if(z){if(som_idle()&&_gz_osa_pin_id()==_gz_som_pin)som_release();else _gz_som_fatal=1;return z;}
 if(!som_live(slot)||_gz_roh_owner[3]!=1||_gz_roh_owner[4]!=1||!_gz_roh_owner[5]||_gz_roh_owner[5]!=_gz_roh_serial||
  _gz_roh_owner[16]!=slot||_gz_roh_owner[17]!=_gz_osa_source(_gz_som_pin)||_gz_roh_owner[18]!=_gz_osa_bytes(_gz_som_pin)){
  _gz_som_fatal=1;_gz_roh_fail(_gz_roh_owner[5],42);return 42;
 }
 _gz_som_token=_gz_roh_owner[5];return 0;
}
u32 _gz_roh_abort_clean(u32 token){u32 z;
 if(_gz_som_active!=1||_gz_som_fatal||!token||token!=_gz_som_token||_gz_osa_pin_id()!=_gz_som_pin)return 5;
 z=_gz_som_abort_previous(token);if(z)return z;
 if(!som_idle()){_gz_som_fatal=1;return 42;}
 /* Original abort has finished its final source read. The source pin and
  * this separate entry exclusion remain held through its owner release. */
 som_release();return 0;
}
/* Recovery checks allocation/slot metadata only. A damaged source body must
 * not prevent retiring an operation whose native writes were all undone. */
extern u32 _gz_bmh_aborted_scope(u32);
u32 _gz_som_recovery_finish_check(u32 token){const volatile u32*o=_gz_roh_owner;
 const SomInstalled*s=&_gz_som_installed;const OsaPin*p=&_gz_osa_state.pin;
 if(bsr_blocked()||_gz_som_active!=1||_gz_som_fatal||!token||token!=_gz_som_token||
  _gz_som_slot>9||_gz_osa_validate(_gz_som_pin)||p->serial!=_gz_som_pin||
  o[0]!=0x524f4831u||o[1]!=240||o[2]!=2||o[3]!=4||o[4]!=1||o[5]!=token||o[6]!=1||!o[7]||
  (o[12]&&_gz_bmh_aborted_scope(token)!=1))return 42;
 if(s->valid!=1||s->slot!=_gz_som_slot||p->slot!=s->slot||o[16]!=s->slot||
  s->source!=p->allocation||s->source!=o[17]||s->bytes!=p->bytes||s->bytes!=o[18]||
  s->identity!=p->identity||s->generation!=p->generation||s->revision!=_gz_roh_registry_revision||s->revision!=o[26])return 42;
 return 0;
}
/* Private nonfallible tail: caller has completed the final same-frame checks.
 * Release the read pin, retain the slot allocation, and require a new install. */
void _gz_som_recovery_finish_leaf(void){som_uninstall();som_release();}
u32 _gz_slots_install(u32 slot){u32 z,pin;
 if(bsr_blocked()||_gz_som_active||_gz_som_fatal||_gz_som_guard_previous())return 5;
 z=_gz_osa_acquire(slot);if(z)return z;pin=_gz_osa_pin_id();
 z=_gz_pfb_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),0);
 if(!z)z=_gz_bcb_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),0);
 if(!z)z=_gz_lgb_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),0);
 if(!z)z=_gz_b294m_archive_validate(_gz_osa_source(pin),_gz_osa_bytes(pin),0);
 if(!z)z=_gz_som_install_previous(slot);
 if(_gz_osa_pin_id()!=pin){_gz_som_fatal=1;return 42;}
 if(!z&&_gz_osa_validate(pin))z=42;
 if(!z){u32 p=_gz_osa_source(pin),n=_gz_osa_bytes(pin);
  if(!p||n<216||!som_descriptor(p))z=42;
  else{_gz_som_installed.slot=slot;_gz_som_installed.source=p;_gz_som_installed.bytes=n;
   _gz_som_installed.identity=_gz_osa_state.pin.identity;_gz_som_installed.generation=_gz_osa_state.pin.generation;
   _gz_som_installed.revision=_gz_roh_registry_revision;
   __asm__ volatile("sync":::"memory");_gz_som_installed.valid=1;
  }
 }
 if(z)som_uninstall();
 /* The previous installer copied to the separate main working buffers. No
  * source pointer escapes this pin as a new permission to read later. */
 _gz_osa_release_leaf();return z;
}

/* Read-only PF2 source receipt. Exact selected-source ownership and the old
 * owner service remain authoritative. No lifetime/collision/write capability
 * is granted by successfully reading a serialized packet. */
extern u32 _gz_roh_pot_authority(u32,u32,u32,u32,u32);
extern volatile u32 _gz_roh_live_frame;
static u32 pfb_readable(u32 token,u32 frame){const volatile u32*o=_gz_roh_owner;
 if(!token||_gz_som_token!=token||_gz_som_active!=1||_gz_som_fatal||
  o[0]!=0x524f4831u||o[1]!=240||o[2]!=2||o[3]!=1||o[4]!=1||o[5]!=token||o[6]||o[7]||
  !o[11]||o[12]!=1||o[10]!=frame||frame!=_gz_roh_live_frame||!som_live(_gz_som_slot))return 0;
 return _gz_roh_pot_authority(token,o[54],o[55],frame,0)==1;
}
u32 _gz_pfb_read_bank(u32 token,u32 frame){u32 p,n;
 if(!pfb_readable(token,frame))return 0;
 p=_gz_osa_source(_gz_som_pin);n=_gz_osa_bytes(_gz_som_pin);
 if(!p||!n||_gz_pfb_archive_validate(p,n,1)||!pfb_readable(token,frame))return 0;
 return p+((const volatile u32*)p)[46];
}
u32 _gz_pfb_read_member(u32 token,u32 packet,u32 frame){u32 p,n,d;
 p=_gz_pfb_read_bank(token,frame);if(!p||packet<p+64)return 0;
 n=((const volatile u32*)p)[4];d=packet-(p+64);
 return !(d&2047u)&&d/2048u<n?1:0;
}

/* Read-only original-request records from the actual selected, installed slot.
 * Returned addresses expire with this callback/frame and source pin. They do
 * not make saved native pointers callable or grant a lifetime/replay lease. */
u32 _gz_bcb_read_bank(u32 token,u32 frame){u32 p,n;
 if(!pfb_readable(token,frame))return 0;
 p=_gz_osa_source(_gz_som_pin);n=_gz_osa_bytes(_gz_som_pin);
 if(!p||!n||_gz_bcb_archive_validate(p,n,1)||!pfb_readable(token,frame))return 0;
 return p+((const volatile u32*)p)[48];
}
u32 _gz_bcb_read_member(u32 token,u32 index,u32 frame){u32 p=_gz_bcb_read_bank(token,frame);
 return p&&index<((const volatile u32*)p)[4]?p+96+index*192:0;
}
