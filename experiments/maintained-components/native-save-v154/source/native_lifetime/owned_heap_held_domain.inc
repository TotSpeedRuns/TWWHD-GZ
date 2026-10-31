/* The complete original non-storage physical exclusions are retained.
 * The storage portion is checked through the real fixed held view below,
 * not the phase2 producer-only writable-output API. */
GHC_FN u32 ghc_held_private_apart(u32 p,u32 n){u32 a=(u32)_gz_rsa_image_begin,b=(u32)_gz_rsa_image_last;
 return a>=0x10000000u&&b>=a&&b<0xbffe0000u&&
 ghc_apart(p,n,a,b-a+1)&&_gz_ghc_all_private_apart(p,n)&&_gz_gpn_native_private_apart(p,n)&&
 _gz_rsa_private_apart(p,n)&&_gz_osa_private_apart(p,n)&&
 _gz_krc_private_apart(p,n)&&_gz_cgp_private(p,n)&&
 _gz_hr_buffer_private((const void*)p,n);
}

/* Actual same-TU native domain provider for the private apply/inverse
 * experiment. No archived root or caller-selected range enrolls memory.
 * Native bodies are touched only by the separate held writer after this
 * CURRENT proposal, genuine GHS/HG/GPA/job and physical exclusion check.
 */
GHC_API u32 _gz_ghc_held_domain(u32 req, u32 f, u32 pointer, u32 bytes) {
    const OisHeldView *view = &_gz_ois_held_record;
    const GhcHeader *current;
    const GhdCut *cut;
    u32 same, hg;
    _gz_ghw_report[31] = 1; /* Observation only, never a grant. */
    if (!_gz_ghs_apply_scope(req, f) || !ghc_proposal.active ||
        ghc_proposal.purpose != GHS_VERSION || ghc_proposal.request != req ||
        ghc_proposal.frame != f || pointer != ghc_proposal.raw ||
        bytes != ghc_proposal.bytes || !bytes || (bytes & 3u) ||
        view->valid != 1 || view->request != req || view->frame != f ||
        view->missing != 255 || view->current_expected_valid ||
        view->current.archive != view->current.allocation ||
        !_gz_ois_held_apart(req, f, view, pointer, bytes)) return 0;
    _gz_ghw_report[31] = 2;
    current = (const GhcHeader *)view->current.archive;
    if (current->version != GHC_PROCESS_VERSION || current->valid ||
        current->bytes != view->current.bytes || current->cut != 208 ||
        current->domain.start != pointer || current->domain.span != bytes ||
        current->domain.heap != ghc_proposal.root.heap ||
        current->domain.heap_generation != ghc_proposal.root.generation ||
        current->request != req || current->frame != f) return 0;
    cut = (const GhdCut *)(view->current.archive + current->cut);
    _gz_ghw_report[31] = 3;
    hg = ghc_actual_hg(req, f);
    if (!hg || hg != view->hg || current->hg != hg ||
        !ghc_gpa(hg, cut) || !ghc_held_private_apart(pointer, bytes) ||
        !_gz_ois_held_apart(req, f, view, pointer, bytes) || !ghc_hl_lock()) return 0;
    same = ghc_same_root() && ghc_hl_healthy();
    ghc_hl_unlock();
    same = same && _gz_ghs_apply_scope(req, f) && ghc_actual_hg(req, f) == hg &&
        ghc_gpa(hg, cut);
    _gz_ghw_report[31] = same ? 0 : 4;
    return same;
}

#include "owned_metadata_journal.inc"

#include "owned_metadata_roundtrip.inc"
