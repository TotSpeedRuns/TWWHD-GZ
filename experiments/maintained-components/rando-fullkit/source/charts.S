/* Native chart item IDs C2..FE select chart61..1. Grant missing ownership only:
 * native item-get also clears the new chart's two state bits; existing chart
 * progress is retained by never invoking its provider when already owned.
 * All accesses are inside the existing first256B save record borrow.
 */
.section .text.rando_grant_charts,"ax",@progbits
.globl _gz_rando_grant_charts
.type _gz_rando_grant_charts,@function
_gz_rando_grant_charts:
    stwu 1,-64(1)
    mflr 0
    stw 0,68(1)
    stmw 29,52(1)
    li 29,0
.Lchart:
    srwi 8,29,5
    slwi 8,8,2
    addi 8,8,212
    lwzx 30,28,8
    /* Native chart bit mask rotates modulo32, unlike slw's6-bit count. */
    li 31,1
    rotlw 31,31,29
    and. 8,30,31
    bne .Lnext
    li 3,254
    subf 3,29,3
    bl _gz_rando_kit_native_item_get
    srwi 8,29,5
    slwi 8,8,2
    addi 8,8,212
    lwzx 8,28,8
    and. 8,8,31
    beq .Lfail
.Lnext:
    addi 29,29,1
    cmpwi 29,61
    blt .Lchart
    li 3,0
    b .Ldone
.Lfail:
    li 3,1
.Ldone:
    lmw 29,52(1)
    lwz 0,68(1)
    mtlr 0
    addi 1,1,64
    blr
.size _gz_rando_grant_charts,.-_gz_rando_grant_charts
.type _gz_rando_kit_native_item_get,@function
