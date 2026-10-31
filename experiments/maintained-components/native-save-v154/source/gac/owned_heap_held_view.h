#ifndef GZ_OIS_HELD_RESTORE_VIEW_H
#define GZ_OIS_HELD_RESTORE_VIEW_H

#include "owned_images.h"
#include "owned_heap_save.h"

enum {
    OIS_HELD_MAGIC = 0x4f485631u,
    OIS_HELD_VERSION = 1,
    OIS_HELD_MISSING_ALL = 255
};

/* This is the real pending SDK staging region. It has no read-pin token
 * and is deliberately not an OisLease or a published OisCutPair.
 * reservation_generation is the OIS current-cut counter at reservation;
 * it may be zero. It is not a native HL generation or a published current
 * generation. Actual allocation identity, request and frame distinguish it.
 */
typedef struct {
    AgU32 allocation, archive, capacity, bytes;
    AgU32 identity, reservation_generation, reserved[2];
} OisHeldCurrent;

typedef struct {
    AgU32 magic, bytes, version, valid;
    AgU32 request, frame, thread, slot;
    AgU32 hg, job_generation, missing, current_expected_valid;
    OisLease saved;
    OisHeldCurrent current;
    GhcSaveProducer producer;
} OisHeldView;

typedef char OisHeldCurrent32[sizeof(OisHeldCurrent) == 32 ? 1 : -1];
typedef char OisHeldView176[sizeof(OisHeldView) == 176 ? 1 : -1];

/* Exact fixed receiver owned by OIS; no stack, archive or caller buffer is
 * adopted. A record alone never authenticates a live hold or native write.
 */
extern OisHeldView _gz_ois_held_record;

/* Only a genuine Restore2 HELD callback with a completed pending capture
 * can obtain this const view. The original saved pin remains OIS-owned.
 * Current remains valid0 and immutable; no publish, allocation or release
 * occurs here. NULL means refusal, never a synthetic empty cut.
 */
const OisHeldView *_gz_ois_held_view(AgU32 request, AgU32 frame);
AgU32 _gz_ois_held_validate(AgU32 request, AgU32 frame, const OisHeldView *view);

/* Physical exclusion only: it does not authorize following a historical
 * pointer, native object access or a world write. The actual writer must
 * independently own the genuine current HG/Gate/write purpose.
 */
AgU32 _gz_ois_held_apart(AgU32 request, AgU32 frame, const OisHeldView *view,
    AgU32 pointer, AgU32 bytes);

/* Unpublished staging-tail scratch only;0 refuses. */
AgU32 _gz_ois_held_page(AgU32 request, AgU32 frame, const OisHeldView *view);

/* Fixed actual pending view ownership only; never a native writer grant. */
AgU32 _gz_ois_held_owner(AgU32 request, AgU32 frame, const OisHeldView *view);

/* Owned CURRENT working tail after decoder scratch, never native authority. */
AgU32 _gz_ois_held_work(AgU32 request, AgU32 frame, const OisHeldView *view, AgU32 bytes);

#endif
