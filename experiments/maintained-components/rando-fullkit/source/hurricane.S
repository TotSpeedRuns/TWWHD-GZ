/* Separate, wider borrowed field: only save+68D byte, event6901. Caller must
 * hold current observed root/save ownership for this named byte; core's256B
 * inventory borrow alone does not authorize this call. Actual Rando read hook
 * selects6901 only while Mode13, original Orca0B20 and temp0402 otherwise.
 */
.section .text.rando_grant_hurricane,"ax",@progbits
.globl _gz_rando_grant_hurricane
.type _gz_rando_grant_hurricane,@function
_gz_rando_grant_hurricane:
    lbz 8,1677(28)
    andi. 8,8,1
    bne .Lowned
    stwu 1,-16(1)
    mflr 0
    stw 0,20(1)
    addi 3,28,1572
    li 4,26881
    bl _gz_rando_event_set
    lwz 0,20(1)
    mtlr 0
    addi 1,1,16
.Lowned:
    lbz 3,1677(28)
    andi. 3,3,1
    xori 3,3,1
    blr
.size _gz_rando_grant_hurricane,.-_gz_rando_grant_hurricane
.type _gz_rando_event_set,@function
