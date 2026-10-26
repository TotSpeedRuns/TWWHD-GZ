/* The two qualified practice-travel call sites already passed their original
 * ROH/live/index/busy/admission gates. Do not call this from capture, restore,
 * draw, or a frame tick: captured cutscene state must keep its own semantics.
 *
 * Native ShieldNoDraw/SwordNoDraw read event3F40 from *heap_slot+0644.
 * The native offEventBit leaf clears only byte3F's mask40: root+0683.
 * Use the existing relocated native DATA alias, never a historical actor or
 * model pointer. The current game-info root is checked before the sole byte
 * read/store. No allocator/model refresh, event skip, or native API is called.
 *
 * This leaf preserves all incoming GPRs, fullCR, LR, CTR and XER. No FP/GQR
 * instruction is used. In particular raw warp's live r8/r9 are untouched.
 */
.section .text,"ax",@progbits
.balign 4
.global _gz_practice_gear_travel
_gz_practice_gear_travel:
    stwu 1,-32(1)
    stw 0,8(1)
    stw 11,12(1)
    stw 12,16(1)
    mfcr 0
    stw 0,20(1)

    lis 12,_gz_heap_slot@ha
    lwz 12,_gz_heap_slot@l(12)
    lis 11,0x1000
    cmplw 12,11
    blt _gz_practice_gear_travel_done
    /* This upper bound leaves more than0684 bytes before DATA's end. */
    lis 11,0xBFFF
    cmplw 12,11
    bge _gz_practice_gear_travel_done
    andi. 11,12,3
    bne _gz_practice_gear_travel_done

    lbz 11,0x683(12)
    andi. 0,11,0x40
    beq _gz_practice_gear_travel_done
    andi. 11,11,0xBF
    stb 11,0x683(12)

_gz_practice_gear_travel_done:
    lwz 0,20(1)
    mtcrf 255,0
    lwz 0,8(1)
    lwz 11,12(1)
    lwz 12,16(1)
    addi 1,1,32
    blr
