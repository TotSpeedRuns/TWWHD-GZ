.section .text,"ax",@progbits
.balign 4
.global _gz_mnu_input_bridge
.type _gz_mnu_input_bridge,@function
/* Existing menu-input frame owns the canonical sample in r31 and edge word
 * at its SP+10. Preserve every volatile integer/status register at this seam;
 * the new C dispatcher is integer-only and never edits controller bytes.
 */
_gz_mnu_input_bridge:
    stwu 1,-96(1)
    stw 0,8(1)
    mflr 0
    stw 0,100(1)
    stw 3,12(1)
    stw 4,16(1)
    stw 5,20(1)
    stw 6,24(1)
    stw 7,28(1)
    stw 8,32(1)
    stw 9,36(1)
    stw 10,40(1)
    stw 11,44(1)
    stw 12,48(1)
    mfcr 0
    stw 0,52(1)
    mfctr 0
    stw 0,56(1)
    mfxer 0
    stw 0,60(1)
    lwz 3,0(31)
    lwz 4,112(1)
    bl _gz_mnu_tick
    lwz 3,12(1)
    lwz 4,16(1)
    lwz 5,20(1)
    lwz 6,24(1)
    lwz 7,28(1)
    lwz 8,32(1)
    lwz 9,36(1)
    lwz 10,40(1)
    lwz 11,44(1)
    lwz 12,48(1)
    lwz 0,56(1)
    mtctr 0
    lwz 0,60(1)
    mtxer 0
    lwz 0,52(1)
    mtcrf 255,0
    lwz 0,100(1)
    mtlr 0
    lwz 0,8(1)
    addi 1,1,96
    blr
.size _gz_mnu_input_bridge,.-_gz_mnu_input_bridge

.balign 4
.global _gz_mnu_inv_horizontal
/* Preserve all original amount/quest/controller paths. A plain D-pad Left
 * from the side panel is navigation. Holding L deliberately retains the
 * original decrease path. Analog intent never reaches these amount bits.
 */
_gz_mnu_inv_horizontal:
    cmpwi 10,24
    bge .Lmnu_side_panel
    b _gz_inv_far_8
.Lmnu_side_panel:
    andi. 9,8,0x0800
    bne .Lmnu_panel_left
    b _gz_inv_health_input
.Lmnu_panel_left:
    lwz 9,0(31)
    andi. 9,9,0x0020
    beq .Lmnu_panel_escape
    b _gz_inv_health_input
.Lmnu_panel_escape:
    cmpwi 10,24
    bne .Lmnu_panel_not_health
    li 10,5
    b .Lmnu_panel_store
.Lmnu_panel_not_health:
    cmpwi 10,25
    bne .Lmnu_panel_bottom
    li 10,17
    b .Lmnu_panel_store
.Lmnu_panel_bottom:
    li 10,23
.Lmnu_panel_store:
    stw 10,8(29)
    b _gz_mask
