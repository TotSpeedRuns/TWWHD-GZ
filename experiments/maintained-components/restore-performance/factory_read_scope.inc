/* One genuine native READ hold around only the five mapped-view queries.
 * HG calls the metadata-only callback through SRH before/after acquisition
 * and at every seal. No owner/common/source/actor callback is made there. */
#include "factory_read_scope.h"

extern u32 _gz_hg_capture_begin(u32);
extern u32 _gz_hg_capture_seal(u32);
extern u32 _gz_hg_capture_end(u32);
extern u32 _gz_hg_platform_thread(void);
extern u32 _gz_nsv_access_entered(void);
extern u32 _gz_nrt_access_entered(void);
extern u32 _gz_srh_base_access_entered(void);
extern u32 _gz_gqk_access_entered(void);
extern u32 _gz_doh_access_entered(void);
extern u32 _gz_dck_access_entered(void);
extern u32 _gz_roh_factory_source_begin(u32, u32);
extern u32 _gz_roh_factory_source_end(u32, u32, u32);

enum {BMR_MAGIC = 0x424d5231u, BMR_CONTEXT = 1580};

static struct {
    GzFactoryReadScope scope;
    u32 seal;
    u32 entered;
    u32 poison;
    u32 serial;
} bmh_read;

static u32 bmh_read_hash(const GzFactoryReadScope *scope)
{
    const u8 *bytes = (const u8 *)scope;
    u32 i;
    u32 value = 2166136261u;

    for (i = 0; i < sizeof(*scope); i++) {
        value = (value ^ ((i >= 12 && i < 16) ? 0 : bytes[i])) * 16777619u;
    }
    return value;
}

static void bmh_read_retire(void)
{
    u32 i;

    /* The monotonic purpose serial survives retirement. */
    for (i = 0; i < sizeof(bmh_read.scope) / 4; i++) {
        ((volatile u32 *)&bmh_read.scope)[i] = 0;
    }
    bmh_read.seal = 0;
    bmh_read.entered = 0;
    bmh_read.poison = 0;
}

static void bmh_read_publish(void)
{
    bmh_read.seal = bmh_read_hash(&bmh_read.scope);
    __asm__ volatile("sync" ::: "memory");
    bmh_read.scope.valid = 1;
}

static u32 bmh_read_matches_at(u32 token, u32 frame, u32 thread)
{
    const GzFactoryReadScope *scope = &bmh_read.scope;
    const volatile u32 *owner = _gz_roh_owner;

    if (bmh_read.entered != 1 || bmh_read.poison || scope->valid != 1 ||
        scope->magic != BMR_MAGIC || scope->bytes != sizeof(*scope) ||
        scope->version != 1 || !scope->serial || scope->serial != bmh_read.serial ||
        bmh_read.seal != bmh_read_hash(scope) || !token || scope->token != token ||
        scope->frame != frame || frame != _gz_pbac_frame ||
        !thread || scope->thread != thread || scope->epoch != _gz_pbac_epoch ||
        !_gz_bmh_busy || !_gz_bmh_scope(token, frame)) {
        return 0;
    }
    if (!scope->request || scope->ack == BAD ||
        scope->request != scope->ack + 1 ||
        scope->request != _gz_boh_request || scope->ack != _gz_boh_ack ||
        scope->service < 2 || scope->service > 8 ||
        scope->service != _gz_bmh_receipt.id || owner[12] != scope->service ||
        owner[13] != scope->service_calls || scope->bmh_seal != _gz_bmh_seal ||
        scope->progress_seal != _gz_bmh_progress_seal ||
        scope->progress_mask != _gz_bmh_mask ||
        scope->plan_seal != _gz_boh_plan_seal || scope->run_seal != _gz_boh_run_seal ||
        scope->view_seal != _gz_bmv_seal || scope->hbp_seal != _gz_hbp_seal ||
        scope->pin != _gz_boh_plan.pin || scope->pin != _gz_osa_pin_id() ||
        scope->source != _gz_hbp_recipe.source || scope->used != _gz_hbp_recipe.used ||
        scope->selected != _gz_pbac_selected ||
        _gz_boh_observe_entered || _gz_boh_lifetime_entered || _gz_boh_pumping ||
        _gz_boh_recovery_entered || _gz_bsg_access_entered() ||
        _gz_wgp_unreleased() || _gz_nsv_access_entered() ||
        _gz_nrt_access_entered() || _gz_srh_base_access_entered() ||
        _gz_gqk_access_entered() || _gz_doh_access_entered() ||
        _gz_dck_access_entered()) {
        return 0;
    }
    return 1;
}

u32 _gz_bmh_read_context_owned(u32 token)
{
    u32 thread;

    if (bmh_read.entered != 1 || bmh_read.poison) {
        return 0;
    }
    thread = _gz_hg_platform_thread();

    return bmh_read_matches_at(token, _gz_pbac_frame, thread);
}

const GzFactoryReadScope *_gz_bmh_source_context(u32 token, u32 frame)
{
    if (!bmh_read_matches_at(token, frame, _gz_hg_platform_thread()) ||
        !bmh_read.scope.hold || _gz_hg_capture_seal(bmh_read.scope.hold) ||
        !bmh_read_matches_at(token, frame, _gz_hg_platform_thread())) {
        return 0;
    }
    return &bmh_read.scope;
}

static u32 bmh_read_leave(u32 result)
{
    u32 seal;
    u32 end;

    if (!bmh_read.entered || !bmh_read.scope.hold) {
        return result ? result : BMR_CONTEXT;
    }
    seal = _gz_hg_capture_seal(bmh_read.scope.hold);
    end = _gz_hg_capture_end(bmh_read.scope.hold);
    if (end) {
        /* Keep the actual purpose represented on uncertain physical release. */
        bmh_read.poison = 1;
        return result ? result : end;
    }
    bmh_read_retire();
    return result ? result : seal;
}

static u32 bmh_read_enter(u32 token, u32 frame)
{
    GzFactoryReadScope *scope = &bmh_read.scope;
    u32 hold;
    u32 thread;

    if (bmh_read.entered || bmh_read.poison || scope->valid || bmh_read.seal) {
        bmh_read.poison = 1;
        return BMR_CONTEXT;
    }
    if (bmh_read.serial == BAD || !_gz_bmh_busy || !_gz_bmh_scope(token, frame)) {
        return BMR_CONTEXT;
    }
    bmh_read.entered = 1;
    bmh_read.serial++;
    thread = _gz_hg_platform_thread();
    scope->magic = BMR_MAGIC;
    scope->bytes = sizeof(*scope);
    scope->version = 1;
    scope->serial = bmh_read.serial;
    scope->token = token;
    scope->frame = frame;
    scope->epoch = _gz_pbac_epoch;
    scope->thread = thread;
    scope->request = _gz_boh_request;
    scope->ack = _gz_boh_ack;
    scope->service = _gz_roh_owner[12];
    scope->service_calls = _gz_roh_owner[13];
    scope->bmh_seal = _gz_bmh_seal;
    scope->progress_seal = _gz_bmh_progress_seal;
    scope->progress_mask = _gz_bmh_mask;
    scope->plan_seal = _gz_boh_plan_seal;
    scope->run_seal = _gz_boh_run_seal;
    scope->view_seal = _gz_bmv_seal;
    scope->hbp_seal = _gz_hbp_seal;
    scope->pin = _gz_boh_plan.pin;
    scope->source = _gz_hbp_recipe.source;
    scope->used = _gz_hbp_recipe.used;
    scope->selected = _gz_pbac_selected;
    scope->hold = 0;
    bmh_read_publish();
    if (!bmh_read_matches_at(token, frame, thread)) {
        bmh_read_retire();
        return BMR_CONTEXT;
    }
    hold = _gz_hg_capture_begin(token);
    if (!hold) {
        bmh_read_retire();
        return BMR_CONTEXT;
    }
    scope->valid = 0;
    scope->hold = hold;
    bmh_read_publish();
    if (!_gz_bmh_source_context(token, frame)) {
        return bmh_read_leave(BMR_CONTEXT);
    }
    return 0;
}
