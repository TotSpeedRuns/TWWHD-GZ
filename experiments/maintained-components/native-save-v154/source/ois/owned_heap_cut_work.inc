/* Real OIS-owned bounded workspace for the generic PRIVATE comparison.
 * No new allocator/native alias/stub, caller buffer or actor access. */
static U ois_cut_work_scope(const OisCutPair *pair)
{
    return ois_cut_pair_matches(pair) &&
        _gz_ghs_scope(pair->request, pair->frame, 3) &&
        _gz_ghs_role(pair->request, pair->frame) == OIS_CUT_CURRENT &&
        _gz_ghs_target(pair->request, pair->frame) == pair->slot + 1;
}

static void ois_cut_work_describe(OisCutWork *work)
{
    zero(work, sizeof(*work));
    work->pointer = ois_cut.work.pointer;
    work->capacity = ois_cut.work.capacity;
    work->identity = ois_cut.work.identity;
    work->pair_token = ois_cut.token;
    work->request = ois_cut.request;
    work->frame = ois_cut.frame;
    work->generation = ois_cut.generation;
}

static U ois_cut_work_matches(const OisCutWork *work)
{
    OisCutWork expected;
    if (!region_ok(&ois_cut.work) || !ois_cut.work.pointer) return 0;
    ois_cut_work_describe(&expected);
    return same(work, &expected, sizeof(expected));
}

U _gz_ois_cut_work_reserve(const OisCutPair *input, U bytes, OisCutWork *output)
{
    OisCutPair pair;
    U result = ois_cut_lock();
    if (result) return result;
    if (!external((U)input, sizeof(*input)) || !external((U)output, sizeof(*output)) ||
        !apart((U)input, sizeof(*input), (U)output, sizeof(*output)) ||
        bytes < 192 || bytes > OIS_CUT_WORK_MAX) return done(OIS_ARGUMENT);
    copy(&pair, input, sizeof(pair));
    if (!ois_cut_work_scope(&pair)) return done(GHC_CONTEXT);
    if (ois_cut.work.pointer) return done(OIS_BUSY);
    result = allocate_region(&ois_cut.work, bytes);
    if (result) {
        if (retained_conflict) {
            /* Same original rejection policy: an overlapping SDK address
             * supplies no free authority. Preserve every real saved/current
             * owner and keep the entered storage owner retained. */
            ois_cut.phase = OIS_CUT_RETAINED;
            return GHC_RETAINED;
        }
        return done(result);
    }
    if (!ois_cut_work_scope(&pair)) return done(GHC_CHANGED);
    ois_cut_work_describe(output);
    return done(OIS_OK);
}

U _gz_ois_cut_work_match(const OisCutPair *input, const OisCutWork *lease,
    U pointer, U bytes)
{
    OisCutPair pair;
    OisCutWork work;
    U result = ois_cut_lock();
    if (result) return 0;
    if (!external((U)input, sizeof(*input)) || !external((U)lease, sizeof(*lease))) {
        done(OIS_ARGUMENT);
        return 0;
    }
    copy(&pair, input, sizeof(pair));
    copy(&work, lease, sizeof(work));
    result = ois_cut_work_scope(&pair) && ois_cut_work_matches(&work) &&
        bytes && contains(ois_cut.work.pointer, ois_cut.work.capacity, pointer, bytes);
    if (result) result = ois_cut_work_scope(&pair) && ois_cut_work_matches(&work);
    done(OIS_OK);
    return result ? 1u : 0u;
}
