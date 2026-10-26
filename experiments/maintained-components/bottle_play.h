/* Private metadata validation for the exact whole retail GetPlay entry.
 * No new native DATA alias: both roots are decoded from verified instructions.
 */
#ifndef GZ_BOTTLE_PLAY_H
#define GZ_BOTTLE_PLAY_H

typedef struct BtlPlayRoots {
    U play;
    U initialized;
} BtlPlayRoots;

/* Retail instruction bytes are generated from an owned input into ignored local-assets/. */
#include "bottle_play_profile.inc"

static U btl_data_root(U address)
{
    return address >= 0x10000000u && address < 0xbfff0000u && !(address & 3u);
}

static U btl_decode_address(U high, U low)
{
    return ((high & 0xffffu) << 16) + (U)(int)(short)(low & 0xffffu);
}

static U btl_play_roots(BtlPlayRoots *roots)
{
    const volatile U *code = (const volatile U *)(U)_gz_btl_native_play;
    U index, play, initialized;

    for (index = 0; index < 22; index++) {
        if (code[index] != btl_play_profile[index])
            return 0;
    }
    play = btl_decode_address(code[7], code[16]);
    initialized = btl_decode_address(code[4], code[5]);
    if (!btl_data_root(play) || !btl_data_root(initialized))
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
