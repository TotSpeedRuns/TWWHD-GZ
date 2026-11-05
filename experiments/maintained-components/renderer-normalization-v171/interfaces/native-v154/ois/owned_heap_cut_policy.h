#ifndef GZ_OIS_HEAP_CUT_POLICY_H
#define GZ_OIS_HEAP_CUT_POLICY_H
#include "owned_heap_save.h"

/* Pure scalar policy shared by the real owner and ROOT host fixtures.
 * These predicates never authenticate native callbacks or grant a hold. */
static inline GhsU ois_cut_reservation_policy(GhsU role, GhsU target_plus_one,
    GhsU slots, GhsU image_version, GhsU required_version)
{
    return (role == GHS_COMMAND_SAVE || role == GHS_COMMAND_RESTORE) &&
        target_plus_one && target_plus_one <= slots && image_version == required_version;
}

static inline GhsU ois_cut_saved_policy(const GhcSaveProducer *saved,
    GhsU request, GhsU frame, GhsU target)
{
    GhsU age = frame - saved->frame;
    if (saved->slot != target || !age || age >= 0x80000000u) return 0;
    if (saved->version == 1)
        return saved->slot == 9 && !saved->reserved;
    return saved->version == 2 && saved->slot < 10 &&
        saved->reserved == GHS_COMMAND_SAVE && saved->request < request;
}

static inline GhsU ois_cut_staged_policy(const GhcSaveProducer *current,
    GhsU request, GhsU frame, GhsU thread, GhsU target, GhsU role)
{
    return current->version == 2 && current->request == request &&
        current->frame == frame && current->thread == thread &&
        current->slot == target && current->reserved == role;
}
#endif
