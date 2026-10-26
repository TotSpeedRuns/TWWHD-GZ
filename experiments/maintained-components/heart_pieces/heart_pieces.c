/* Heart-piece progression edit; native meter owns normalization and refill.
 * No item-give callback, reward/statistics flag, direct HP/max-HP or HUD store.
 */
#include "heart_pieces.h"
typedef HpcU U;
typedef HpcByte Byte;
extern volatile U _gz_state[], _gz_page, _gz_heap_slot;
extern volatile Byte *volatile _gz_route_heap;
extern U _gz_roh_user_guard(void), _gz_live_check(void), _gz_route_heap_check(void);
extern volatile Byte *_gz_btl_native_play(void);
/* Exact existing R4 whole-entry/relocation/current-root validation. */
#include "bottle_play.h"

static __attribute__((noinline,noclone)) U hpc_next_max(U maximum)
{
    U whole, piece, target;
    if(maximum<4u||maximum>80u)
        return 0;
    whole=maximum&~3u;
    piece=(maximum+1u)&3u;
    target=whole|piece;
    return target<=80u?target:maximum;
}

static __attribute__((noinline,noclone)) U hpc_ui_current(volatile Byte *save)
{
    return _gz_state[0]&&_gz_page==HPC_INVENTORY_PAGE&&
        _gz_state[2]==HPC_CONTAINER_ROW&&btl_save_current(save);
}

/* Pure scalar predicate, shared by the real current-field reader below.
 * A true result alone never grants a native pointer or write capability.
 */
static __attribute__((noinline,noclone)) U hpc_quiet_values(U mode,U battle,
                                                         U pending_max,U life_bits)
{
    return mode!=6u&&!battle&&!pending_max&&!(life_bits&0x7fffffffu);
}

static U hpc_quiet(volatile Byte *play,U delta)
{
    U battle=0x101ea078u+delta;
    U life_bits;
    /* Actual EUR meter skips ordinary life processing in minigame6 and
     * while restoring its battle-heart state. Never enqueue behind that.
     */
    if(!btl_data_root(battle))
        return 0;
    /* Both IEEE754 signed zeros are quiet; NaN/Inf/any delta refuses.
     * Byte access avoids a float/integer alias and never consumes a pickup.
     */
    life_bits=((U)play[0x5b44]<<24)|((U)play[0x5b45]<<16)|
        ((U)play[0x5b46]<<8)|play[0x5b47];
    return hpc_quiet_values(play[0x5cea],*(volatile Byte*)battle,
                           *(volatile unsigned short*)(play+0x5b5e),life_bits);
}

U _gz_hpc_apply(volatile Byte *save)
{
    BtlPlayRoots roots;
    volatile Byte *play;
    U maximum,target,delta;
    if(_gz_roh_user_guard()||_gz_live_check()||_gz_route_heap_check()||
       !hpc_ui_current(save)||!btl_play_roots(&roots))
        return 0;
    /* A cold call is refused by the existing full-entry gate above. The
     * genuine warm native getter supplies Play; no supplied API outcome.
     */
    play=_gz_btl_native_play();
    if(!btl_play_current(play,&roots))
        return 0;
    delta=(U)&_gz_heap_slot-0x101f84f4u;
    if(!hpc_quiet(play,delta))
        return 0;
    maximum=*(volatile unsigned short*)save;
    target=hpc_next_max(maximum);
    if(!target||target==maximum)
        return 0;
    /* Keep every current gate fresh immediately before this sole store.
     * The ordinary game callback serializes this with the native meter;
     * this metadata check is not a general concurrent native-writer lock.
     */
    if(_gz_roh_user_guard()||_gz_live_check()||_gz_route_heap_check()||
       !hpc_ui_current(save)||!btl_play_current(play,&roots)||
       *(volatile unsigned short*)save!=maximum||!hpc_quiet(play,delta))
        return 0;
    *(volatile short*)(play+0x5b5e)=(short)((int)target-(int)maximum);
    return 1;
}
