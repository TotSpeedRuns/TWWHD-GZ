/* Private fixed callers only. FULL executes the exact original hash/note;
 * REFUSE never falls back through a lost pin. The force-full call is used
 * only at the healthy final WCT precommit boundary. No CURRENT result caches. */
#include "source_pin_digest.h"
static u32 spd_changed(u32 pin,u32 source,u32 used,u32 identity,u32 generation,
        u32 revision,u32 expected,u32 category,u32 force_full)
{
    GzSourceDigest value=_gz_osa_verified_source_digest(pin,source,used,identity,generation,revision);
    u32 mode=(u32)(value>>32),actual;
    if(mode!=GZ_DIGEST_FULL&&mode!=GZ_DIGEST_ISSUED)return 1;
    if(mode==GZ_DIGEST_ISSUED&&!force_full)return (u32)value!=expected;
    actual=(_gz_svd_note(category,used),btr_hash((const void*)source,used));
    if(actual!=expected)return 1;
    if(mode==GZ_DIGEST_ISSUED) {
        GzSourceDigest after=_gz_osa_verified_source_digest(pin,source,used,identity,generation,revision);
        if(after!=value)return 1;
    }
    return 0;
}
