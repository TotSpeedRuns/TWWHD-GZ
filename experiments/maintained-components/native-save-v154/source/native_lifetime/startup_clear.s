/* Initial game loader may leave user BSS to the application. Explicitly
 * establish only the payload zero runs before CRT/observer construction.
 * No OS APIs, allocation, native pointers or initialized DATA are accessed.
 * Preserve the incoming ABI state; replay the entry's displaced mflr r0.
 */
.text
.align 2
.global _gz_startup_clear
.type _gz_startup_clear,@function
_gz_startup_clear:
 stwu 1,-96(1)
 stw 0,8(1)
 mflr 0
 stw 0,12(1)
 mfcr 0
 stw 0,16(1)
 mfctr 0
 stw 0,20(1)
 mfxer 0
 stw 0,24(1)
 stw 3,28(1)
 stw 4,32(1)
 stw 5,36(1)
 li 0,0
 lis 3,_gz_startup_zero_0@ha
 addi 3,3,_gz_startup_zero_0@l
 lis 4,0xd
 ori 4,4,0xb080
 mtctr 4
.Lstartup_zero_0:
 stw 0,0(3)
 stw 0,4(3)
 stw 0,8(3)
 stw 0,12(3)
 stw 0,16(3)
 stw 0,20(3)
 stw 0,24(3)
 stw 0,28(3)
 addi 3,3,32
 bdnz .Lstartup_zero_0
 sync
 lwz 3,28(1)
 lwz 4,32(1)
 lwz 5,36(1)
 lwz 0,24(1)
 mtxer 0
 lwz 0,20(1)
 mtctr 0
 lwz 0,16(1)
 mtcr 0
 lwz 0,12(1)
 mtlr 0
 lwz 0,8(1)
 addi 1,1,96
 mflr 0
 b _gz_startup_entry_resume
.size _gz_startup_clear,.-_gz_startup_clear
