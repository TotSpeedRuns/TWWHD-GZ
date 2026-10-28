/* Include after the exact source module implementation. No new state, output
 * pointer, callback, heap/body access or supplied native/API result. */
#include "source_pin_digest.h"
#define SPD_API __attribute__((noinline,section(".text.gz_source_digest")))
#define SPD_RESULT(mode,value) (((GzSourceDigest)(mode)<<32)|(u32)(value))
SPD_API GzSourceDigest _gz_osa_verified_source_digest(u32 pin,u32 source,u32 used,
        u32 identity,u32 generation,u32 revision)
{
    const OsaPin *p=&_gz_osa_state.pin;
    u32 digest;
    if(!all_ok()||owner.entered||owner.writer) return SPD_RESULT(GZ_DIGEST_REFUSE,0);
    if(!owner.lease.active) {
        if(pin||p->active) return SPD_RESULT(GZ_DIGEST_REFUSE,0);
        return SPD_RESULT(GZ_DIGEST_FULL,0);
    }
    if(!pin||!identity||!generation||!revision||revision!=_gz_roh_registry_revision||
       _gz_osa_validate(pin)||p->serial!=pin||p->allocation!=source||p->bytes!=used||
       p->identity!=identity||p->generation!=generation) return SPD_RESULT(GZ_DIGEST_REFUSE,0);
    digest=so_raw_hash(pin); /* Sealed real Row.hash, fully checked in so_acquire. */
    if(_gz_osa_validate(pin)||p->serial!=pin||p->allocation!=source||p->bytes!=used||
       p->identity!=identity||p->generation!=generation||revision!=_gz_roh_registry_revision)
        return SPD_RESULT(GZ_DIGEST_REFUSE,0);
    return SPD_RESULT(GZ_DIGEST_ISSUED,digest);
}
SPD_API GzSourceDigest _gz_som_verified_source_digest(u32 source,u32 used)
{
    const SomInstalled *s=&_gz_som_installed;
    GzSourceDigest result;
    if(!so_pin()) {
        if(_gz_som_active||_gz_som_pin||_gz_som_token||_gz_som_fatal)
            return SPD_RESULT(GZ_DIGEST_REFUSE,0);
        return _gz_osa_verified_source_digest(0,source,used,0,0,0);
    }
    if(_gz_som_active!=1||_gz_som_fatal||_gz_som_slot>9||!_gz_som_pin||
       !som_logical_installed(_gz_som_slot)||s->source!=source||s->bytes!=used||
       (_gz_som_token&&(_gz_roh_owner[5]!=_gz_som_token||_gz_roh_serial!=_gz_som_token||_gz_roh_owner[4]!=1)))
        return SPD_RESULT(GZ_DIGEST_REFUSE,0);
    result=_gz_osa_verified_source_digest(_gz_som_pin,source,used,s->identity,s->generation,s->revision);
    /* Preserve complete original ROH hashing outside the genuine ordinary
     * WCT invocation, whose healthy publication has the forced-full exit. */
    if((u32)(result>>32)==GZ_DIGEST_ISSUED&&!_gz_wct_source_digest_scope())
        return SPD_RESULT(GZ_DIGEST_FULL,0);
    return result;
}
SPD_API u32 _gz_osa_verify_source_final(u32 pin,u32 source,u32 used,u32 identity,u32 generation,u32 revision)
{
    GzSourceDigest before=_gz_osa_verified_source_digest(pin,source,used,identity,generation,revision),after;
    u32 actual;
    if((u32)(before>>32)!=GZ_DIGEST_ISSUED)return 42;
    actual=(_gz_svd_note(SVD_ROH_ARCHIVE_FNV,used),sc_hash((const void*)source,used));
    after=_gz_osa_verified_source_digest(pin,source,used,identity,generation,revision);
    return before==after&&actual==(u32)before?0:42;
}
#undef SPD_RESULT
#undef SPD_API
