/* Included by the real OIS owner after its existing capture/storage code.
 * This is private storage read ownership inside the original genuine held
 * callback. It adds no native world writer, allocation, pin or publication.
 */
extern GhsU _gz_ghs_held_source_matches(GhsU request, GhsU frame,
    const GhcSaveProducer *producer);

OisHeldView _gz_ois_held_record
    __attribute__((section(".bss.gz_ois_held")));

static void ois_held_invalidate(void)
{
    _gz_ois_held_record.valid = 0;
    __sync_synchronize();
}

static U ois_held_ready(U request, U frame)
{
    /* Observation only: preserve the original short-circuit order. */
    if (!(request)) { _gz_ghs_report[29] = 1; return 0; }
    if (!(frame)) { _gz_ghs_report[29] = 2; return 0; }
    if (!(header())) { _gz_ghs_report[29] = 3; return 0; }
    if (!(state.entered == 1)) { _gz_ghs_report[29] = 4; return 0; }
    if (!(!state.writer)) { _gz_ghs_report[29] = 5; return 0; }
    if (!(state.thread)) { _gz_ghs_report[29] = 6; return 0; }
    if (!(state.thread == ois_heap.thread)) { _gz_ghs_report[29] = 7; return 0; }
    if (!(state.thread == (U)OSGetCurrentThread())) { _gz_ghs_report[29] = 8; return 0; }
    if (!(ois_heap.phase == 3)) { _gz_ghs_report[29] = 9; return 0; }
    if (!(!ois_heap.result)) { _gz_ghs_report[29] = 10; return 0; }
    if (!(ois_heap.purpose == GHS_VERSION)) { _gz_ghs_report[29] = 11; return 0; }
    if (!(ois_heap.role == OIS_CUT_CURRENT)) { _gz_ghs_report[29] = 12; return 0; }
    if (!(ois_heap.image_version == GHC_PROCESS_VERSION)) { _gz_ghs_report[29] = 13; return 0; }
    if (!(ois_heap.request == request)) { _gz_ghs_report[29] = 14; return 0; }
    if (!(ois_heap.frame == frame)) { _gz_ghs_report[29] = 15; return 0; }
    if (!(ois_heap.target < OIS_SLOTS)) { _gz_ghs_report[29] = 16; return 0; }
    if (!(ois_heap.used >= 192)) { _gz_ghs_report[29] = 17; return 0; }
    if (!(!(ois_heap.used & 3u))) { _gz_ghs_report[29] = 18; return 0; }
    if (!(region_ok(&state.staging))) { _gz_ghs_report[29] = 19; return 0; }
    if (!(state.staging.pointer)) { _gz_ghs_report[29] = 20; return 0; }
    if (!(ois_heap.used <= state.staging.capacity)) { _gz_ghs_report[29] = 21; return 0; }
    if (!(!state.tx.phase)) { _gz_ghs_report[29] = 22; return 0; }
    if (!(ois_cut.phase == OIS_CUT_RESERVED)) { _gz_ghs_report[29] = 23; return 0; }
    if (!(ois_cut.request == request)) { _gz_ghs_report[29] = 24; return 0; }
    if (!(ois_cut.frame == frame)) { _gz_ghs_report[29] = 25; return 0; }
    if (!(ois_cut.thread == state.thread)) { _gz_ghs_report[29] = 26; return 0; }
    if (!(ois_cut.slot == ois_heap.target)) { _gz_ghs_report[29] = 27; return 0; }
    if (!(pin_valid(&ois_cut.saved))) { _gz_ghs_report[29] = 28; return 0; }
    if (!(_gz_ghs_scope(request, frame, 2) == 1)) { _gz_ghs_report[29] = 29; return 0; }
    if (!(_gz_ghs_role(request, frame) == GHS_COMMAND_RESTORE)) { _gz_ghs_report[29] = 30; return 0; }
    if (!(_gz_ghs_target(request, frame) == ois_heap.target + 1)) { _gz_ghs_report[29] = 31; return 0; }
    if (!(_gz_ghs_image_version(request, frame) == GHC_PROCESS_VERSION)) { _gz_ghs_report[29] = 32; return 0; }
    _gz_ghs_report[29] = 0;
    return 1;
}

/* The original SDK header and prefix protection is retained for every OIS
 * allocation, including the saved user region, staging, undo/map and pair
 * workspace. This helper does not call back into GHS or itself through a
 * generic private-output checker.
 */
static U ois_held_regions_apart(U pointer, U bytes)
{
    const Region *regions[OIS_SLOTS + 5];
    U i, j;
    if (!span(pointer, bytes) ||
        !apart(pointer, bytes, (U)&state, sizeof(state)) ||
        !apart(pointer, bytes, (U)&ois_heap, sizeof(ois_heap)) ||
        !apart(pointer, bytes, (U)&ois_cut, sizeof(ois_cut))) return 0;
    for (i = 0; i < OIS_SLOTS; ++i) {
        if (!slot_ok(i)) return 0;
        regions[i] = &state.slots[i].region;
    }
    regions[OIS_SLOTS] = &state.staging;
    regions[OIS_SLOTS + 1] = &state.tx.undo;
    regions[OIS_SLOTS + 2] = &state.tx.map;
    regions[OIS_SLOTS + 3] = &ois_cut.current;
    regions[OIS_SLOTS + 4] = &ois_cut.work;
    for (i = 0; i < OIS_SLOTS + 5; ++i) {
        const Region *region = regions[i];
        if (!region_ok(region)) return 0;
        if (!region->pointer) continue;
        if (!span(region->heap, 0x54) ||
            !apart(pointer, bytes, region->heap, 0x54) ||
            region->pointer < 0x10000040u ||
            region->capacity > 0xffffffffu - 64 ||
            !apart(pointer, bytes, region->pointer - 64,
                region->capacity + 64)) return 0;
        for (j = 0; j < i; ++j) {
            if (regions[j]->pointer &&
                !apart(region->pointer, region->capacity,
                    regions[j]->pointer, regions[j]->capacity)) return 0;
        }
    }
    return 1;
}

static U ois_held_receiver(void)
{
    U pointer = (U)&_gz_ois_held_record;
    /* Positive ownership is this exact compiled GLOBAL object. The general
     * metadata/image writer correctly refuses it through ois_cut_external.
     * No arbitrary output pointer or archive field selects this exception.
     */
    if (!(span(pointer, sizeof(_gz_ois_held_record)))) { _gz_ghs_report[31] = 1; return 0; }
    if (!(_gz_abi_metadata_span(pointer, sizeof(_gz_ois_held_record)))) { _gz_ghs_report[31] = 2; return 0; }
    if (!(_gz_ghs_private_apart(pointer, sizeof(_gz_ois_held_record)))) { _gz_ghs_report[31] = 3; return 0; }
    if (!(ois_held_regions_apart(pointer, sizeof(_gz_ois_held_record)))) { _gz_ghs_report[31] = 4; return 0; }
    _gz_ghs_report[31] = 0;
    return 1;
}

static U ois_held_images(void)
{
    const GhcSaveProducer *saved;
    const GhcHeader *current = (const GhcHeader *)state.staging.pointer;
    U result;
    result = _gz_ghc_validate((const void *)ois_cut.saved.archive,
        ois_cut.saved.bytes, 1);
    if (result) return 0;
    result = _gz_ghc_validate((const void *)state.staging.pointer,
        ois_heap.used, 0);
    if (result || !ois_heap_image_identity() ||
        current->version != GHC_PROCESS_VERSION) return 0;
    saved = (const GhcSaveProducer *)(ois_cut.saved.archive + 128u);
    return ois_cut_saved_policy(saved, ois_heap.request,
        ois_heap.frame, ois_heap.target);
}

static void ois_held_describe(void)
{
    OisHeldView *view = &_gz_ois_held_record;
    zero(view, sizeof(*view));
    view->magic = OIS_HELD_MAGIC;
    view->bytes = sizeof(*view);
    view->version = OIS_HELD_VERSION;
    view->request = ois_heap.request;
    view->frame = ois_heap.frame;
    view->thread = state.thread;
    view->slot = ois_heap.target;
    view->hg = _gz_ghs_hg(ois_heap.request, ois_heap.frame);
    view->job_generation = _gz_jwa_ghs_generation(ois_heap.request);
    view->missing = OIS_HELD_MISSING_ALL;
    view->current_expected_valid = 0;
    copy(&view->saved, &ois_cut.saved, sizeof(view->saved));
    view->current.allocation = state.staging.pointer;
    view->current.archive = state.staging.pointer;
    view->current.capacity = state.staging.capacity;
    view->current.bytes = ois_heap.used;
    view->current.identity = state.staging.identity;
    view->current.reservation_generation = ois_cut.generation;
    copy(&view->producer, (const void *)(state.staging.pointer + 128u),
        sizeof(view->producer));
}

static U ois_held_matches(void)
{
    const OisHeldView *view = &_gz_ois_held_record;
    return view->magic == OIS_HELD_MAGIC && view->bytes == sizeof(*view) &&
        view->version == OIS_HELD_VERSION && view->valid == 1 &&
        view->request == ois_heap.request && view->frame == ois_heap.frame &&
        view->thread == state.thread && view->slot == ois_heap.target &&
        view->hg && view->hg == _gz_ghs_hg(view->request, view->frame) &&
        view->job_generation &&
        view->job_generation == _gz_jwa_ghs_generation(view->request) &&
        view->missing == OIS_HELD_MISSING_ALL && !view->current_expected_valid &&
        same(&view->saved, &ois_cut.saved, sizeof(view->saved)) &&
        view->current.allocation == state.staging.pointer &&
        view->current.archive == state.staging.pointer &&
        view->current.capacity == state.staging.capacity &&
        view->current.bytes == ois_heap.used &&
        view->current.identity == state.staging.identity &&
        view->current.reservation_generation == ois_cut.generation &&
        !view->current.reserved[0] && !view->current.reserved[1] &&
        same(&view->producer, (const void *)(state.staging.pointer + 128u),
            sizeof(view->producer)) &&
        view->producer.hg == view->hg &&
        view->producer.job_generation == view->job_generation &&
        _gz_ghs_held_source_matches(view->request, view->frame,
            &view->producer) == 1;
}

const OisHeldView *_gz_ois_held_view(U request, U frame)
{
    U result;
    _gz_ghs_report[28] = 1;
    _gz_ghs_report[29] = _gz_ghs_report[30] = _gz_ghs_report[31] = 0;
    result = ois_heap_lock(request, frame, 2);
    if (result) { _gz_ghs_report[30] = result; return 0; }
    _gz_ghs_report[28] = 2;
    if (!ois_held_ready(request, frame) || !ois_held_receiver()) {
        done(GHC_CONTEXT);
        return 0;
    }
    _gz_ghs_report[28] = 3;
    ois_held_invalidate();
    if (!ois_held_images()) {
        done(GHC_CHANGED);
        return 0;
    }
    _gz_ghs_report[28] = 4;
    ois_held_describe();
    __sync_synchronize();
    _gz_ois_held_record.valid = 1;
    _gz_ghs_report[28] = 5;
    if (!ois_held_ready(request, frame) || !ois_held_receiver() ||
        !ois_held_matches() || !ois_held_images() ||
        !ois_held_ready(request, frame) || !ois_held_matches()) {
        ois_held_invalidate();
        done(GHC_CHANGED);
        return 0;
    }
    _gz_ghs_report[28] = 6;
    done(OIS_OK);
    return &_gz_ois_held_record;
}

U _gz_ois_held_validate(U request, U frame, const OisHeldView *view)
{
    U result;
    if (view != &_gz_ois_held_record) return OIS_ARGUMENT;
    result = ois_heap_lock(request, frame, 2);
    if (result) return result;
    result = ois_held_ready(request, frame) && ois_held_receiver() &&
        ois_held_matches() && ois_held_images() &&
        ois_held_ready(request, frame) && ois_held_matches();
    return done(result ? OIS_OK : GHC_CHANGED);
}

U _gz_ois_held_apart(U request, U frame, const OisHeldView *view,
    U pointer, U bytes)
{
    U result;
    if (view != &_gz_ois_held_record) return 0;
    result = ois_heap_lock(request, frame, 2);
    if (result) return 0;
    result = ois_held_ready(request, frame) && ois_held_receiver() &&
        ois_held_matches() &&
        _gz_ghc_all_private_apart(pointer, bytes) &&
        ois_held_regions_apart(pointer, bytes) &&
        ois_held_ready(request, frame) && ois_held_matches();
    done(OIS_OK);
    return result ? 1u : 0u;
}

/* Fixed 64 KiB page output, wholly beyond the immutable CURRENT wire image.
 * The real capture already reserves >64 KiB of private batch scratch, clears
 * it before sealing, and never publishes those tail bytes. No allocation,
 * world pointer or general-purpose output exception is introduced here.
 */
U _gz_ois_held_page(U request, U frame, const OisHeldView *view)
{
    U result, offset, pointer = 0;
    if (view != &_gz_ois_held_record) return 0;
    result = ois_heap_lock(request, frame, 2);
    if (result) return 0;
    if (ois_held_ready(request, frame) && ois_held_receiver() && ois_held_matches() &&
        ois_heap.used <= 0xffffffe0u) {
        offset = (ois_heap.used + 31u) & ~31u;
        if (offset <= state.staging.capacity &&
            0x10000u <= state.staging.capacity - offset &&
            state.staging.pointer <= 0xffffffffu - offset &&
            span(state.staging.pointer + offset, 0x10000u) &&
            apart(state.staging.pointer + offset, 0x10000u,
                state.staging.pointer, ois_heap.used))
            pointer = state.staging.pointer + offset;
    }
    if (!ois_held_ready(request, frame) || !ois_held_matches()) pointer = 0;
    done(OIS_OK);
    return pointer;
}

/* Positive ownership of this exact pending fixed view, not a caller's
 * metadata tuple and not a writable capture-output grant. Full image format
 * is checked by the writer before it dirties anything; immutable wire bytes
 * stay separate from both native targets and the reusable staging tail.
 */
U _gz_ois_held_owner(U request, U frame, const OisHeldView *view)
{
    U result;
    if (view != &_gz_ois_held_record) return 0;
    result = ois_heap_lock(request, frame, 2);
    if (result) return 0;
    result = ois_held_ready(request, frame) && ois_held_receiver() &&
        ois_held_matches() && ois_heap_image_identity() &&
        ois_held_ready(request, frame) && ois_held_matches();
    done(OIS_OK);
    return result ? 1u : 0u;
}

/* Fixed actual CURRENT staging tail after the decoder's first64KiB.
 * No image payload, other OIS allocation, native object or host-selected
 * address can become this working area. It is never published as SAVED.
 */
U _gz_ois_held_work(U request, U frame, const OisHeldView *view, U bytes)
{
    U result, offset, pointer = 0;
    if (view != &_gz_ois_held_record || !bytes || bytes > 0x40000000u) return 0;
    result = ois_heap_lock(request, frame, 2);
    if (result) return 0;
    if (ois_held_ready(request, frame) && ois_held_receiver() && ois_held_matches() &&
        ois_heap.used <= 0xfffeffe0u) {
        offset = ((ois_heap.used + 31u) & ~31u) + 0x10000u;
        if (offset <= state.staging.capacity && bytes <= state.staging.capacity - offset &&
            state.staging.pointer <= 0xffffffffu - offset &&
            span(state.staging.pointer + offset, bytes) &&
            apart(state.staging.pointer + offset, bytes, state.staging.pointer, ois_heap.used))
            pointer = state.staging.pointer + offset;
    }
    if (!ois_held_ready(request, frame) || !ois_held_matches()) pointer = 0;
    done(OIS_OK);
    return pointer;
}
