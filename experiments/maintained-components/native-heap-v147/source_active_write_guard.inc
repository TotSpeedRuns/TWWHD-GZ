/* Append after the genuine SO owner implementation. No new state and no
 * payload/native object read. Normal unleased serialize/decode stays unchanged.
 * The existing so_validate() and all_ok() inspect fixed owner/row/cache metadata
 * only; neither calls a foreign predicate, callback or image_write_allowed. */
extern U so_write_validate(U) __asm__("so_validate");
U __attribute__((noinline,section(".text.gz_so_write")))
so_active_write_apart(U pointer,U bytes)
{
    Lease *lease = &owner.lease;
    if (owner.magic != 0x534f5731u || owner.version != 1 || !owner.serial ||
        owner.inverse != ~owner.serial || owner.quarantine || lease->active > 1)
        return 0;
    if (!lease->active) return 1;
    if (so_write_validate(lease->serial)) return 0;
    return bytes <= 0xffffffffu-pointer && lease->capacity <= 0xffffffffu-lease->pointer &&
        (pointer+bytes <= lease->pointer || lease->pointer+lease->capacity <= pointer);
}
