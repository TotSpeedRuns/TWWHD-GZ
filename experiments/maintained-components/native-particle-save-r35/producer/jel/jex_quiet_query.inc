/* Included after the real JEL/JEX definitions. The caller may supply only
 * its two fixed private witness buffers, authenticated before any write.
 * enter(0) is the original nonblocking metadata lock, not a JPA read lease.
 * No native object, event payload, old address, or observer history is written.
 */
#include "jex_quiet.h"
extern u32 _gz_lsc_jex_view_buffer(u32, void *, u32);

u32 _gz_jex_quiet_read(u32 slot, JexQuietView *out, u32 bytes)
{
    JexQuietView value;
    u32 i, j;
    volatile u32 *dst;
    const u32 *src;

    if (slot >= JEL_ROWS || bytes != sizeof(value) ||
        _gz_lsc_jex_view_buffer(slot, out, bytes) != 1 || !enter(0))
        return 0;

    for (i = 0; i < 24; ++i)
        value.report[i] = _gz_jex_report[i];
    src = (const u32 *)(_gz_jex_owners + slot);
    for (i = 0; i < 10; ++i)
        value.owner[i] = src[i];
    for (i = 0; i < JEX_DEPTH; ++i) {
        src = (const u32 *)(_gz_jex_scopes + i);
        for (j = 0; j < 8; ++j)
            value.scope[i][j] = src[j];
    }
    value.depth = _gz_jex_depth;
    value.cookie = _gz_jex_cookie;
    for (i = 0; i < JEX_CAP / 32; ++i)
        value.returned[i] = _gz_jex_returned[i];

    if (!jq_quiet(&value)) {
        leave();
        return 0;
    }
    /* Volatile fixed-size words forbid an implicit freestanding memcpy. */
    dst = (volatile u32 *)out;
    src = (const u32 *)&value;
    for (i = 0; i < sizeof(value) / 4; ++i)
        dst[i] = src[i];
    leave();
    return 1;
}
