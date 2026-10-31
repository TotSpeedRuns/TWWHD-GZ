#ifndef GZ_OIS_SAVED_CURRENT_CUTS_H
#define GZ_OIS_SAVED_CURRENT_CUTS_H
#include "owned_images.h"

enum {
    OIS_CUT_MAGIC = 0x4f435031u,
    OIS_CUT_VERSION = 1,
    OIS_CUT_SAVED = 1,
    OIS_CUT_CURRENT = 2,
    OIS_CUT_MISSING_ALL = 255
};
/* Two PO_CAP8192 censuses,84 bytes per combined row plus8192 fixed slack.
 * Existing image/slot capacities are not increased. */
enum { OIS_CUT_WORK_MAX = 1384448u };

/* This is private storage read ownership, never a game-world/read/write
 * capability. Both leases are independently issued and exactly matched by
 * OIS. No field in the image or caller sets the native producer purpose. */
typedef struct {
    AgU32 magic, bytes, version, valid;
    AgU32 request, frame, generation, token;
    AgU32 missing, slot, reserved[2];
    OisLease saved, current;
} OisCutPair;
typedef char OisCutPair112[sizeof(OisCutPair) == 112 ? 1 : -1];

typedef struct {
    AgU32 pointer, capacity, identity, pair_token;
    AgU32 request, frame, generation, reserved;
} OisCutWork;
typedef char OisCutWork32[sizeof(OisCutWork) == 32 ? 1 : -1];

/* Actual reservation mirror, not an authority query. Does not call GHS,
 * avoiding producer-scope recursion. Returns0 without exact private tuple. */
AgU32 _gz_ois_heap_role(AgU32 request, AgU32 frame);

/* UI hint:0 absent,1 present,2 busy/corrupt/refusal (keep generic route,
 * never fall through to old GZS Restore). Native capture still requires
 * the genuine callback, real saved pin and full image validation. */
AgU32 _gz_ois_cut_occupied(AgU32 slot);

/* Current is an internal region, not an eleventh user slot. At most one
 * paired reader exists. Ordinary OIS calls stay BUSY until release/drop.
 * A stale/wrong release never frees another pair or the user's saved cut.
 * No release API frees or overwrites the actual saved archive. */
AgU32 _gz_ois_cut_acquire(AgU32 request, AgU32 frame, OisCutPair *output);
AgU32 _gz_ois_cut_validate(const OisCutPair *input);
AgU32 _gz_ois_cut_release(const OisCutPair *input);
AgU32 _gz_ois_cut_drop(AgU32 request, AgU32 frame);

/* Genuine post-cleanup Restore callback only. Fixed OIS allocates workspace
 * outside both images, retains its region, and retires it with the pair.
 * Match authenticates a bounded private workspace interval; it grants no
 * native object read/write permission. No caller buffer is adopted. */
AgU32 _gz_ois_cut_work_reserve(const OisCutPair *pair, AgU32 bytes, OisCutWork *output);
AgU32 _gz_ois_cut_work_match(const OisCutPair *pair, const OisCutWork *work,
    AgU32 pointer, AgU32 bytes);
#endif
