"""Portable scalar reference checks, not native APIs or production execution.

These model only row validation/selection and metadata retirement closure.
They do not replace the original synchronization or entry/locate providers.
"""
import copy


def collider_plan(rows, pointer, size, active, serial, cached):
    """Return the exact retirement indices, or None before any mutation."""
    if pointer & 3 or pointer < 0x10000000 or pointer >= 0xc0000000 or not size or size > 0xc0000000-pointer:
        return None
    count = 0
    indices = []
    overflow = False
    for i, (p, generation, state) in enumerate(rows):
        if state == 0:
            if p or generation:
                return None
            continue
        if state not in (1, 2, 3) or p & 3 or p < 0x10000000 or p > 0xc0000000-0x50 or generation > serial or (state == 1 and not generation) or (state == 3 and generation):
            return None
        if state in (1, 3):
            count += 1
            if not overflow and pointer < p+0x50 and p < pointer+size:
                indices.append(i)
                if cached and len(indices) == 33:
                    overflow = True
    if count != active:
        return None
    if overflow:
        return [i for i, (p, _, state) in enumerate(rows) if state in (1, 3) and pointer < p+0x50 and p < pointer+size]
    return indices


def sdk_drain(heaps, allocations, optimized):
    """Compare the old full-repeat closure with the no-empty-repeat rule.

    Heap: live, heap key, generation, buffer, backing heap/pointer/generation,
    retire site. Allocation: live, heap, heap generation, pointer, generation,
    retire site. All values are scalar test metadata, not real SDK results.
    """
    heaps = copy.deepcopy(heaps)
    allocations = copy.deepcopy(allocations)
    passes = 0
    while True:
        passes += 1
        changed = False
        for i, h in enumerate(heaps):
            if h[0] != 2:
                continue
            h[0] = 0
            if not optimized:
                changed = True
            for r in allocations:
                if not r[0] or r[1] != h[1] or (r[2] and r[2] != h[2]):
                    continue
                r[0] = 0
                r[5] = h[7]
                first = None
                for j, child in enumerate(heaps):
                    if child[0] != 1:
                        continue
                    if (child[6] == r[4] and child[5] == r[3] and child[4] == r[1]) or (not child[6] and child[3] == r[3]):
                        child[0] = 2
                        child[7] = h[7]
                        if first is None:
                            first = j
                if optimized and first is not None and first <= i:
                    changed = True
        if not changed:
            return heaps, allocations, passes
