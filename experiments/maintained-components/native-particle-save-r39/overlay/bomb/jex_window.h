#ifndef GZ_JEX_WINDOW_H
#define GZ_JEX_WINDOW_H

/* Pure fixed-buffer causal-window operations. A successful operation grants
 * no native pointer, actor lifetime, device ownership or replay authority.
 * Native callers hold JEL's actual mutex and supply its own current arrays.
 * Tests may export these same bodies with JEXW_API defined to an empty token.
 */
typedef unsigned int JexwU;

enum {
    JEXW_CAPACITY = 512, JEXW_EVENT_WORDS = 16,
    JEXW_REPORT_WORDS = 24, JEXW_RETURNED_WORDS = 16,
    JEXW_REPORT_VERSION = 2,
    JEXW_OK = 0, JEXW_HEADER = 41, JEXW_COUNTS = 42,
    JEXW_SCOPE = 43, JEXW_BITMAP = 45, JEXW_RESERVED = 47,
    JEXW_WINDOW = 53, JEXW_EXHAUSTED = 54, JEXW_WRAP = 55
};

/* Report2 words: 4 total issued event serial; 5 dense used slots; 6 next
 * recycle search cursor; 7 unreturned native removals; 8 scope cookie issued;
 * 9 active scope depth; 10 last scope request; 11 scope completions;
 * 12 historical foreign-thread attribution count; 13 recycled slots;
 * 14..17 retained kinds1..4; 18..23 reserved zero.
 * Returned bits are PHYSICAL SLOT bits. An unreturned kind2 slot is pinned.
 */
typedef struct {
    JexwU slot, serial, used, cursor, outstanding, recycled, kind;
    JexwU kinds[4];
} JexwPlan;

typedef struct {
    JexwU slot, cookie, frame, thread;
    JexwU event[JEXW_EVENT_WORDS];
} JexwReturnPlan;

typedef char JexwPlan44[sizeof(JexwPlan) == 44 ? 1 : -1];
typedef char JexwReturnPlan80[sizeof(JexwReturnPlan) == 80 ? 1 : -1];

#ifndef JEXW_API
#define JEXW_API static inline
#endif

JEXW_API JexwU jexw_metadata_reason(const JexwU *report,
                                   const JexwU *returned) {
    JexwU i, bits, returned_count = 0, kinds = 0, first, used, serial;
    if (!report || !returned || report[0] != 0x4a455831u ||
        report[1] != 96 || report[2] != JEXW_REPORT_VERSION || report[3])
        return JEXW_HEADER;
    serial = report[4];
    used = report[5];
    if (used != (serial < JEXW_CAPACITY ? serial : JEXW_CAPACITY) ||
        report[6] >= JEXW_CAPACITY || (used < JEXW_CAPACITY && report[6] != used) ||
        report[13] != serial - used || report[7] > used)
        return JEXW_COUNTS;
    for (i = 0; i < 4; ++i) {
        if (report[14 + i] > used - kinds) return JEXW_COUNTS;
        kinds += report[14 + i];
    }
    if (kinds != used || report[7] > report[15]) return JEXW_COUNTS;
    if (report[9] > 4 || report[11] > report[8] ||
        report[8] - report[11] != report[9] ||
        (!report[8] && report[10]) || (report[8] && !report[10]))
        return JEXW_SCOPE;
    for (i = 0; i < JEXW_RETURNED_WORDS; ++i) {
        first = i * 32;
        bits = returned[i];
        if (first >= used) {
            if (bits) return JEXW_BITMAP;
        } else if (used - first < 32 &&
                   (bits & ~((1u << (used - first)) - 1u))) {
            return JEXW_BITMAP;
        }
        while (bits) {
            bits &= bits - 1u;
            ++returned_count;
        }
    }
    if (returned_count > report[15] ||
        report[7] != report[15] - returned_count) return JEXW_BITMAP;
    for (i = 18; i < JEXW_REPORT_WORDS; ++i)
        if (report[i]) return JEXW_RESERVED;
    return JEXW_OK;
}

/* Full locked source validation, including exact retained serial uniqueness.
 * The temporary serial set is a bounded4KiB stack object, not new game BSS.
 * The wire snapshot intentionally omits these512 event payloads and therefore
 * can validate only its versioned metadata; it cannot recreate full history.
 */
JEXW_API JexwU jexw_window_reason(const JexwU *report, const JexwU *events,
                                 const JexwU *returned) {
    JexwU seen[1024], kinds[4] = {0, 0, 0, 0};
    JexwU i, j, serial, kind, bit, pending = 0, index, reason;
    reason = jexw_metadata_reason(report, returned);
    if (reason || !events) return reason ? reason : JEXW_WINDOW;
    for (i = 0; i < 1024; ++i) seen[i] = 0;
    for (i = 0; i < JEXW_CAPACITY; ++i) {
        const JexwU *event = events + i * JEXW_EVENT_WORDS;
        if (i >= report[5]) {
            for (j = 0; j < JEXW_EVENT_WORDS; ++j)
                if (event[j]) return JEXW_WINDOW;
            continue;
        }
        serial = event[0];
        kind = event[1];
        bit = (returned[i / 32] >> (i % 32)) & 1u;
        if (!serial || serial > report[4] || kind < 1 || kind > 4 ||
            !event[2] || !event[3] || (bit && kind != 2)) return JEXW_WINDOW;
        index = (serial * 2654435761u) & 1023u;
        for (j = 0; j < 1024 && seen[index]; ++j) {
            if (seen[index] == serial) return JEXW_WINDOW;
            index = (index + 1u) & 1023u;
        }
        if (j == 1024) return JEXW_WINDOW;
        seen[index] = serial;
        ++kinds[kind - 1];
        if (kind == 2 && !bit) ++pending;
    }
    if (pending != report[7]) return JEXW_WINDOW;
    for (i = 0; i < 4; ++i)
        if (kinds[i] != report[14 + i]) return JEXW_WINDOW;
    return JEXW_OK;
}

/* Plans are immutable values, not caller capabilities. commit recomputes the
 * plan against the same current arrays before making any store.
 */
JEXW_API JexwU jexw_plan(const JexwU *report, const JexwU *events,
                        const JexwU *returned, JexwU kind, JexwPlan *output) {
    JexwPlan next;
    JexwU reason, i, index, old_kind = 0, bit;
    reason = jexw_metadata_reason(report, returned);
    if (reason) return reason;
    if (!events || !output || kind < 1 || kind > 4) return JEXW_WINDOW;
    if (report[4] == 0xffffffffu) return JEXW_WRAP;
    if (report[5] < JEXW_CAPACITY) {
        index = report[5];
        for (i = 0; i < JEXW_EVENT_WORDS; ++i)
            if (events[index * JEXW_EVENT_WORDS + i]) return JEXW_WINDOW;
    } else {
        index = report[6];
        for (i = 0; i < JEXW_CAPACITY; ++i) {
            const JexwU *event = events + index * JEXW_EVENT_WORDS;
            old_kind = event[1];
            bit = (returned[index / 32] >> (index % 32)) & 1u;
            if (!event[0] || event[0] > report[4] || old_kind < 1 ||
                old_kind > 4 || (bit && old_kind != 2)) return JEXW_WINDOW;
            if (old_kind != 2 || bit) break;
            index = (index + 1u) & (JEXW_CAPACITY - 1u);
        }
        if (i == JEXW_CAPACITY) return JEXW_EXHAUSTED;
    }
    next.slot = index;
    next.serial = report[4] + 1u;
    next.used = report[5] + (report[5] < JEXW_CAPACITY);
    next.cursor = (index + 1u) & (JEXW_CAPACITY - 1u);
    next.outstanding = report[7] + (kind == 2);
    next.recycled = next.serial - next.used;
    next.kind = kind;
    for (i = 0; i < 4; ++i) next.kinds[i] = report[14 + i];
    if (old_kind) {
        if (!next.kinds[old_kind - 1]) return JEXW_WINDOW;
        --next.kinds[old_kind - 1];
    }
    ++next.kinds[kind - 1];
    if (next.outstanding > JEXW_CAPACITY) return JEXW_COUNTS;
    for (i = 0; i < sizeof(next) / 4; ++i)
        ((JexwU *)output)[i] = ((const JexwU *)&next)[i];
    return JEXW_OK;
}

JEXW_API JexwU jexw_commit(JexwU *report, JexwU *events, JexwU *returned,
                          const JexwPlan *plan, const JexwU *event) {
    JexwPlan actual;
    JexwU i, reason;
    if (!plan || !event) return JEXW_WINDOW;
    reason = jexw_plan(report, events, returned, plan->kind, &actual);
    if (reason) return reason;
    for (i = 0; i < sizeof(actual) / 4; ++i)
        if (((const JexwU *)plan)[i] != ((const JexwU *)&actual)[i])
            return JEXW_WINDOW;
    if (event[0] != actual.serial || event[1] != actual.kind ||
        !event[2] || !event[3]) return JEXW_WINDOW;
    /* Burn the exact serial before the first slot store. No fallible work
     * remains. Do not assign the whole report or clear concurrent fault bits.
     */
    report[4] = actual.serial;
    __sync_synchronize();
    returned[actual.slot / 32] &= ~(1u << (actual.slot % 32));
    for (i = 0; i < JEXW_EVENT_WORDS; ++i)
        events[actual.slot * JEXW_EVENT_WORDS + i] = event[i];
    report[5] = actual.used;
    report[6] = actual.cursor;
    report[7] = actual.outstanding;
    report[13] = actual.recycled;
    for (i = 0; i < 4; ++i) report[14 + i] = actual.kinds[i];
    __sync_synchronize();
    return JEXW_OK;
}

JEXW_API JexwU jexw_find_remove(const JexwU *report, const JexwU *events,
                               const JexwU *returned, JexwU cookie,
                               JexwU *output_slot) {
    JexwU reason, i, found = JEXW_CAPACITY, matches = 0;
    reason = jexw_metadata_reason(report, returned);
    if (reason) return reason;
    if (!events || !output_slot || !cookie || cookie > report[4])
        return JEXW_WINDOW;
    for (i = 0; i < report[5]; ++i) {
        if (events[i * JEXW_EVENT_WORDS] != cookie) continue;
        ++matches;
        found = i;
    }
    if (matches != 1 || events[found * JEXW_EVENT_WORDS + 1] != 2 ||
        (returned[found / 32] & (1u << (found % 32)))) return JEXW_WINDOW;
    *output_slot = found;
    return JEXW_OK;
}

JEXW_API JexwU jexw_plan_return(const JexwU *report, const JexwU *events,
                               const JexwU *returned, JexwU cookie,
                               JexwU frame, JexwU thread, JexwReturnPlan *output) {
    JexwReturnPlan next;
    JexwU i, slot, reason;
    if (!output || !frame || !thread) return JEXW_WINDOW;
    reason = jexw_find_remove(report, events, returned, cookie, &slot);
    if (reason) return reason;
    if (!report[7] || events[slot * JEXW_EVENT_WORDS + 3] != thread ||
        !events[slot * JEXW_EVENT_WORDS + 2] ||
        frame - events[slot * JEXW_EVENT_WORDS + 2] >= 0x80000000u)
        return JEXW_WINDOW;
    next.slot = slot; next.cookie = cookie; next.frame = frame; next.thread = thread;
    for (i = 0; i < JEXW_EVENT_WORDS; ++i)
        next.event[i] = events[slot * JEXW_EVENT_WORDS + i];
    for (i = 0; i < sizeof(next) / 4; ++i)
        ((JexwU *)output)[i] = ((const JexwU *)&next)[i];
    return JEXW_OK;
}

/* The actual native return is recorded BEFORE its kind4 completion append.
 * A full window can consequently recycle this very slot. The caller retains
 * the copied entry in plan and must never set its old slot bit afterward.
 */
JEXW_API JexwU jexw_mark_return(JexwU *report, const JexwU *events,
                               JexwU *returned, const JexwReturnPlan *plan) {
    JexwReturnPlan actual;
    JexwU i, reason;
    if (!plan) return JEXW_WINDOW;
    reason = jexw_plan_return(report, events, returned, plan->cookie,
                             plan->frame, plan->thread, &actual);
    if (reason) return reason;
    for (i = 0; i < sizeof(actual) / 4; ++i)
        if (((const JexwU *)plan)[i] != ((const JexwU *)&actual)[i])
            return JEXW_WINDOW;
    --report[7];
    returned[actual.slot / 32] |= 1u << (actual.slot % 32);
    __sync_synchronize();
    return JEXW_OK;
}

#endif
