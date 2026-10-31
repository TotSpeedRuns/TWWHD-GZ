#include "metadata_slot_plan.h"
#include <stdint.h>

static HmpU range(const void *pointer, HmpU bytes) {
    uintptr_t address = (uintptr_t)pointer;
    return address && bytes && bytes <= UINTPTR_MAX - address;
}

static HmpU apart(const void *a, HmpU bytes, const void *b, HmpU other) {
    uintptr_t left = (uintptr_t)a, right = (uintptr_t)b;
    return left >= right ? left - right >= other : right - left >= bytes;
}

static HmpU shape(const HmpSelection *s, HmpU *count, HmpU *bytes) {
    HmpU n;
    if (!s || ((uintptr_t)s & 3u) || !range(s, sizeof(*s)) || !s->slots ||
        s->current_count > s->slots || s->saved_count > s->slots ||
        s->stride < sizeof(HmpSlot) || (s->stride & 3u)) return 0;
    n = s->current_count > s->saved_count ? s->current_count : s->saved_count;
    if (n && s->stride > 0xffffffffu / n) return 0;
    *count = n; *bytes = n * s->stride;
    return 1;
}

HmpU _gz_hmp_slots(const HmpSelection *s, const unsigned char *classes,
                   void *output, HmpU *written) {
    HmpU count, bytes, current = 0, reusable = 0, i, pass, done = 0;
    if (!shape(s, &count, &bytes) || !range(classes, s->slots) ||
        !range(written, sizeof(*written)) || ((uintptr_t)written & 3u) ||
        !apart(written, sizeof(*written), s, sizeof(*s)) ||
        !apart(written, sizeof(*written), classes, s->slots)) return HMP_ARGUMENT;
    if (bytes && (!range(output, bytes) || ((uintptr_t)output & 3u) ||
        !apart(output, bytes, s, sizeof(*s)) || !apart(output, bytes, classes, s->slots) ||
        !apart(output, bytes, written, sizeof(*written)))) return HMP_ARGUMENT;
    *written = 0;
    if (count > s->capacity) return HMP_SPACE;
    if (s->saved_count > 0xffffffffu - s->serial) return HMP_EXHAUSTED;
    for (i = 0; i < s->slots; ++i) {
        if (classes[i] > HMP_SLOT_CURRENT) return HMP_FORMAT;
        if (classes[i] == HMP_SLOT_CURRENT) ++current;
        if (classes[i] == HMP_SLOT_REUSABLE) ++reusable;
    }
    if (current != s->current_count) return HMP_CHANGED;
    if (s->saved_count > current && s->saved_count - current > reusable) return HMP_SPACE;
    for (pass = 0; pass < 2; ++pass) {
        HmpU wanted = pass ? HMP_SLOT_REUSABLE : HMP_SLOT_CURRENT;
        for (i = 0; i < s->slots && done < count; ++i) {
            HmpSlot *row;
            if (classes[i] != wanted) continue;
            row = (HmpSlot *)((unsigned char *)output + done * s->stride);
            row->index = i + 1;
            row->source = done < s->saved_count ? done + 1 : 0;
            row->generation = row->source ? s->serial + row->source : 0;
            ++done;
        }
    }
    *written = done;
    return done == count ? HMP_OK : HMP_CHANGED;
}

HmpU _gz_hmp_pin(const HmpSelection *s, void *output, HmpU count,
                 HmpU ordinal, HmpU current_index, HmpU generation) {
    HmpU expected, bytes, i, found = count, old;
    HmpSlot *target, *other;
    if (!shape(s, &expected, &bytes) || count != expected || !count ||
        !range(output, bytes) || ((uintptr_t)output & 3u) ||
        !apart(output, bytes, s, sizeof(*s)) || !ordinal || ordinal > s->saved_count ||
        !current_index || current_index > s->slots || !generation || generation > s->serial)
        return HMP_ARGUMENT;
    for (i = 0; i < count; ++i) {
        HmpSlot *row = (HmpSlot *)((unsigned char *)output + i * s->stride);
        if (row->source != (i < s->saved_count ? i + 1 : 0) ||
            !row->index || row->index > s->slots ||
            row->generation != (row->source ? s->serial + row->source : 0)) return HMP_FORMAT;
        if (row->index == current_index) {
            if (found != count) return HMP_FORMAT;
            found = i;
        }
    }
    if (found == count) return HMP_CHANGED;
    target = (HmpSlot *)((unsigned char *)output + (ordinal - 1) * s->stride);
    other = (HmpSlot *)((unsigned char *)output + found * s->stride);
    old = target->index; target->index = other->index; other->index = old;
    target->generation = generation;
    return HMP_OK;
}
