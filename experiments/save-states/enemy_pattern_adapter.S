/* Soft-float private C -> exact native hard-float reuse initializer.
 * This adapter grants no lifetime/owner permission and is not called by any
 * normal restore entry. A future typed held participant must own all inputs.
 * Input: r3 face, r4 model wrapper, r5 current pattern wrapper, r6 frame u8.
 */
.section .text,"ax",@progbits
.balign 4
.global _gz_ept_native_reuse
.type _gz_ept_native_reuse,@function
_gz_ept_native_reuse:
 stwu 1,-96(1)
 mflr 0
 stw 0,100(1)
 stw 30,88(1)
 stw 31,92(1)
 mr 30,3
 mr 31,4
 stw 6,12(1)
 li 0,0
 stw 0,8(1)
 lis 0,0x3f80
 stw 0,16(1)
 lfs 1,16(1)
 li 6,1
 li 7,2
 li 8,0
 li 9,-1
 li 10,1
 bl _gz_ept_native_init
 cmpwi 3,0
 beq 1f
 mr 3,30
 mr 4,31
 lwz 5,12(1)
 bl _gz_ept_native_entry
 li 3,1
1:
 lwz 30,88(1)
 lwz 31,92(1)
 lwz 0,100(1)
 mtlr 0
 addi 1,1,96
 blr
.size _gz_ept_native_reuse,.-_gz_ept_native_reuse
