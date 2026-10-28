/* Append after the exact wide-digest SO implementation. There is no supplied
 * validation result or callback: this fixed dispatcher runs the real complete
 * saved-only validator itself before recording a kind's first success. */
#include "source_pin_format.h"
#define SFM_API __attribute__((noinline,section(".text.gz_source_format")))
GzSourceFormatMemo _gz_sfm_memo __attribute__((section(".bss.gz_sfm")));
/* Opaque names preserve the original compiler call graph of existing helpers. */
extern GzSourceDigest sfm_original_digest(u32,u32) __asm__("_gz_som_verified_source_digest");
extern u32 sfm_original_pin(void) __asm__("so_pin");
extern u32 sfm_original_scope(void) __asm__("_gz_source_reuse_scope");
extern u32 sfm_original_all_ok(void) __asm__("all_ok");
extern u32 sfm_original_view(void) __asm__("osa_view_current");
extern u32 _gz_pfb_archive_saved_validate(u32,u32);
extern u32 _gz_bcb_archive_saved_validate(u32,u32);
extern u32 _gz_lgb_archive_saved_validate(u32,u32);
extern u32 _gz_b294m_archive_saved_validate(u32,u32);
extern u32 _gz_sand_archive_saved_validate(u32,u32);
extern u32 _gz_cga_archive_saved_validate(u32,u32);

/* These three helpers are actual local definitions in selected R923. The
 * asm names retain their original GCC call graph; this suffix does not add a
 * second custodian, owner, pin or supplied validation-result interface. */
extern u32 sfm_swc_view(void) __asm__("swc_view");
extern u32 sfm_swc_context(u32,u32,u32) __asm__("swc_context");
extern u32 sfm_swc_idle(void) __asm__("swc_idle");
extern GzSourceDigest sfm_osa_digest(u32,u32,u32,u32,u32,u32)
    __asm__("_gz_osa_verified_source_digest");
#include "source_custodian_format_policy.h"

static SFM_API u32 sfm_swc_same(const SwcState *a,const SwcState *b)
{
    const u32 *x=(const u32 *)a,*y=(const u32 *)b;
    u32 i;
    for (i=0;i<sizeof(*a)/4;i++) if (x[i]!=y[i]) return 0;
    return 1;
}
/* Only the lexical held admission/installation calls qualify. Internal SO
 * decoding precedes swc publication and stays on the unchanged FULL path.
 * The three original work callbacks may each advance the real revision;
 * this helper accepts only the newly sealed, actual current value and never
 * permits a change during one saved-only validation/memo operation. */
static SFM_API GzSourceDigest sfm_swc_digest(u32 source,u32 used,
        SwcState *before,const SwcState *expected)
{
    GzSourceDigest value;
    u32 i;
    if (!gz_scf_stage(swc.phase,swc.borrowed,swc.work_count) ||
        swc.error || !sfm_swc_view() || !sfm_swc_idle() ||
        !sfm_swc_context(swc.key.request,swc.key.frame,swc.key.slot) ||
        swc.revision!=_gz_roh_registry_revision ||
        swc.source!=source || swc.used!=used) return
            ((GzSourceDigest)GZ_DIGEST_REFUSE<<32);
    if (expected && !sfm_swc_same(&swc,expected)) return
        ((GzSourceDigest)GZ_DIGEST_REFUSE<<32);
    for (i=0;i<sizeof(*before)/4;i++)
        ((u32 *)before)[i]=((const volatile u32 *)&swc)[i];
    value=sfm_osa_digest(before->pin,source,used,before->identity,
                        before->generation,before->revision);
    if ((u32)(value>>32)!=GZ_DIGEST_ISSUED ||
        (u32)value!=before->raw_hash || !sfm_swc_view() ||
        !sfm_swc_idle() || !sfm_swc_context(before->key.request,
            before->key.frame,before->key.slot) ||
        before->revision!=_gz_roh_registry_revision ||
        !sfm_swc_same(&swc,before)) return
            ((GzSourceDigest)GZ_DIGEST_REFUSE<<32);
    return value;
}

static SFM_API u32 sfm_swc_format_check(u32 source,u32 used,u32 kind)
{
    SwcState before_key,after_key;
    GzSourceDigest before,after;
    u32 pin,plan,z;
    before=sfm_swc_digest(source,used,&before_key,0);
    if ((u32)(before>>32)!=GZ_DIGEST_ISSUED) return SFM_REFUSE;
    pin=before_key.pin;
    if (sfm_original_pin()!=pin) return SFM_REFUSE;
    plan=gz_sfm_plan(_gz_sfm_memo.pin,_gz_sfm_memo.mask,pin,kind);
    if (plan==SFM_REFUSE) return SFM_REFUSE;
    if (plan==SFM_MINT) {
        /* Run exactly the original complete saved-only leaf. No previous
         * caller result or logical installation result can supply a bit. */
        switch (kind) {
        case SFM_PF: z=_gz_pfb_archive_saved_validate(source,used);break;
        case SFM_BC: z=_gz_bcb_archive_saved_validate(source,used);break;
        case SFM_LG: z=_gz_lgb_archive_saved_validate(source,used);break;
        case SFM_B294: z=_gz_b294m_archive_saved_validate(source,used);break;
        case SFM_SAND: z=_gz_sand_archive_saved_validate(source,used);break;
        default: z=_gz_cga_archive_saved_validate(source,used);break;
        }
        if (z) return SFM_REFUSE;
    }
    after=sfm_swc_digest(source,used,&after_key,&before_key);
    if (after!=before || sfm_original_pin()!=pin) return SFM_REFUSE;
    if (plan==SFM_MINT) {
        if (_gz_sfm_memo.pin!=pin) {
            _gz_sfm_memo.mask=0;
            _gz_sfm_memo.pin=pin;
        }
        _gz_sfm_memo.mask|=1u<<(kind-1);
    }
    return SFM_REUSE;
}

SFM_API u32 _gz_som_source_format_check(u32 source,u32 used,u32 kind)
{
    GzSourceDigest before,after;
    u32 pin,plan,z,phase,scope;
    if (kind<SFM_PF || kind>SFM_CGA) return SFM_REFUSE;
    /* Preserve capture, recovery and other callers outside the two genuine
     * purposes. FULL does not inspect or update the memo. */
    scope=sfm_original_scope();
    if (scope==SBP_NONE) return _gz_swc_pending()?SFM_REFUSE:SFM_FULL;
    if (scope!=SBP_CHECK) return SFM_REFUSE;
    phase=gz_sfm_som_phase(_gz_som_active,_gz_som_pin,_gz_som_token,_gz_som_fatal);
    if (phase==SFM_FULL) {
        if (_gz_swc_pending()) return sfm_swc_format_check(source,used,kind);
        /* Exact old full helper, not an issued digest/memo result. The real
         * private metadata/view predicates allow internal entered1 decode
         * with no lease, and still reject invalid/stale/lost live views. */
        if (!gz_sfm_full_lease_phase(owner.lease.active,owner.lease.serial,
                _gz_osa_state.pin.active) || !sfm_original_all_ok() ||
                !sfm_original_view()) return SFM_REFUSE;
        return SFM_FULL;
    }
    if (phase!=SFM_CHECK) return SFM_REFUSE;
    before=sfm_original_digest(source,used);
    if ((u32)(before>>32)==GZ_DIGEST_FULL) return SFM_FULL;
    if ((u32)(before>>32)!=GZ_DIGEST_ISSUED) return SFM_REFUSE;
    pin=sfm_original_pin();
    plan=gz_sfm_plan(_gz_sfm_memo.pin,_gz_sfm_memo.mask,pin,kind);
    if (plan==SFM_REFUSE) return SFM_REFUSE;
    if (plan==SFM_MINT) {
        /* Exactly one known source validator; no arbitrary function pointer. */
        switch (kind) {
        case SFM_PF: z=_gz_pfb_archive_saved_validate(source,used);break;
        case SFM_BC: z=_gz_bcb_archive_saved_validate(source,used);break;
        case SFM_LG: z=_gz_lgb_archive_saved_validate(source,used);break;
        case SFM_B294: z=_gz_b294m_archive_saved_validate(source,used);break;
        case SFM_SAND: z=_gz_sand_archive_saved_validate(source,used);break;
        default: z=_gz_cga_archive_saved_validate(source,used);break;
        }
        if (z) return SFM_REFUSE;
    }
    /* Real source/used/id/generation/revision/Row.hash and original owner,
     * lease and genuine purpose are checked again on an already-minted hit. */
    after=sfm_original_digest(source,used);
    if (after!=before || sfm_original_pin()!=pin) return SFM_REFUSE;
    if (plan==SFM_MINT) {
        if (_gz_sfm_memo.pin!=pin) {
            _gz_sfm_memo.mask=0;
            _gz_sfm_memo.pin=pin;
        }
        _gz_sfm_memo.mask|=1u<<(kind-1);
    }
    return SFM_REUSE;
}
#undef SFM_API
