/* Fixed whole-call purpose around the original BOH/BFM bodies. No API result,
 * source pointer, owner token or callback is supplied by a caller. */
#include "source_bomb_scope.h"
#define SBP_FN __attribute__((noinline,noclone,section(".text.gz_source_bomb")))
GzSourceBombCall _gz_sbp_call __attribute__((section(".bss.gz_sbp")));
extern u32 _gz_hg_platform_thread(void);
extern u32 _gz_roh_registry_revision;
/* Opaque C identifiers retain the original static helper call graph. */
extern u32 sbp_boh_sealed(void) __asm__("boh_sealed");
/* The original HBP helper is fully inlined in the selected parent. Use the
 * same fixed predicates and its existing emitted private hash implementation. */
extern u32 sbp_private_hash(const void*,u32) __asm__("btr_hash");
static SBP_FN u32 sbp_hbp_sealed(void)
{
    const HbpRecipe *s=&_gz_hbp_recipe;
    return s->magic==0x48425031u && s->bytes==sizeof(*s) &&
        s->version==1 && s->valid==1 &&
        _gz_hbp_seal==sbp_private_hash(s,sizeof(*s));
}

static SBP_FN u32 sbp_enter(u32 kind)
{
    GzSourceBombCall *s=&_gz_sbp_call;
    u32 thread;
    if (s->entered) { s->poison=1; return 0; }
    /* Legacy, failed and recovery calls keep their original bodies and FULL
     * behavior. No new OS/source query is made for those paths. */
    if (bfm_active!=1 || !bfm_request ||
        bfm_request!=_gz_boh_request || bfm_request==_gz_boh_ack ||
        _gz_roh_owner[3]>=ROH_FAILED_CLEAN ||
        _gz_boh_recovery_entered || _gz_boh_recovery.valid ||
        _gz_boh_recovery_request!=_gz_boh_recovery_ack) return 0;
    thread=_gz_hg_platform_thread();
    if (!thread) return 0;
    s->kind=kind; s->request=bfm_request; s->frame=_gz_pbac_frame;
    s->thread=thread; s->poison=0; s->seen_plan=!!_gz_boh_plan.valid;
    __asm__ volatile("sync":::"memory"); s->entered=1;
    return 1;
}
static SBP_FN void sbp_leave(void)
{
    u32 i;
    /* Fixed private retirement only, including after successful publication.
     * It performs no native/source query or fallible post-commit callback. */
    for (i=0; i<sizeof(_gz_sbp_call)/4; ++i)
        ((volatile u32 *)&_gz_sbp_call)[i]=0;
}
SBP_FN void _gz_boh_step(void)
{
    u32 own=sbp_enter(1);
    sbp_boh_step_inner();
    if (own) sbp_leave();
}
SBP_FN void _gz_bfm_tick(void)
{
    u32 own=sbp_enter(2);
    sbp_bfm_tick_inner();
    if (own) sbp_leave();
}
SBP_FN u32 _gz_bfm_source_digest_scope(void)
{
    GzSourceBombCall *s=&_gz_sbp_call;
    const volatile u32 *o=_gz_roh_owner;
    const HbpRecipe *h=&_gz_hbp_recipe;
    u32 phase=gz_sbp_record_plan(s->entered,s->kind,s->poison,s->seen_plan);
    if (phase!=SBP_CHECK) return phase;
    if (!s->request || s->request!=bfm_request ||
        s->request!=_gz_boh_request || s->request==_gz_boh_ack ||
        s->frame!=_gz_pbac_frame || !s->thread ||
        s->thread!=_gz_hg_platform_thread() || bfm_active!=1 ||
        bfm_slot>9 || _gz_pbac_selected!=bfm_slot ||
        _gz_boh_recovery_entered || _gz_boh_recovery.valid ||
        _gz_boh_recovery_request!=_gz_boh_recovery_ack) return SBP_REFUSE;
    /* Original preissue/decode has no BOH plan. Once this invocation has seen
     * a plan, its disappearance is a refusal, never a permissive fallback. */
    phase=gz_sbp_plan_phase(_gz_boh_plan.valid,s->seen_plan);
    if (phase!=SBP_CHECK) return phase;
    if (!sbp_boh_sealed() || !sbp_hbp_sealed() ||
        _gz_boh_journal.phase>BOH_APPLIED || _gz_boh_journal.error ||
        _gz_boh_plan.hbp_seal!=_gz_hbp_seal ||
        _gz_boh_plan.token!=h->token || _gz_boh_plan.pin!=h->pin ||
        !h->pin || h->pin!=_gz_osa_pin_id() || h->slot!=bfm_slot ||
        h->used!=bfm_bytes || h->identity!=bfm_identity ||
        h->generation!=bfm_generation || !h->identity || !h->generation ||
        !h->revision || h->revision!=_gz_roh_registry_revision ||
        o[0]!=ROH_MAGIC || o[1]!=240 || o[2]!=2 || !o[5] ||
        o[5]!=h->token || _gz_roh_serial!=h->token ||
        !gz_sbp_owner_phase(o[3],o[4],o[6],o[7],o[11],o[12]) ||
        !gz_sbp_deadline(o[9],o[8],s->frame,o[10]) ||
        o[13]>SBP_ROH_MAX_SERVICES || o[16]!=bfm_slot ||
        o[17]!=h->source || o[18]!=h->used || o[22]!=bfm_hash ||
        o[26]!=h->revision) return SBP_REFUSE;
    s->seen_plan=1;
    return SBP_CHECK;
}
#undef SBP_FN
