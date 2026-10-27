/* Join actual MPD visit notifications to one current MSS attempt.
 * No native API, input, menu, actor or save stores. This module changes only
 * trainer metadata; it neither replays nor assumes the order of a pad batch.
 */
#include "mss_visit_join.h"

extern volatile MssJoinWord _gz_mss_state[];
extern MssJoinWord _gz_mpd_report[];
extern volatile MssJoinWord _gz_game_counter, _gz_mss_frame_counter;
extern volatile MssJoinWord _gz_pbac_epoch, _gz_tools_flags, _gz_state[];
extern volatile unsigned char _gz_loading_store;

volatile MssVisitJoin _gz_msj_report = {
    .magic = 0x4d534a31u,
    .bytes = 128,
    .version = 1
};

enum {
    MSJ_CLEAR = 0,
    MSJ_DISABLED = 1,
    MSJ_FRAME_GAP = 2,
    MSJ_CONTEXT = 3,
    MSJ_STALE_BEGIN = 4,
    MSJ_ALREADY_VISITING = 5,
    MSJ_NO_OPENING = 6,
    MSJ_STALE_VISIT = 7,
    MSJ_NOT_TWO_FRAMES = 8,
    MSJ_CONSUMED = 9,
    MSJ_EXHAUSTED = 10,
    MSJ_REENTRY = 11
};

static void msj_cancel(MssJoinWord reason);

typedef struct MssPublication {
    MssJoinWord active, counter_increased;
    MssJoinWord type, display, phase;
    MssJoinWord actor, first_frame;
} MssPublication;

static MssJoinWord msj_enter(void)
{
    MssJoinWord expected;

    for (;;) {
        expected = 0;
        if (__atomic_compare_exchange_n(&_gz_msj_report.busy, &expected, 1, 0,
                                        __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
            return 1;
        if (expected != 1 && expected != 2)
            return 0;
        /* The poison transition and the owner's release-CAS have one atomic
         * order. If release wins, retry normal acquisition and process this
         * notification. If poison wins, the owner must cancel before release.
         * A nested notification never takes ownership of a nonzero guard.
         */
        if (__atomic_compare_exchange_n(&_gz_msj_report.busy, &expected, 2, 0,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
            __atomic_fetch_add(&_gz_msj_report.lost, 1, __ATOMIC_RELAXED);
            return 0;
        }
    }
}

static void msj_leave(const MssPublication *publication)
{
    MssJoinWord expected = 1;

    if (__atomic_compare_exchange_n(&_gz_msj_report.busy, &expected, 0, 0,
                                    __ATOMIC_RELEASE, __ATOMIC_RELAXED))
        return;
    /* A poisoned guard remains exclusively owned until cleanup is complete.
     * No final plain check can race with publication and release. Only a type
     * published by this invocation is rolled back; older completed labels are
     * outside the pending join. 'lost' is a diagnostic count, never authority.
     */
    if (publication && publication->active) {
        if (_gz_mss_state[0] == publication->actor &&
            _gz_mss_state[45] == publication->first_frame &&
            _gz_mss_state[36] == 3 && _gz_mss_state[37] == 3 &&
            _gz_mss_state[42] == 4) {
            _gz_mss_state[36] = publication->type;
            _gz_mss_state[37] = publication->display;
            _gz_mss_state[42] = publication->phase;
        }
        if (publication->counter_increased)
            _gz_msj_report.published--;
    }
    msj_cancel(MSJ_REENTRY);
    __atomic_store_n(&_gz_msj_report.busy, 0, __ATOMIC_RELEASE);
}

static void msj_cancel(MssJoinWord reason)
{
    if ((_gz_msj_report.armed || _gz_msj_report.bound) &&
        _gz_msj_report.cancelled != 0xffffffffu)
        _gz_msj_report.cancelled++;
    _gz_msj_report.armed = 0;
    _gz_msj_report.bound = 0;
    _gz_msj_report.reason = reason;
}

static MssJoinWord msj_enabled(void)
{
    return (_gz_tools_flags & 16u) && !_gz_state[0] && !_gz_loading_store;
}

static MssJoinWord msj_current_attempt(void)
{
    return _gz_msj_report.armed && _gz_msj_report.actor &&
        _gz_msj_report.epoch == _gz_pbac_epoch &&
        _gz_msj_report.actor == _gz_mss_state[0] &&
        _gz_msj_report.begin_native == _gz_mss_state[45] &&
        _gz_mss_state[42] >= 1 && _gz_mss_state[42] <= 4;
}

void _gz_msj_begin(MssJoinWord *state, MssJoinWord native_frame)
{
    MssJoinWord frame = _gz_game_counter;
    MssJoinWord epoch = _gz_pbac_epoch;

    if (!msj_enter())
        return;
    /* Called before the original first-pause initialization. Retire the old
     * visit token before those stores so it cannot label a new attempt.
     */
    msj_cancel(MSJ_CLEAR);
    if (_gz_msj_report.attempt == 0xffffffffu) {
        _gz_msj_report.reason = MSJ_EXHAUSTED;
        goto done;
    }
    _gz_msj_report.attempt++;
    _gz_msj_report.epoch = epoch;
    _gz_msj_report.actor = state[0];
    _gz_msj_report.begin_native = native_frame;
    _gz_msj_report.begin_common = frame;
    if (!msj_enabled() || state != (const MssJoinWord *)_gz_mss_state || !state[0]) {
        _gz_msj_report.reason = MSJ_DISABLED;
        goto done;
    }
    /* Require the current continuous gameplay baseline. Enabling in an old
     * Menu/ToMenu/ToPlay sample cannot retrospectively attach that visit.
     */
    if (native_frame != _gz_mss_frame_counter ||
        !_gz_mpd_report[3] || _gz_mpd_report[5] != epoch ||
        frame - _gz_mpd_report[4] > 1u || !_gz_mpd_report[6] ||
        _gz_mpd_report[7] || _gz_mpd_report[13] != 1 ||
        !_gz_msj_report.observed || !_gz_msj_report.gameplay_baseline ||
        _gz_msj_report.observed_epoch != epoch ||
        frame - _gz_msj_report.observed_common > 1u ||
        _gz_msj_report.observed_mode != 1) {
        _gz_msj_report.reason = MSJ_STALE_BEGIN;
        goto done;
    }
    _gz_msj_report.armed = 1;
done:
    msj_leave(0);
}

void _gz_msj_observe(MssJoinWord frame, MssJoinWord mode, MssJoinWord epoch)
{
    if (!msj_enter())
        return;
    if (!msj_enabled() || mode == 0) {
        msj_cancel(MSJ_DISABLED);
        _gz_msj_report.gameplay_baseline = 0;
    }
    if (_gz_msj_report.observed && epoch != _gz_msj_report.observed_epoch) {
        msj_cancel(MSJ_CONTEXT);
        _gz_msj_report.gameplay_baseline = 0;
    } else if (_gz_msj_report.observed && frame == _gz_msj_report.observed_common) {
        goto done;
    } else if (_gz_msj_report.observed &&
               frame - _gz_msj_report.observed_common != 1u) {
        msj_cancel(MSJ_FRAME_GAP);
        _gz_msj_report.gameplay_baseline = 0;
    }
    _gz_msj_report.observed = 1;
    _gz_msj_report.observed_common = frame;
    _gz_msj_report.observed_epoch = epoch;
    _gz_msj_report.observed_mode = mode;
    if (!msj_enabled() || mode == 0)
        goto done;
    if (_gz_msj_report.armed && !msj_current_attempt())
        msj_cancel(MSJ_CONTEXT);
    if (mode == 1) {
        /* The first completed frame after the raw pause must show opening or
         * Menu. A later unrelated visit cannot inherit an unaccepted press.
         * This is a conservative observation lane, not a timing classifier.
         */
        if (_gz_msj_report.armed && !_gz_msj_report.bound)
            msj_cancel(MSJ_NO_OPENING);
        _gz_msj_report.gameplay_baseline = 1;
    } else if (mode == 3) {
        if (_gz_msj_report.bound) {
            msj_cancel(MSJ_ALREADY_VISITING);
            _gz_msj_report.gameplay_baseline = 0;
        }
    } else if (mode == 4) {
        if (!_gz_msj_report.bound)
            msj_cancel(MSJ_STALE_VISIT);
        _gz_msj_report.gameplay_baseline = 0;
    } else if (mode != 2) {
        msj_cancel(MSJ_STALE_VISIT);
        _gz_msj_report.gameplay_baseline = 0;
    }
done:
    msj_leave(0);
}

void _gz_msj_visit_begin(MssJoinWord frame, MssJoinWord epoch)
{
    if (!msj_enter())
        return;
    if (!msj_enabled() || !msj_current_attempt() ||
        _gz_msj_report.bound || !_gz_msj_report.gameplay_baseline ||
        _gz_msj_report.observed_common != frame ||
        _gz_msj_report.observed_epoch != epoch || _gz_msj_report.observed_mode != 2 ||
        frame != _gz_game_counter || epoch != _gz_pbac_epoch ||
        !_gz_mpd_report[3] || _gz_mpd_report[4] != frame ||
        _gz_mpd_report[5] != epoch || _gz_mpd_report[13] != 2 ||
        !_gz_mpd_report[6] || _gz_mpd_report[7] ||
        frame - _gz_msj_report.begin_common >= 0x80000000u ||
        _gz_mpd_report[11] == 0xffffffffu || _gz_msj_report.visit == 0xffffffffu) {
        msj_cancel(MSJ_STALE_VISIT);
        goto done;
    }
    _gz_msj_report.visit++;
    _gz_msj_report.visit_attempt = _gz_msj_report.attempt;
    _gz_msj_report.visit_epoch = epoch;
    _gz_msj_report.visit_common = frame;
    _gz_msj_report.visit_native = _gz_mss_frame_counter;
    _gz_msj_report.completed_before = _gz_mpd_report[11];
    _gz_msj_report.bound = 1;
done:
    msj_leave(0);
}

void _gz_msj_visit_complete(MssJoinWord frame, MssJoinWord epoch,
                            MssJoinWord settled_frames)
{
    MssPublication publication = {0};

    if (!msj_enter())
        return;
    if (!msj_enabled() || !msj_current_attempt() ||
        !_gz_msj_report.bound || _gz_msj_report.visit_attempt != _gz_msj_report.attempt ||
        _gz_msj_report.visit_epoch != epoch || epoch != _gz_pbac_epoch ||
        frame != _gz_game_counter || _gz_msj_report.observed_common != frame ||
        (_gz_msj_report.observed_mode != 1 && _gz_msj_report.observed_mode != 4) ||
        _gz_mpd_report[4] != frame || _gz_mpd_report[5] != epoch ||
        _gz_mpd_report[13] != _gz_msj_report.observed_mode ||
        !_gz_mpd_report[10] || _gz_mpd_report[9] != settled_frames ||
        _gz_mpd_report[11] != _gz_msj_report.completed_before + 1u ||
        _gz_msj_report.consumed_attempt == _gz_msj_report.attempt) {
        msj_cancel(MSJ_STALE_VISIT);
        goto done;
    }
    _gz_msj_report.completed_visit = _gz_msj_report.visit;
    _gz_msj_report.completed_attempt = _gz_msj_report.attempt;
    _gz_msj_report.completed_frames = settled_frames;
    _gz_msj_report.completed_common = frame;
    _gz_msj_report.completed_native = _gz_mss_frame_counter;
    if (settled_frames != 2) {
        msj_cancel(MSJ_NOT_TWO_FRAMES);
        goto done;
    }
    _gz_msj_report.consumed_attempt = _gz_msj_report.attempt;
    _gz_msj_report.armed = _gz_msj_report.bound = 0;
    publication.active = 1;
    publication.type = _gz_mss_state[36];
    publication.display = _gz_mss_state[37];
    publication.phase = _gz_mss_state[42];
    publication.actor = _gz_mss_state[0];
    publication.first_frame = _gz_mss_state[45];
    _gz_mss_state[36] = 3;
    _gz_mss_state[37] = 3;
    _gz_mss_state[42] = 4;
    if (_gz_msj_report.published != 0xffffffffu) {
        _gz_msj_report.published++;
        publication.counter_increased = 1;
    }
    _gz_msj_report.reason = MSJ_CONSUMED;
done:
    msj_leave(&publication);
}
