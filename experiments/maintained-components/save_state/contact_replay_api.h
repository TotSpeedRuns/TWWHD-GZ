#ifndef GZ_CONTACT_REPLAY_API_H
#define GZ_CONTACT_REPLAY_API_H
typedef unsigned int u32;
enum {CGR_CAP=8192,CGR_OWNERS=512,CGR_SPACE=0x400000u,CGR_MANAGER_RESERVE=2048,CGR_CONTEXT=1180,CGR_SOURCE=1181,CGR_BIND=1182,CGR_CHANGED=1183,CGR_ALIAS=1184,CGR_PARTIAL=1188};
void _gz_cgw_tick(void);
u32 _gz_cgw_space(void),_gz_cgw_rows(void),_gz_cgw_private(u32,u32),_gz_cgw_active(void),_gz_cgw_begin(u32,u32,u32,u32,u32);
void _gz_cgw_end(u32);
u32 _gz_cgr_empty(void),_gz_cgr_sealed(u32),_gz_cgr_prepare(u32,u32),_gz_cgr_apply(u32,u32,u32),_gz_cgr_undo(u32,u32),_gz_cgr_before(u32,u32),_gz_cgr_forget(u32);
void _gz_cgr_finish(u32);
u32 _gz_cgr_prepared_before(u32,u32);
u32 _gz_cgr_target_check(u32,u32);
u32 _gz_cgr_pot_packet(u32,u32,u32,u32,u32,u32),_gz_cgr_span_count(u32,u32),_gz_cgr_span_word(u32,u32,u32,u32);
/* PREPARE only: original source entry to an already qualified current pot.
 * Neither a generic resolver nor permission to read/replay an actor. */
u32 _gz_cgr_pot_rebound_mode(u32,u32);
u32 _gz_cgr_pot_source_owner(u32,u32,const PoEntry*,u32,PoEntry*);
extern u32 _gz_cgr_report[32];
#endif
