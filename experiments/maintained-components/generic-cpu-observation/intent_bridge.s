.section .text,"ax",@progbits
.balign 4
.global _gz_gai_vpad_bridge
.type _gz_gai_vpad_bridge,@function
/* Called after the ORIGINAL real VPADRead and mr30,r3, before its ROH guard.
 * Outer native input frame: r31=returned buffer, r30=result count,
 * SP+0x10=requested count, SP+0x14=channel. Only channel0 GamePad is admitted.
 * Return r3=consume intent; every other volatile integer/status register is
 * restored. No source/native authority API is called by this bridge.
 */
_gz_gai_vpad_bridge:
 stwu 1,-96(1)
 stw 0,8(1)
 mflr 0
 stw 0,100(1)
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
 mr 3,31
 mr 4,30
 lwz 5,112(1)
 lwz 6,116(1)
 bl _gz_gai_view
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
.size _gz_gai_vpad_bridge,.-_gz_gai_vpad_bridge
