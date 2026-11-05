#ifndef GZ_OBJECT_BINDING_TRY_H
#define GZ_OBJECT_BINDING_TRY_H
#include "object_binding.h"

/* Separate metadata-only APIs. One lock attempt; no native payload access,
 * allocator call, callback or wait. A successful binding is still missing31,
 * session0: it grants neither lifetime ownership nor permission to read it.
 *
 * Caller storage is exactly one aligned 64-byte buffer in high DATA, outside
 * all protected images, observer tables and observed managed heap extents.
 * Caller stack buffers are deliberately unsupported by these new functions.
 * The caller must own the buffer and exclude other writers for the call.
 *
 * BUSY means the common metadata lock was not obtained. BUDGET means that
 * matching could not finish within the shared metadata-visit bound. Both
 * refuse conservatively; neither is an invalid-argument or valid match.
 * Every failure leaves the caller's buffer unchanged. Capture writes only
 * after all checks. Validate reads one copy and never modifies its input.
 */
enum { OB_BUSY = 6, OB_BUDGET = 7, OB_TRY_WORK_LIMIT = 65536 };
ObU32 _gz_ob_try_capture(ObU32 pointer, ObU32 bytes, ObBinding *out);
ObU32 _gz_ob_try_validate(const ObBinding *in);
#endif
