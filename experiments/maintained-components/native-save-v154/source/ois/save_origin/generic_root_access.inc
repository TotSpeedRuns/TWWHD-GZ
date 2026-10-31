/* Included only in the selected save-root owner translation unit. This is a
 * distinct native diagnostic producer, not manufactured KRC/VWS stage state.
 * It retains every current unknown resource/worker/device closure bit.
 */
extern SpoU32 _gz_gac_kernel_scope(SpoU32), _gz_gac_read_under_root(SpoU32);
extern SpoU32 _gz_gac_kernel_identity(SpoU32);
SpoU32 _gz_gac_root_report[16] = { 0x47435231u, 64, 1 };

static SpoU32 gac_root_context_owned(SpoU32 cookie)
{
    return _gz_gac_root_entered == 1 && cookie &&
           cookie == _gz_gac_root_serial && _gz_gac_root_frame == _gz_pbac_frame &&
           _gz_gac_root_thread && _gz_gac_root_thread == _gz_hg_platform_thread() &&
           srh_sealed() && _gz_gac_kernel_scope(_gz_gac_root_frame);
}

SpoU32 _gz_gac_particle_entry(SpoU32 frame)
{
    return frame == _gz_gac_root_frame && !_gz_gac_root_gate &&
           gac_root_context_owned(_gz_gac_root_serial) ? _gz_gac_root_serial : 0;
}

SpoU32 _gz_gac_particle_identity(SpoU32 frame, SpoU32 cookie, SpoU32 thread)
{
    /* Foreign guarded JPA writers may inspect these scalar identities only.
     * No native target pointer or memory-copy permission is returned. */
    return _gz_gac_root_entered == 1 && cookie && cookie == _gz_gac_root_serial &&
           thread && thread == _gz_gac_root_thread && frame == _gz_gac_root_frame &&
           frame == _gz_pbac_frame && srh_sealed() && _gz_gac_kernel_identity(frame);
}

SpoU32 _gz_gac_root_read_scope(SpoU32 frame)
{
    return frame == _gz_gac_root_frame && gac_root_context_owned(_gz_gac_root_serial) &&
           _gz_gac_root_gate && !_gz_hg_capture_seal(_gz_gac_root_gate) &&
           !_gz_jpg_capture_gpu_ready(3, frame);
}

SpoU32 _gz_gac_root_access(void)
{
    SpoU32 frame = _gz_pbac_frame, gate = 0, particle = 0, z = 0, end;
    if (_gz_srh_any_access_entered() || _gz_gqk_access_entered() ||
        _gz_dck_access_entered() || _gz_doh_access_entered() ||
        !_gz_gac_kernel_scope(frame) || !srh_sealed() ||
        _gz_gac_root_serial == 0xffffffffu)
        return 28;
    ++_gz_gac_root_serial;
    _gz_gac_root_entered = 1;
    _gz_gac_root_frame = frame;
    _gz_gac_root_thread = _gz_hg_platform_thread();
    _gz_gac_root_report[3] = frame;
    _gz_gac_root_report[4] = _gz_gac_root_serial;
    _gz_gac_root_report[9] = _gz_gac_root_report[10] = _gz_gac_root_report[11] = 0;
    z = _gz_jpg_capture_begin(3, frame);
    _gz_gac_root_report[9] = z;
    if (z)
        goto done;
    particle = 1;
    /* Real GPU submitted/DrawDone/wait/retired path, while the allocator is
     * OPEN. This is not a drain of all other resource/worker submitters. */
    z = _gz_jpg_capture_gpu(3, frame);
    _gz_gac_root_report[10] = z;
    if (z)
        goto done;
    gate = _gz_hg_capture_begin(_gz_gac_root_serial);
    if (!gate) {
        z = 86;
        goto done;
    }
    _gz_gac_root_gate = gate;
    _gz_gac_root_report[12] = gate;
    z = _gz_hg_capture_seal(gate);
    if (z)
        goto done;
    z = srh_current();
    if (z)
        goto done;
    _gz_gac_root_report[13] = srh_hash(&_gz_srh_binding, sizeof(_gz_srh_binding));
    if (!_gz_gac_root_read_scope(frame)) {
        z = 5;
        goto done;
    }
    z = _gz_gac_read_under_root(frame);
    if (z)
        goto done;
    z = srh_current();
    if (!z)
        z = _gz_hg_capture_seal(gate);
done:
    if (gate) {
        end = _gz_hg_capture_end(gate);
        if (end)
            _gz_gac_root_report[14] |= 1;
        if (!z)
            z = end;
        _gz_gac_root_report[6] = end;
    }
    if (particle && !(_gz_gac_root_report[14] & 1u)) {
        end = _gz_jpg_capture_end(3, frame);
        if (end)
            _gz_gac_root_report[14] |= 2;
        if (!z)
            z = end;
        _gz_gac_root_report[11] = end;
    }
    if (!z && !_gz_gac_kernel_scope(frame))
        z = 5;
    _gz_gac_root_report[5] = z;
    _gz_gac_root_report[7] = _gz_hl_control.flags;
    _gz_gac_root_report[8] = _gz_hg_control[15];
    if (!_gz_gac_root_report[14])
        _gz_gac_root_gate = _gz_gac_root_entered = 0;
    return z;
}
