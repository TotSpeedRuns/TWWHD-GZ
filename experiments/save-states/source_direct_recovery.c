/* No-lifetime rollback: metadata only, never archive contents. */
extern u32 _gz_dck_finish_scope(u32,u32);
extern volatile u32 _gz_roh_live_frame;
u32 _gz_som_direct_recovery_finish_check(u32 token){const volatile u32*o=_gz_roh_owner;
 const SomInstalled*s=&_gz_som_installed;const OsaPin*p=&_gz_osa_state.pin;
 if(bsr_blocked()||_gz_som_active!=1||_gz_som_fatal||!token||token!=_gz_som_token||
  _gz_som_slot>9||_gz_osa_validate(_gz_som_pin)||p->serial!=_gz_som_pin||
  o[0]!=0x524f4831u||o[1]!=240||o[2]!=2||o[3]!=4||o[4]!=1||o[5]!=token||o[6]!=1||!o[7]||
  ((o[12]!=5&&o[12]!=2&&o[12]!=6)||_gz_dck_finish_scope(token,_gz_roh_live_frame)!=1))return 42;
 if(s->valid!=1||s->slot!=_gz_som_slot||p->slot!=s->slot||o[16]!=s->slot||
  s->source!=p->allocation||s->source!=o[17]||s->bytes!=p->bytes||s->bytes!=o[18]||
  s->identity!=p->identity||s->generation!=p->generation||s->revision!=_gz_roh_registry_revision||s->revision!=o[26])return 42;
 return 0;
}
