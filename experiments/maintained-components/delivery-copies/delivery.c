/* No imports, globals or native calls. Preserve all unrelated slot bytes,
 * availability bits and bindings; count actual copies rather than one bit. */
#include "delivery.h"

typedef DelU U;
typedef DelByte B;

U _gz_del_count(const B slots[8], U item)
{
    U i, n = 0;
    if (!slots || item < DEL_FIRST || item > DEL_LAST)
        return 0;
    for (i = 0; i < 8; ++i)
        n += slots[i] == item;
    return n;
}

static void copy(void *out, const void *in, U n)
{
    U i;
    for (i = 0; i < n; ++i)
        ((B *)out)[i] = ((const B *)in)[i];
}

static void remap_removed(const DelInput *in, DelPlan *p, U removed)
{
    U b, j, old, next;
    for (b = 0; b < DEL_BUTTONS; ++b) {
        old = in->selected[b];
        if (old < 48 || old >= 56 || old - 48 < removed)
            continue;
        if (old - 48 > removed)
            next = old - 1;
        else {
            /* The selected copy was removed. Keep another identical letter
             * selected when it remains, otherwise clear the binding/cache. */
            next = DEL_NONE;
            for (j = 0; j < 8; ++j)
                if (p->after.slots[j] == in->slots[removed]) {
                    next = 48 + j;
                    break;
                }
        }
        p->after.selected[b] = (B)next;
        p->after.usable[b] = next == DEL_NONE ? DEL_NONE : p->after.slots[next - 48];
        if (p->after.usable[b] == DEL_NONE)
            p->after.selected[b] = DEL_NONE;
        if (p->after.selected[b] != in->selected[b] ||
            p->after.usable[b] != in->usable[b])
            p->binding_mask |= 1u << b;
    }
}

U _gz_del_plan(const DelInput *in, U item, U action, DelPlan *out)
{
    DelPlan p;
    U i, at = 8, empty = 8, add, result = DEL_OK, bit;
    if (!in || !out || item < DEL_FIRST || item > DEL_LAST ||
        (action != DEL_TOGGLE && action != DEL_ADD && action != DEL_REMOVE))
        return DEL_ARGUMENT;
    copy(&p.after, in, sizeof(*in));
    p.before_count = _gz_del_count(in->slots, item);
    p.after_count = p.before_count;
    p.operation = p.changed = p.grant_bag = p.binding_mask = 0;
    for (i = 0; i < 8; ++i) {
        if (in->slots[i] == item && at == 8)
            at = i;
        if (in->slots[i] == DEL_NONE && empty == 8)
            empty = i;
    }
    add = action == DEL_ADD || (action == DEL_TOGGLE && !p.before_count);
    bit = 1u << (item - DEL_FIRST);
    if (add) {
        if (empty == 8)
            result = DEL_FULL;
        else {
            p.after.slots[empty] = (B)item;
            p.after.flags |= bit;
            p.after_count++;
            p.operation = 1;
            p.changed = p.grant_bag = 1;
        }
    } else if (at < 8) {
        for (i = at; i < 7; ++i)
            p.after.slots[i] = in->slots[i + 1];
        p.after.slots[7] = DEL_NONE;
        p.after_count--;
        if (p.after_count)
            p.after.flags |= bit;
        else
            p.after.flags &= ~bit;
        p.operation = 2;
        p.changed = 1;
        remap_removed(in, &p, at);
    }
    /* Output is detached metadata, including the full-bag no-change plan.
     * The existing guarded inventory writer alone can commit it. */
    copy(out, &p, sizeof(p));
    return result;
}
