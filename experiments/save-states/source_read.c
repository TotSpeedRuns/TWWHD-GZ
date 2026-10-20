/* Immutable saved-data READ lease. It grants no native actor lifetime, scene
 * mutation, constructor/deletion, resource or replay permission. Source room
 * identity is validated against the saved World, never against the live room.
 * All calls execute on the existing serialized nonyielding game lane. */
extern volatile u32 _gz_slots_selected;
typedef struct {
 u32 magic,bytes,version,active,token,slot,pin,source,used,frame;
 u32 identity,generation,revision,fatal,reserved0,reserved1;
} BsrState;
static BsrState _gz_bsr_state={0x4253524fu,64,1,0,0,0,0,0,0,0,0,0,0,0,0,0};
typedef char BsrStateSize[(sizeof(BsrState)==64)?1:-1];
static u32 bsr_shape(void){const BsrState*s=&_gz_bsr_state;
 return s->magic==0x4253524fu&&s->bytes==64&&s->version==1&&s->active<=1&&s->fatal<=1&&!s->reserved0&&!s->reserved1;
}
static u32 bsr_blocked(void){return !bsr_shape()||_gz_bsr_state.active||_gz_bsr_state.fatal;}
static void bsr_clear(void){u32 i;for(i=3;i<16;i++)((volatile u32*)&_gz_bsr_state)[i]=0;}
static u32 bsr_issued(void){const BsrState*s=&_gz_bsr_state;const OsaPin*p=&_gz_osa_state.pin;
 return bsr_shape()&&!s->fatal&&s->active==1&&s->token&&s->token==s->pin&&s->slot<10&&s->source&&s->used>=216&&
  !_gz_osa_validate(s->pin)&&p->slot==s->slot&&p->allocation==s->source&&p->bytes==s->used&&
  p->identity==s->identity&&p->generation==s->generation;
}
static u32 bsr_live(u32 token,u32 frame){const BsrState*s=&_gz_bsr_state;
 return token&&s->token==token&&frame&&s->frame==frame&&frame==_gz_roh_live_frame&&s->slot==_gz_slots_selected&&
  s->revision==_gz_roh_registry_revision&&!_gz_som_active&&!_gz_som_fatal&&som_idle()&&!_gz_som_guard_previous()&&bsr_issued();
}
u32 _gz_bsr_begin(u32 slot,u32 frame){BsrState*s=&_gz_bsr_state;u32 z,p,n;
 if(bsr_blocked()||_gz_som_active||_gz_som_fatal||!som_idle()||_gz_som_guard_previous())return 5;
 if(slot>9||slot!=_gz_slots_selected||!frame||frame!=_gz_roh_live_frame||!_gz_roh_registry_revision)return 42;
 z=_gz_osa_acquire(slot);if(z)return z;
 s->pin=_gz_osa_pin_id();p=_gz_osa_source(s->pin);n=_gz_osa_bytes(s->pin);
 /* Validate saved directory/World/typed packets while the real allocation pin
  * excludes every normal slot writer. Installed working copies are irrelevant
  * to read-only source planning and do not become authority through this API. */
 z=(!p||n<216)?42:_gz_bcb_archive_validate(p,n,0);
 if(!z)z=_gz_pfb_archive_validate(p,n,0);
 if(!z)z=_gz_lgb_archive_validate(p,n,0);
 if(!z)z=_gz_b294m_archive_validate(p,n,0);
 if(!z)z=_gz_hrp_archive_validate(p,n,0);
 if(!z)z=_gz_ckb_archive_validate(p,n,0);
 if(z){_gz_osa_release_leaf();bsr_clear();return z;}
 s->token=s->pin;s->slot=slot;s->source=p;s->used=n;s->frame=frame;
 s->identity=_gz_osa_state.pin.identity;s->generation=_gz_osa_state.pin.generation;s->revision=_gz_roh_registry_revision;
 __asm__ volatile("sync":::"memory");s->active=1;
 if(!bsr_live(s->token,frame)){
  if(bsr_issued()){_gz_osa_release_leaf();bsr_clear();return 42;}
  s->fatal=1;return 42;
 }
 return 0;
}
/* Token getter permits clean release after a frame/selection change. It never
 * returns a source pointer and does not extend the validity of a READ result. */
u32 _gz_bsr_token(void){return bsr_issued()?_gz_bsr_state.token:0;}
u32 _gz_bsr_end(u32 token){BsrState*s=&_gz_bsr_state;
 if(!token||s->token!=token||!bsr_issued())return 5;
 /* No source reads, callbacks or fallible work follows this final release. */
 _gz_osa_release_leaf();bsr_clear();return 0;
}
static u32 bsr_component(u32 token,u32 frame,u32 index){
 if(!bsr_live(token,frame)||index>=((volatile const u32*)_gz_bsr_state.source)[3])return 0;
 return _gz_bsr_state.source+((const volatile u32*)_gz_bsr_state.source)[4+index*2];
}
u32 _gz_bsr_read_world(u32 token,u32 frame){return bsr_component(token,frame,19);}
u32 _gz_bsr_read_pots(u32 token,u32 frame){return bsr_component(token,frame,21);}
u32 _gz_bsr_read_bank(u32 token,u32 frame){return bsr_component(token,frame,22);}
u32 _gz_bsr_read_member(u32 token,u32 index,u32 frame){u32 p=_gz_bsr_read_bank(token,frame);
 return p&&index<((const volatile u32*)p)[4]?p+96+index*192:0;
}

u32 _gz_bsr_read_layers(u32 token,u32 frame){return bsr_component(token,frame,23);}

u32 _gz_bsr_read_bombs(u32 token,u32 frame){return bsr_component(token,frame,24);}
