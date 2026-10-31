#include "held_heap_roundtrip.h"
#include "owned_heap_held_view.h"
#include "owned_heap_native.h"
#include "opaque_chunks.h"
#include "world_global_cut.h"
#include "metadata_slot_plan.h"
#include "owned_heap_process.h"

typedef struct {
    GhwU phase, request, frame, hg, job, slot, root, bytes;
    GhwU forward, inverse, result, dirty, scratch, verified, reason, reserved;
    OisHeldView view;
} GhwState;

typedef char GhwState240[sizeof(GhwState) == 240 ? 1 : -1];
static GhwState ghw __attribute__((section(".bss.gz_ghw")));
volatile GhwU _gz_ghw_report[32]
    __attribute__((section(".data.gz_ghw"))) = {0x47485731u, 128, 2};

static void clear(void *pointer, GhwU bytes) {
    GhwU i;
    for (i = 0; i < bytes / 4; ++i) ((GhwU *)pointer)[i] = 0;
}

static GhwU equal(const void *left, const void *right, GhwU bytes) {
    GhwU i;
    for (i = 0; i < bytes / 4; ++i)
        if (((const GhwU *)left)[i] != ((const GhwU *)right)[i]) return 0;
    return !(bytes & 3u);
}

GhwU _gz_ghw_private_apart(GhwU pointer, GhwU bytes) {
    return ghc_policy_apart(pointer, bytes, (GhwU)&ghw, sizeof(ghw)) &&
        ghc_policy_apart(pointer, bytes, (GhwU)_gz_ghw_report, sizeof(_gz_ghw_report));
}

GhwU _gz_ghw_scope(GhwU request, GhwU frame, GhwU phase) {
    return phase >= GHW_PREPARE && phase <= GHW_VERIFY && ghw.phase == phase &&
        ghw.request == request && ghw.frame == frame &&
        _gz_ghs_apply_scope(request, frame) == 1 &&
        _gz_ghs_hg(request, frame) == ghw.hg &&
        _gz_jwa_ghs_generation(request) == ghw.job &&
        ghw.view.request == request && ghw.view.frame == frame &&
        ghw.view.hg == ghw.hg && ghw.view.job_generation == ghw.job &&
        ghw.view.slot == ghw.slot &&
        equal(&ghw.view, &_gz_ois_held_record, sizeof(ghw.view));
}

static GhwU domain(GhwU phase) {
    return _gz_ghw_scope(ghw.request, ghw.frame, phase) &&
        _gz_ghc_held_domain(ghw.request, ghw.frame, ghw.root, ghw.bytes) &&
        _gz_ois_held_page(ghw.request, ghw.frame, &_gz_ois_held_record) == ghw.scratch;
}

/* All addresses come from the genuine CURRENT producer. The historical
 * snapshot supplies opaque bytes, never a native address to follow.
 * Decode one bounded page into the unused, unpublished CURRENT staging tail.
 * No second whole-domain allocation or new 64 KiB BSS buffer is needed.
 */
static GhwU stream(const GhcHeader *image, GhwU phase, GhwU write) {
    GhwU at = 0, page = 0, bytes, done, i;
    const unsigned char *packed = (const unsigned char *)image + image->data;
    const GhwU *scratch = (const GhwU *)ghw.scratch;
    if (!domain(phase)) return 0;
    while (at < ghw.bytes) {
        volatile GhwU *native = (volatile GhwU *)(ghw.root + at);
        bytes = ghw.bytes - at;
        if (bytes > OC_ENCODE_PAGE) bytes = OC_ENCODE_PAGE;
        if (!domain(phase) ||
            oc_decode_page(packed, image->data_bytes, ghw.bytes, page,
                (void *)scratch, OC_ENCODE_PAGE, &done) != OC_OK || done != bytes)
            return 0;
        if (write) {
            for (i = 0; i < bytes / 4; ++i) native[i] = scratch[i];
            if (phase == GHW_APPLY) ghw.forward = at + bytes;
            else ghw.inverse = at + bytes;
            _gz_ghw_report[11] = ghw.forward;
            _gz_ghw_report[12] = ghw.inverse;
        } else {
            for (i = 0; i < bytes / 4; ++i) {
                if (native[i] != scratch[i]) {
                    _gz_ghw_report[19] = at + i * 4u;
                    return 0;
                }
            }
        }
        at += bytes;
        ++page;
    }
    __sync_synchronize();
    return domain(phase);
}

static GhwU finish(GhwU result, GhwU reason) {
    ghw.result = result;
    ghw.reason = reason;
    _gz_ghw_report[9] = result;
    _gz_ghw_report[10] = reason;
    _gz_ghw_report[11] = ghw.forward;
    _gz_ghw_report[12] = ghw.inverse;
    _gz_ghw_report[17] = ghw.verified;
    _gz_ghw_report[18] = ghw.dirty;
    if (result != GHW_RETAINED) {
        __sync_synchronize();
        ghw.phase = 0;
    }
    _gz_ghw_report[3] = ghw.phase;
    return result;
}

/* Private first retained experiment. Scene labels and outside LIVE
 * bindings must remain identical. These are PRIVATE byte comparisons, not
 * native read grants or a claim that graphics/audio/reference closure exists.
 * Cross-scene and partial ENDING support remain explicit subsequent work. */
static GhwU same_scene_and_outside(const GhcHeader *saved,const GhcHeader *current) {
    const GhcProcessDirectory *s=(const GhcProcessDirectory*)((const unsigned char*)saved+saved->reserved[0]);
    const GhcProcessDirectory *c=(const GhcProcessDirectory*)((const unsigned char*)current+current->reserved[0]);
    const GhcProcessRow *sr=(const GhcProcessRow*)(s+1),*cr=(const GhcProcessRow*)(c+1);
    GhwU i=0,j=0,n=0;
    if(!equal((const unsigned char*)saved+192,(const unsigned char*)current+192,16)||
       !equal(&s->scene,&c->scene,sizeof(s->scene))||s->ending||c->ending||s->outside!=c->outside)return 0;
    while(i<s->count||j<c->count){
        while(i<s->count&&sr[i].binding!=GHCP_BIND_OUTSIDE)++i;
        while(j<c->count&&cr[j].binding!=GHCP_BIND_OUTSIDE)++j;
        if(i==s->count||j==c->count)return i==s->count&&j==c->count&&n==s->outside;
        if(!equal(sr+i,cr+j,sizeof(*sr)))return 0;
        ++i;++j;++n;
    }
    return n==s->outside;
}

GhwU _gz_ghw_roundtrip_held(GhwU request, GhwU frame) {
    const OisHeldView *view;
    const GhcHeader *saved, *current;
    GhwU i, reason = 0, metadata;
    if (ghw.phase) return GHW_RETAINED;
    if (!_gz_ghs_apply_scope(request, frame)) return GHW_REFUSED;
    view = _gz_ois_held_view(request, frame);
    if (!view || view != &_gz_ois_held_record) return GHW_REFUSED;

    clear(&ghw, sizeof(ghw));
    ghw.phase = GHW_PREPARE;
    ghw.request = request; ghw.frame = frame;
    ghw.hg = view->hg; ghw.job = view->job_generation; ghw.slot = view->slot;
    for (i = 0; i < sizeof(*view) / 4; ++i)
        ((GhwU *)&ghw.view)[i] = ((const GhwU *)view)[i];
    for (i = 3; i < 32; ++i) _gz_ghw_report[i] = 0;
    _gz_ghw_report[3] = GHW_PREPARE;
    _gz_ghw_report[4] = request; _gz_ghw_report[5] = frame;
    _gz_ghw_report[6] = ghw.hg; _gz_ghw_report[7] = ghw.job;
    _gz_ghw_report[8] = ghw.slot;
    _gz_ghw_report[24] = 2; /* Private CPU commit if fully verified; inverse on failure. */
    _gz_ghw_report[30] = 255; /* External/global/device closure still missing. */

    if (!_gz_ghw_scope(request, frame, GHW_PREPARE) ||
        _gz_ois_held_validate(request, frame, view)) return finish(GHW_REFUSED, 1);
    saved = (const GhcHeader *)view->saved.archive;
    current = (const GhcHeader *)view->current.archive;
    if (saved->version != GHC_PROCESS_VERSION || current->version != GHC_PROCESS_VERSION ||
        saved->root_mask != 7u || current->root_mask != 7u ||
        saved->domain.heap != current->domain.heap ||
        saved->domain.start != current->domain.start ||
        saved->domain.span != current->domain.span ||
        saved->domain.heap_generation != current->domain.heap_generation ||
        !current->domain.span || (current->domain.span & 3u)) return finish(GHW_REFUSED, 2);
    /* The decoder is intentionally restricted to new 64 KiB AS9 packets.
     * Other packet readers remain compatible with legacy 2 MiB pages.
     */
    if (((const unsigned char *)saved)[saved->data + 12] != 0 ||
        ((const unsigned char *)saved)[saved->data + 13] != 1 ||
        ((const unsigned char *)saved)[saved->data + 14] != 0 ||
        ((const unsigned char *)saved)[saved->data + 15] != 0 ||
        ((const unsigned char *)current)[current->data + 12] != 0 ||
        ((const unsigned char *)current)[current->data + 13] != 1 ||
        ((const unsigned char *)current)[current->data + 14] != 0 ||
        ((const unsigned char *)current)[current->data + 15] != 0) return finish(GHW_REFUSED, 3);

    if (!same_scene_and_outside(saved,current)) return finish(GHW_REFUSED, 10);

    ghw.root = current->domain.start;
    ghw.bytes = current->domain.span;
    ghw.scratch = _gz_ois_held_page(request, frame, view);
    _gz_ghw_report[13] = ghw.root; _gz_ghw_report[14] = ghw.bytes;
    _gz_ghw_report[15] = saved->checksum; _gz_ghw_report[16] = current->checksum;
    _gz_ghw_report[20] = ghw.scratch;
    _gz_ghw_report[21] = OC_ENCODE_PAGE;
    if (!ghw.scratch || !domain(GHW_PREPARE) || !stream(current, GHW_PREPARE, 0) ||
        _gz_wgc_held_step(request, frame, GHW_PREPARE, 0) ||
        _gz_ois_held_validate(request, frame, view)) return finish(GHW_REFUSED, 4);

    if (_gz_hmp_plan_held(request, frame)) return finish(GHW_REFUSED, 8);

    ghw.dirty = 1; ghw.phase = GHW_APPLY;
    __sync_synchronize();
    _gz_ghw_report[3] = GHW_APPLY;
    if (!stream(saved, GHW_APPLY, 1) ||
        _gz_wgc_held_step(request, frame, GHW_APPLY, 1)) reason = 5;
    if (!reason) {
        ghw.phase = GHW_VERIFY;
        _gz_ghw_report[3] = GHW_VERIFY;
        if (!stream(saved, GHW_VERIFY, 0) ||
            _gz_wgc_held_step(request, frame, GHW_VERIFY, 0) ||
            _gz_ois_held_validate(request, frame, view)) reason = 6;
        else {
            ghw.verified = 1;
            metadata = _gz_hmp_commit_held(request, frame);
            _gz_ghw_report[25] = metadata;
            if (metadata == HMP_RT_DIRTY) return finish(GHW_RETAINED, 9);
            if (metadata == HMP_CPU_COMMITTED) {
                ghw.dirty = 0;
                return finish(GHW_CPU_COMMITTED, 0);
            }
            reason = 9;
        }
    }

    /* Every failed/incomplete CPU or metadata application independently
     * inverts before normal execution can resume. Loss of the actual
     * hold retains owners rather than guessing recovery. */
    ghw.phase = GHW_INVERSE;
    _gz_ghw_report[3] = GHW_INVERSE;
    if (!domain(GHW_INVERSE) || _gz_ois_held_validate(request, frame, view) ||
        !stream(current, GHW_INVERSE, 1) ||
        _gz_wgc_held_step(request, frame, GHW_INVERSE, 1) ||
        !stream(current, GHW_INVERSE, 0) ||
        _gz_wgc_held_step(request, frame, GHW_INVERSE, 0) ||
        _gz_hmp_verify_held(request, frame) ||
        _gz_ois_held_validate(request, frame, view)) return finish(GHW_RETAINED, reason ? reason : 7);
    ghw.dirty = 0;
    return finish(reason ? GHW_RECOVERED : GHW_ROUNDTRIP, reason);
}
