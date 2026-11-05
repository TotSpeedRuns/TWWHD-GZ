/* Actual EUR context fragments, not ordinary C ABI functions. */
.section .text.rando_quickdraw,"ax",@progbits
.globl _gz_rando_quickdraw
.type _gz_rando_quickdraw,@function
_gz_rando_quickdraw:

    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    bne .Lquick_fast

    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    beq .Lquick_skip
    b _gz_rando_quickdraw_original
.Lquick_skip:
    b _gz_rando_quickdraw_skip
.Lquick_fast:

    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 0,1
    cmpwi 0,1
    b _gz_rando_quickdraw_fast
.size _gz_rando_quickdraw,.-_gz_rando_quickdraw

.section .text.rando_demo_cleanup,"ax",@progbits
.globl _gz_rando_demo_cleanup
.type _gz_rando_demo_cleanup,@function
_gz_rando_demo_cleanup:

    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    bne .Lcleanup_skip

    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    beq .Lcleanup_forced
    b _gz_rando_demo_cleanup_original
.Lcleanup_forced:
    b _gz_rando_demo_cleanup_forced
.Lcleanup_skip:

    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_demo_cleanup_skip
.size _gz_rando_demo_cleanup,.-_gz_rando_demo_cleanup

.section .text.rando_tact_falling,"ax",@progbits
.globl _gz_rando_tact_falling
.type _gz_rando_tact_falling,@function
_gz_rando_tact_falling:

    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    bne .Lfalling_nop

    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    beq .Lfalling_original_skip
    b _gz_rando_tact_falling_resume
.Lfalling_original_skip:
    b _gz_rando_tact_falling_skip
.Lfalling_nop:

    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_tact_falling_resume
.size _gz_rando_tact_falling,.-_gz_rando_tact_falling

.type _gz_rando_tools_flags,@object
.type _gz_rando_quickdraw_original,@function
.type _gz_rando_quickdraw_fast,@function
.type _gz_rando_quickdraw_skip,@function
.type _gz_rando_demo_cleanup_original,@function
.type _gz_rando_demo_cleanup_forced,@function
.type _gz_rando_demo_cleanup_skip,@function
.type _gz_rando_tact_falling_resume,@function
.type _gz_rando_tact_falling_skip,@function
