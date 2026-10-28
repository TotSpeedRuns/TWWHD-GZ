/* Classify pause technique from controller event order, never player speed.
 *
 * This module shares the legacy trainer's 50-word state. The named indices
 * below document that ABI without changing its size or its other counters.
 * A press spans all consecutive samples where held OR trigger is present:
 * the two views of one physical press must not become two pause events.
 */
#include "mss_visit_join.h"

typedef unsigned int MssWord;
typedef long long MssSignedWide;
typedef unsigned long long MssWide;

enum MssStateIndex {
    MSS_PREVIOUS_HELD = 11,
    MSS_CURRENT_TYPE = 36,
    MSS_DISPLAY_TYPE = 37,
    MSS_PHASE = 42,
    MSS_BASELINE_X = 43,
    MSS_BASELINE_Y = 44,
    MSS_FIRST_FRAME = 45,
    MSS_LAST_RESUME = 46,
    MSS_RESUME = 47,
    MSS_PREVIOUS_TRIGGER = 48,
    MSS_LAST_INPUT_FRAME = 49
};

enum MssPhase {
    MSS_IDLE,
    MSS_AFTER_PAUSE,
    MSS_AFTER_RESUME,
    MSS_AFTER_STICK,
    MSS_COMPLETE
};

enum MssType {
    MSS_UNKNOWN,
    MSS_FAST,
    MSS_SLOW,
    MSS_INSTA, /* Pause -> B -> stick, including B+stick on close edge. */
    MSS_SAME_SAMPLE = 4 /* Unresolved order; never replace a completed label. */
};

/* Decode a calibrated IEEE-754 stick component into signed Q16. Keeping this
 * integer-only avoids adding a floating-point ABI to the input callback.
 * Invalid components cannot establish a movement event. The existing 0.1
 * vector deadband is retained; this revision does not retune stick geometry.
 */
static int mss_axis(MssWord bits, int *out)
{
    MssWord exponent = (bits >> 23) & 255;
    MssWord mantissa = bits & 0x7fffff;
    MssWord shift;
    MssWord value;

    if ((bits & 0x7fffffffu) > 0x40000000u)
        return 0;
    if (exponent)
        mantissa |= 0x800000;
    shift = exponent ? 134 - exponent : 133;
    value = exponent >= 127 ? 65536 : shift >= 32 ? 0 : mantissa >> shift;
    *out = bits >> 31 ? -(int)value : (int)value;
    return 1;
}

static MssWord mss_stick_moved(MssWord x, MssWord y,
                                MssWord old_x, MssWord old_y)
{
    int a, b, c, d;
    MssSignedWide dx, dy;

    if (!mss_axis(x, &a) || !mss_axis(y, &b) ||
        !mss_axis(old_x, &c) || !mss_axis(old_y, &d))
        return 0;
    dx = a - c;
    dy = b - d;
    return (MssWide)(dx * dx + dy * dy) >= 42954916ull;
}

static void mss_result(MssWord *state, MssWord type)
{
    if (type == MSS_FAST || type == MSS_SLOW || type == MSS_INSTA) {
        state[MSS_CURRENT_TYPE] = type;
        state[MSS_DISPLAY_TYPE] = type;
    } else {
        /* The unchanged legacy coach copies any nonzero36 into37. Unknown
         * current order must stay0 so that consumer preserves the latch. */
        state[MSS_CURRENT_TYPE] = MSS_UNKNOWN;
    }
    state[MSS_PHASE] = MSS_COMPLETE;
}

static void mss_begin(MssWord *state, const MssWord *input, MssWord frame)
{
    _gz_msj_begin(state, frame);
    state[MSS_PHASE] = MSS_AFTER_PAUSE;
    state[MSS_BASELINE_X] = input[1];
    state[MSS_BASELINE_Y] = input[2];
    state[MSS_FIRST_FRAME] = frame;
    state[MSS_CURRENT_TYPE] = MSS_UNKNOWN;
    /* Display37 is the last completed classification, not this collector. */
    state[MSS_RESUME] = 0;
}

MssWord _gz_mss_order_input(MssWord *state, const MssWord *input,
                            MssWord frame, MssWord released)
{
    MssWord held = input[0];
    MssWord trigger = input[13];
    MssWord previous = state[MSS_PREVIOUS_HELD] |
                       state[MSS_PREVIOUS_TRIGGER];
    MssWord edges = (held | trigger) & ~previous;
    MssWord phase = state[MSS_PHASE];
    MssWord duplicate = frame == state[MSS_LAST_INPUT_FRAME] &&
                        held == state[MSS_PREVIOUS_HELD] &&
                        trigger == state[MSS_PREVIOUS_TRIGGER];

    state[MSS_PREVIOUS_TRIGGER] = trigger;
    state[MSS_LAST_INPUT_FRAME] = frame;

    /* If one native poll includes release and re-press, held may never look
     * neutral to this callback. The captured release+trigger pair proves a
     * new press. Trigger alone while continuously held does not.
     */
    if (!duplicate)
        edges |= trigger & released;

    /* A delay does not change event order or invalidate a completed result.
     * The existing trainer reset handles leaving the swimming context.
     */
    if (phase > MSS_COMPLETE) {
        state[MSS_PHASE] = phase = MSS_IDLE;
        state[MSS_CURRENT_TYPE] = MSS_UNKNOWN;
    }

    /* A simultaneous Plus+B sample supplies no closing-button order.
     * Do not invent native priority or replace a completed label. */
    if ((edges & 0x4008u) == 0x4008u) {
        if (phase >= MSS_AFTER_PAUSE && phase <= MSS_AFTER_STICK)
            mss_result(state, MSS_SAME_SAMPLE);
        return edges;
    }

    /* Only Plus begins. Plus-close before stick is Fast; stick before
     * Plus-close is Slow. B-close before/with stick is Insta. Ordinary B
     * outside an open attempt cannot begin or replace a result. */
    if ((edges & 8) || ((edges & 0x4000) &&
        (phase == MSS_AFTER_PAUSE || phase == MSS_AFTER_STICK))) {
        if (phase == MSS_AFTER_STICK) {
            mss_result(state, edges & 8 ? MSS_SLOW : MSS_SAME_SAMPLE);
            return edges;
        }
        if (phase == MSS_AFTER_PAUSE) {
            state[MSS_RESUME] = edges & 8 ? 2 : 1;
            if (mss_stick_moved(input[1], input[2],
                               state[MSS_BASELINE_X], state[MSS_BASELINE_Y]))
                mss_result(state, edges & 8 ? MSS_SAME_SAMPLE : MSS_INSTA);
            else
                state[MSS_PHASE] = MSS_AFTER_RESUME;
            return edges;
        }
        if (edges & 8)
            mss_begin(state, input, frame);
        return edges;
    }

    if (phase != MSS_AFTER_PAUSE && phase != MSS_AFTER_RESUME)
        return edges;
    if (mss_stick_moved(input[1], input[2],
                       state[MSS_BASELINE_X], state[MSS_BASELINE_Y])) {
        if (phase == MSS_AFTER_PAUSE) {
            state[MSS_PHASE] = MSS_AFTER_STICK;
        } else {
            mss_result(state, state[MSS_RESUME] == 1 ? MSS_INSTA : MSS_FAST);
            state[MSS_LAST_RESUME] = state[MSS_RESUME];
        }
    }
    return edges;
}
