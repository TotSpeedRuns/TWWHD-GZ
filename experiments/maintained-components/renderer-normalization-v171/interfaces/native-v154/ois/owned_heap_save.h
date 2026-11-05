#ifndef GZ_OWNED_HEAP_SAVE_PURPOSE_H
#define GZ_OWNED_HEAP_SAVE_PURPOSE_H
/* Fixed native Save/Restore-cut producer; no scalar is a WORLD/READ grant.
 * Producer v1 is the retained Slot10 Save-only record (reserved0).
 * Producer v2 records actual command1/2 in the last legacy reserved word and
 * the ACTUAL command BL return in legacy save_return, never a fake Save LR. */
typedef unsigned int GhsU;
enum {GHS_VERSION=5,GHS_MAGIC=0x48535031u,GHS_QUEUED=7,
 GHS_RESERVED=1,GHS_ARMING=2,GHS_HELD=3,GHS_ENDING=4,GHS_RELEASED=5,GHS_HALTED=6,
 GHS_COMMAND_SAVE=1,GHS_COMMAND_RESTORE=2,GHS_REQUEST_EXHAUSTED=2890};
typedef struct {GhsU magic,bytes,version,request,frame,epoch,slot,thread;
 GhsU original_return,service_return,save_return,hg,lease,job_generation,scene_hash,reserved;} GhcSaveProducer;
typedef char GhcSaveProducer64[sizeof(GhcSaveProducer)==64?1:-1];
/* Exact private scene scalars, separate from the unchanged producer64. */
typedef struct {GhsU epoch,stage[2],room,layer,event,hash,reserved;} GhcSaveScene;
typedef char GhcSaveScene32[sizeof(GhcSaveScene)==32?1:-1];
GhsU _gz_ghs_process_scene(GhsU,GhsU,GhcSaveScene*);
/* Pure format only. It does not authenticate native returns or enroll a hold. */
static inline GhsU ghs_provenance_format(const GhcSaveProducer*p,GhsU request,GhsU frame,
 GhsU hg,GhsU lease,GhsU parent,GhsU thread){return p->magic==GHS_MAGIC&&p->bytes==64&&
 ((p->version==1&&p->slot==9&&!p->reserved)||
  (p->version==2&&p->slot<10&&(p->reserved==GHS_COMMAND_SAVE||p->reserved==GHS_COMMAND_RESTORE)))&&
 request&&frame&&hg&&lease&&parent&&thread&&p->request==request&&p->frame==frame&&p->epoch&&
 p->thread==thread&&p->original_return==parent&&p->service_return&&p->save_return&&p->hg==hg&&
 p->lease==lease&&p->job_generation;}
GhsU _gz_ghs_save_request(GhsU original_return),_gz_ghs_restore_request(GhsU original_return);
/* Fixed producer metadata only;0 is invalid. Target is selected slot+1. */
GhsU _gz_ghs_role(GhsU,GhsU),_gz_ghs_target(GhsU,GhsU),_gz_ghs_image_version(GhsU,GhsU);
GhsU _gz_ghs_picker_present(GhsU slot);
/* Prior authentic issued tuple only, during real RELEASED callback. This
 * does not restore a hold or grant native reads. Exact fixed receiver only. */
extern GhcSaveProducer _gz_gpn_completed_issuer;
GhsU _gz_ghs_completed_source(GhsU,GhsU,GhcSaveProducer*);
void _gz_ghs_tick(void);
GhsU _gz_ghs_busy(void),_gz_ghs_claims(GhsU),_gz_ghs_identity(GhsU),_gz_ghs_context_owned(GhsU);
GhsU _gz_ghs_owner_idle(GhsU),_gz_ghs_scope(GhsU,GhsU,GhsU),_gz_ghs_hg(GhsU,GhsU);
GhsU _gz_ghs_source(GhsU,GhsU,GhcSaveProducer*),_gz_ghs_private_apart(GhsU,GhsU);
GhsU _gz_ghs_heap_propose(GhsU,GhsU);
GhsU _gz_jwa_ghs_try(GhsU),_gz_jwa_ghs_check(GhsU),_gz_jwa_ghs_workers_zero(GhsU);
GhsU _gz_jwa_ghs_generation(GhsU),_gz_jwa_ghs_pending(void),_gz_jwa_ghs_leave(void);
GhsU _gz_jwa_ghs_private_apart(GhsU,GhsU);
extern volatile GhsU _gz_ghs_report[32];
#endif
