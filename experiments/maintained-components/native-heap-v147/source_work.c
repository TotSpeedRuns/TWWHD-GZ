#include "source_work.h"
typedef unsigned int U;
extern U _gz_svd_try(volatile U *);
extern void _gz_svd_unlock(volatile U *);

/* Header: magic, bytes, version, sequence, flags, asynchronous lost flag,
 * rows, row bytes, WORLD, READ, physical missing, session. Then eleven rows
 * of {entered evaluations, extent low32, extent high32, overflow flags};
 * final eight words are reserved zero. No reset/enable/configuration API. */
volatile U _gz_svd_report[64] __attribute__((section(".data.gz_svd"))) = {
    0x53564431u, 256, 1, 0, 0, 0, SVD_ROWS, 16, 0, 0, 31, 0
};
static volatile U svd_busy __attribute__((section(".bss.gz_svd")));
typedef char SvdReport256[sizeof(_gz_svd_report) == 256 ? 1 : -1];

/* Scalar arithmetic only. Saturation records a lower bound, never wraps. */
static U svd_add(volatile U *row, U extent)
{
    U flags = row[3], lo = row[1], hi = row[2], next;
    if (row[0] == 0xffffffffu) flags |= 1;
    else row[0]++;
    if (!(flags & 2)) {
        next = lo + extent;
        if (next < lo) {
            if (hi == 0xffffffffu) {
                hi = next = 0xffffffffu;
                flags |= 2;
            } else hi++;
        }
        row[1] = next;
        row[2] = hi;
    }
    row[3] = flags;
    return flags;
}

void _gz_svd_note(U category, U extent)
{
    U sequence, flags;
    if (category >= SVD_ROWS || _gz_svd_report[3] >= 0xfffffffeu) return;
    if (!_gz_svd_try(&svd_busy)) {
        /* Idempotent aligned sticky store, never reset. This one word is
         * explicitly asynchronous and outside the row seqlock. Any lost note
         * makes this report incomplete; it never changes the real operation. */
        _gz_svd_report[5] = 1;
        return;
    }
    sequence = _gz_svd_report[3];
    if (sequence >= 0xfffffffcu || (sequence & 1)) {
        /* Reserve the last odd/even pair for diagnostic exhaustion. No real
         * admission, owner, token or native result depends on these flags. */
        if (sequence == 0xfffffffcu) {
            _gz_svd_report[3] = 0xfffffffdu;
            __asm__ volatile("sync" ::: "memory");
            _gz_svd_report[4] |= 1;
            __asm__ volatile("sync" ::: "memory");
            _gz_svd_report[3] = 0xfffffffeu;
        }
        _gz_svd_unlock(&svd_busy);
        return;
    }
    _gz_svd_report[3] = sequence + 1;
    __asm__ volatile("sync" ::: "memory");
    flags = svd_add(_gz_svd_report + 12 + category * 4, extent);
    if (flags & 1) _gz_svd_report[4] |= 2;
    if (flags & 2) _gz_svd_report[4] |= 4;
    __asm__ volatile("sync" ::: "memory");
    _gz_svd_report[3] = sequence + 2;
    _gz_svd_unlock(&svd_busy);
}
