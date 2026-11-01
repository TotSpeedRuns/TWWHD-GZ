#ifndef GZ_JEX_QUIET_H
#define GZ_JEX_QUIET_H

/* Observer metadata only. This view never grants a native effect write. */
typedef unsigned int JqU32;
#include "jex_window.h"
typedef struct {
    JqU32 report[24];
    JqU32 owner[10];
    JqU32 scope[4][8];
    JqU32 depth;
    JqU32 cookie;
    JqU32 returned[16];
} JexQuietView;

typedef char JexQuietViewSize[sizeof(JexQuietView) == 336 ? 1 : -1];

static inline JqU32 jq_quiet(const JexQuietView *v)
{
    JqU32 i;
    if (jexw_metadata_reason(v->report, v->returned) ||
        v->depth || v->report[9] || v->owner[2] ||
        v->cookie != v->report[8] || v->cookie != v->report[11] ||
        v->report[12])
        return 0;
    for (i = 0; i < 4; ++i)
        if (v->scope[i][0]) return 0;
    return 1;
}

static inline JqU32 jq_same(const JexQuietView *a, const JexQuietView *b)
{
    const unsigned char *x = (const unsigned char *)a;
    const unsigned char *y = (const unsigned char *)b;
    JqU32 i;
    if (!jq_quiet(a) || !jq_quiet(b))
        return 0;
    for (i = 0; i < sizeof(*a); ++i)
        if (x[i] != y[i])
            return 0;
    return 1;
}

#endif
