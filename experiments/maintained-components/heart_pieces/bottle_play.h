/* Exact whole GetPlay validation, including the loader's DATA relocations.
 * The existing native heap-slot alias anchors the same RPX DATA allocation.
 * Opcode/register fields, branches and non-relocated words remain exact.
 */
#ifndef GZ_BOTTLE_PLAY_H
#define GZ_BOTTLE_PLAY_H

typedef struct BtlPlayRoots {
    U play;
    U initialized;
} BtlPlayRoots;

/* Generated locally from your exact owned EUR v0 input. */
#include "bottle_play_profile.inc"

static U btl_data_root(U address)
{
    return address >= 0x10000000u && address < 0xbfff0000u && !(address & 3u);
}

static U btl_decode_address(U high, U low)
{
    return ((high & 0xffffu) << 16) + (U)(int)(short)(low & 0xffffu);
}

static U btl_play_expected_word(U index, U delta)
{
    U address, high;

    /* These seven halfwords are the complete native entry's type4/type6
     * relocation set. All target native symbol2 (the common DATA base).
     * Recompute their exact expected values; do not mask arbitrary bits.
     */
    switch (index) {
    case 4: address = 0x104753a8u; high = 1; break;
    case 5: address = 0x104753a8u; high = 0; break;
    case 7: address = 0x1046f0b0u; high = 1; break;
    case 10: address = 0x1046f0b0u; high = 0; break;
    case 13: address = 0x101d5de8u; high = 1; break;
    case 14: address = 0x101d5de8u; high = 0; break;
    case 16: address = 0x1046f0b0u; high = 0; break;
    default: return btl_play_profile[index];
    }
    address += delta;
    return (btl_play_profile[index] & 0xffff0000u) |
        (high ? ((address + 0x8000u) >> 16) & 0xffffu : address & 0xffffu);
}

static U btl_play_roots(BtlPlayRoots *roots)
{
    const volatile U *code = (const volatile U *)(U)_gz_btl_native_play;
    U index, play, initialized, anchor = (U)&_gz_heap_slot;
    U delta = anchor - 0x101f84f4u;

    if (!btl_data_root(anchor))
        return 0;
    for (index = 0; index < 22; index++) {
        if (code[index] != btl_play_expected_word(index, delta))
            return 0;
    }
    play = btl_decode_address(code[7], code[16]);
    initialized = btl_decode_address(code[4], code[5]);
    if (!btl_data_root(play) || !btl_data_root(initialized) ||
        !btl_data_root(btl_decode_address(code[13], code[14])))
        return 0;
    /* A cold GetPlay call constructs unrelated native state and calls out.
     * The editor is permitted to use an already-initialized current Play only.
     */
    if (!*(const volatile U *)initialized)
        return 0;
    roots->play = play;
    roots->initialized = initialized;
    return 1;
}

static U btl_save_current(volatile Byte *save)
{
    return save && (U)save == _gz_heap_slot + 0x20u && save == _gz_route_heap;
}

static U btl_play_current(volatile Byte *play, const BtlPlayRoots *expected)
{
    BtlPlayRoots current;

    return btl_play_roots(&current) && current.play == expected->play &&
        current.initialized == expected->initialized &&
        btl_data_root((U)play) && (U)play == current.play;
}

/* Pure current-metadata tail: no native API invocation or supplied result.
 * Production reaches this only after its whole GetPlay call and native fill.
 */
static U btl_start_positive_timer(volatile Byte *save, volatile Byte *play,
                                  const BtlPlayRoots *expected)
{
    if (!btl_save_current(save) || !btl_play_current(play, expected))
        return 0;
    if (!*(volatile unsigned short *)(save + 0x66))
        return 0;
    play[0x5be0] = 1;
    return 1;
}

#endif
