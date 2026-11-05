/* Grouped Rando Mode contexts for native171; replaces Dry Storage row10.
 * No state owner, physical-address constant, or admission-result provider.
 * UI: r10 is the selected Tools row, after the existing confirmation checks.
 * Consumer: r27 is the consumed request, r28 is the existing guarded save base.
 * Neither exported entry is an ordinary C U(void) function.
 */
.section .text.rando_tools_select,"ax",@progbits
.globl _gz_rando_tools_select
.type _gz_rando_tools_select,@function
_gz_rando_tools_select:
    cmpwi 10,10
    beq .Lqueue
    /* Replay the displaced real 02905930 instruction exactly. */
    slwi 11,10,2
    b _gz_rando_tools_xor_resume
.Lqueue:
    lis 9,_gz_rando_inv_pending@ha
    lwz 10,_gz_rando_inv_pending@l(9)
    cmpwi 10,0
    bne .Luidone
    lis 8,_gz_rando_tools_flags@ha
    lwz 10,_gz_rando_tools_flags@l(8)
    andi. 10,10,8192
    li 10,949
    beq .Lenqueue
    li 10,950
.Lenqueue:
    stw 10,_gz_rando_inv_pending@l(9)
.Luidone:
    b _gz_rando_ui_done
.size _gz_rando_tools_select,.-_gz_rando_tools_select

.section .text.rando_inventory_dispatch,"ax",@progbits
.globl _gz_rando_inventory_dispatch
.type _gz_rando_inventory_dispatch,@function
_gz_rando_inventory_dispatch:
    cmpwi 27,949
    beq .Lenable
    cmpwi 27,950
    beq .Ldisable
    b _gz_rando_original_inventory_extension
.Lenable:
    lis 9,_gz_rando_tools_flags@ha
    lwz 10,_gz_rando_tools_flags@l(9)
    andi. 10,10,8192
    bne .Linvdone
    /* Genuine existing 92B grant consumes r27/r28, has no result status.
     * Original inventory guards precede this context. Do not queue 922 then
     * infer success: execute the actual grant before publishing the mode bit.
     */
    stwu 1,-16(1)
    mflr 0
    stw 0,20(1)
    stw 27,8(1)
    li 27,922
    bl _gz_rando_existing_song_apply
    lwz 27,8(1)
    lwz 0,20(1)
    mtlr 0
    addi 1,1,16
    sync
    lis 9,_gz_rando_tools_flags@ha
    lwz 10,_gz_rando_tools_flags@l(9)
    ori 10,10,8192
    stw 10,_gz_rando_tools_flags@l(9)
    sync
    b .Linvdone
.Ldisable:
    lis 9,_gz_rando_tools_flags@ha
    lwz 10,_gz_rando_tools_flags@l(9)
    andi. 8,10,8192
    beq .Linvdone
    li 8,-8193
    and 10,10,8
    stw 10,_gz_rando_tools_flags@l(9)
    sync
.Linvdone:
    b _gz_rando_inventory_done
.size _gz_rando_inventory_dispatch,.-_gz_rando_inventory_dispatch

.type _gz_rando_tools_flags,@object
.type _gz_rando_inv_pending,@object
.type _gz_rando_tools_xor_resume,@function
.type _gz_rando_ui_done,@function
.type _gz_rando_original_inventory_extension,@function
.type _gz_rando_inventory_done,@function
.type _gz_rando_existing_song_apply,@function
