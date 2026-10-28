/* Append after the exact wide-digest SO implementation. There is no supplied
 * validation result or callback: this fixed dispatcher runs the real complete
 * saved-only validator itself before recording a kind's first success. */
#include "source_pin_format.h"
#define SFM_API __attribute__((noinline,section(".text.gz_source_format")))
GzSourceFormatMemo _gz_sfm_memo __attribute__((section(".bss.gz_sfm")));
/* Opaque names preserve the original compiler call graph of existing helpers. */
extern GzSourceDigest sfm_original_digest(u32,u32) __asm__("_gz_som_verified_source_digest");
extern u32 sfm_original_pin(void) __asm__("so_pin");
extern u32 sfm_original_scope(void) __asm__("_gz_wct_source_digest_scope");
extern u32 sfm_original_all_ok(void) __asm__("all_ok");
extern u32 sfm_original_view(void) __asm__("osa_view_current");
extern u32 _gz_pfb_archive_saved_validate(u32,u32);
extern u32 _gz_bcb_archive_saved_validate(u32,u32);
extern u32 _gz_lgb_archive_saved_validate(u32,u32);
extern u32 _gz_b294m_archive_saved_validate(u32,u32);

SFM_API u32 _gz_som_source_format_check(u32 source,u32 used,u32 kind)
{
    GzSourceDigest before,after;
    u32 pin,plan,z,phase;
    if (kind<SFM_PF || kind>SFM_B294) return SFM_REFUSE;
    /* Preserve BOM/BSR/capture and all other non-WCT callers' original FULL
     * validation. This branch does not inspect or update the memo. */
    if (!sfm_original_scope()) return SFM_FULL;
    phase=gz_sfm_som_phase(_gz_som_active,_gz_som_pin,_gz_som_token,_gz_som_fatal);
    if (phase==SFM_FULL) {
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
        default: z=_gz_b294m_archive_saved_validate(source,used);break;
        }
        if (z) return SFM_REFUSE;
    }
    /* Real source/used/id/generation/revision/Row.hash and original owner,
     * lease and WCT scope are checked again even on an already-minted hit. */
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
