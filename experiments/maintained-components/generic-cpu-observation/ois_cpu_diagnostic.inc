/* Fixed diagnostic producer into private OIS slot9, separate from user GZS
 * slots. Type3 can be read as evidence; existing AJ prepare requires type2.
 * No caller selects a native target, source image or capture capacity.
 */
extern U _gz_gac_storage_scope(void), _gz_gac_cleanup_safe(void);
extern U _gz_gac_capture(U, U, U *), _gz_gac_validate(const void *, U);
extern volatile U _gz_gac_report[32];

U _gz_ois_cpu_stage_match(U p, U n)
{
    return header() && state.entered == 1 && state.writer == 4 &&
           state.thread == (U)OSGetCurrentThread() && _gz_gac_storage_scope() &&
           region_ok(&state.staging) && p == state.staging.pointer &&
           n == state.staging.capacity && n == 0x10000u;
}

U _gz_ois_capture_cpu_diagnostic(void)
{
    U z, n = 0, i, whole_hash = 2166136261u;
    Slot *s, old;
    z = lock();
    if (z)
        return z;
    if (!_gz_gac_storage_scope() || state.tx.phase)
        return done(OIS_BUSY);
    s = state.slots + 9;
    if (!slot_ok(9) || s->readers || state.staging.pointer)
        return done(OIS_CORRUPT);
    if (s->generation == 0xffffffffu)
        return done(OIS_EXHAUSTED);
    z = allocate_region(&state.staging, 0x10000u);
    if (z)
        return done(z);
    zero((void *)state.staging.pointer, 128);
    state.writer = 4;
    z = _gz_gac_capture(state.staging.pointer, state.staging.capacity, &n);
    /* No SDK free or new owner publication while a failed read guard is
     * unreleased. Retain exact staging and quarantine this OIS owner. */
    if (!_gz_gac_cleanup_safe()) {
        retained_conflict = 1;
        return z ? z : OIS_CORRUPT;
    }
    state.writer = 0;
    if (!z && (n < 128 || n > state.staging.capacity || (n & 3u)))
        z = OIS_CORRUPT;
    if (!z)
        z = _gz_gac_validate((const void *)state.staging.pointer, n);
    if (z) {
        release_region(&state.staging);
        return done(z);
    }
    for (i = 0; i < n; ++i)
        whole_hash = (whole_hash ^ *(const volatile B *)(state.staging.pointer + i)) * 16777619u;
    copy(&old, s, sizeof(old));
    copy(&s->region, &state.staging, sizeof(s->region));
    zero(&state.staging, sizeof(state.staging));
    s->used = n;
    s->type = 3;
    ++s->generation;
    zero(&s->domain, sizeof(s->domain));
    _gz_gac_report[8] = s->region.identity;
    _gz_gac_report[9] = s->generation;
    _gz_gac_report[10] = n;
    _gz_gac_report[11] = whole_hash;
    __sync_synchronize();
    release_region(&old.region);
    z = done(0);
    /* Readback metadata is published last, after the owner has exited. The
     * complete image remains an immutable private diagnostic, not a slot. */
    _gz_gac_report[12] = s->region.pointer;
    _gz_gac_report[13] = s->region.capacity;
    _gz_gac_report[14] = s->type;
    __sync_synchronize();
    _gz_gac_report[15] = 9;
    return z;
}
