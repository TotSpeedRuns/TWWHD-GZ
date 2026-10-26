/* Entered only at Inventory's navigation point, after Back and R shortcuts.
 * r29 is existing GZ metadata, r31 the current canonical controller sample.
 * Preserve the existing edge/repeat registers; no buttons are synthesized.
 */
.section .text,"ax",@progbits
.balign 4
.global _gz_ins_inventory_bridge
.type _gz_ins_inventory_bridge,@function
_gz_ins_inventory_bridge:
    stwu 1,-64(1)
    mflr 0
    stw 0,68(1)
    stw 3,8(1)
    stw 4,12(1)
    stw 5,16(1)
    stw 6,20(1)
    stw 7,24(1)
    stw 8,28(1)
    stw 9,32(1)
    stw 10,36(1)
    stw 11,40(1)
    stw 12,44(1)
    lwz 3,8(29)
    lwz 4,0(31)
    bl _gz_ins_inventory
    stw 3,8(29)
    lwz 3,8(1)
    lwz 4,12(1)
    lwz 5,16(1)
    lwz 6,20(1)
    lwz 7,24(1)
    lwz 8,28(1)
    lwz 9,32(1)
    lwz 10,36(1)
    lwz 11,40(1)
    lwz 12,44(1)
    lwz 0,68(1)
    mtlr 0
    addi 1,1,64
    /* Replay the one displaced authored instruction, then resume unchanged. */
    lwz 9,0x1c(1)
    b _gz_ins_inventory_resume
.size _gz_ins_inventory_bridge,.-_gz_ins_inventory_bridge
