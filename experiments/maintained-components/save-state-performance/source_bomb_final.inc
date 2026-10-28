/* Reuses the existing real final-FNV expression; no new checksum category. */
#define SBF_API __attribute__((noinline,noclone,section(".text.gz_source_bomb_final")))
SBF_API u32 _gz_source_reuse_scope(void)
{
    u32 bomb=_gz_bfm_source_digest_scope();
    if (bomb==SBP_REFUSE) return SBP_REFUSE;
    return gz_sbp_combine(_gz_wct_source_digest_scope(),bomb);
}
SBF_API u32 _gz_som_verify_bomb_final(u32 token,u32 frame)
{
    const SomInstalled *s=&_gz_som_installed;
    const volatile u32 *o=_gz_roh_owner;
    GzSourceDigest before,after;
    u32 scope=_gz_bfm_source_digest_scope(),z;
    /* Without genuine BOH purpose, the unchanged original owner validate
     * used FULL source hashing. Keep that legacy publication behavior. */
    if (scope==SBP_NONE) return 0;
    if (scope!=SBP_CHECK || !token || token!=_gz_som_token || o[5]!=token ||
        frame!=_gz_roh_live_frame || o[10]!=frame || o[3]!=2 || o[6]!=1 ||
        o[7] || o[12]) return 42;
    before=_gz_som_verified_source_digest(s->source,s->bytes);
    if ((u32)(before>>32)!=GZ_DIGEST_ISSUED || (u32)before!=o[22]) return 42;
    z=_gz_osa_verify_source_final(_gz_som_pin,s->source,s->bytes,
            s->identity,s->generation,s->revision);
    if (z) return z;
    after=_gz_som_verified_source_digest(s->source,s->bytes);
    return after==before && _gz_bfm_source_digest_scope()==SBP_CHECK &&
        token==_gz_som_token && o[5]==token && frame==_gz_roh_live_frame &&
        o[10]==frame && !o[7] && !o[12] ? 0 : 42;
}
#undef SBF_API
