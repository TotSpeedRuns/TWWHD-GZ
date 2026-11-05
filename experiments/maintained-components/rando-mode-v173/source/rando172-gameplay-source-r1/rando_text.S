/* Genuine EUR context fragments, not C-call interfaces. */
.section .text._gz_rando_text_held_0,"ax",@progbits
.globl _gz_rando_text_held_0
.type _gz_rando_text_held_0,@function
_gz_rando_text_held_0:
    stwu 1,-16(1)
    stw 11,4(1)
    stw 12,8(1)
    mfcr 11
    stw 11,12(1)
    lis 12,_gz_rando_tools_flags@ha
    lwz 11,_gz_rando_tools_flags@l(12)
    andi. 11,11,8192

    beq .Loriginal_0
    lwz 0,80(30)
    b .Lrestore_0
.Loriginal_0:
    lwz 0,84(30)
.Lrestore_0:
    lwz 11,12(1)
    mtcrf 255,11
    lwz 12,8(1)
    lwz 11,4(1)
    addi 1,1,16

    b _gz_rando_text_held_0_resume
.size _gz_rando_text_held_0,.-_gz_rando_text_held_0

.section .text._gz_rando_text_held_1,"ax",@progbits
.globl _gz_rando_text_held_1
.type _gz_rando_text_held_1,@function
_gz_rando_text_held_1:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loriginal_1
    lwz 10,80(8)
    b .Lrestore_1
.Loriginal_1:
    lwz 10,84(8)
.Lrestore_1:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_text_held_1_resume
.size _gz_rando_text_held_1,.-_gz_rando_text_held_1

.section .text._gz_rando_text_held_2,"ax",@progbits
.globl _gz_rando_text_held_2
.type _gz_rando_text_held_2,@function
_gz_rando_text_held_2:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loriginal_2
    lwz 8,80(30)
    b .Lrestore_2
.Loriginal_2:
    lwz 8,84(30)
.Lrestore_2:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_text_held_2_resume
.size _gz_rando_text_held_2,.-_gz_rando_text_held_2

.section .text._gz_rando_text_held_3,"ax",@progbits
.globl _gz_rando_text_held_3
.type _gz_rando_text_held_3,@function
_gz_rando_text_held_3:
    stwu 1,-16(1)
    stw 11,4(1)
    stw 12,8(1)
    mfcr 11
    stw 11,12(1)
    lis 12,_gz_rando_tools_flags@ha
    lwz 11,_gz_rando_tools_flags@l(12)
    andi. 11,11,8192

    beq .Loriginal_3
    lwz 0,80(30)
    b .Lrestore_3
.Loriginal_3:
    lwz 0,84(30)
.Lrestore_3:
    lwz 11,12(1)
    mtcrf 255,11
    lwz 12,8(1)
    lwz 11,4(1)
    addi 1,1,16

    b _gz_rando_text_held_3_resume
.size _gz_rando_text_held_3,.-_gz_rando_text_held_3

.section .text._gz_rando_text_held_4,"ax",@progbits
.globl _gz_rando_text_held_4
.type _gz_rando_text_held_4,@function
_gz_rando_text_held_4:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 10,8(1)
    mfcr 0
    stw 0,12(1)
    lis 10,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(10)
    andi. 0,0,8192

    beq .Loriginal_4
    lwz 12,80(11)
    b .Lrestore_4
.Loriginal_4:
    lwz 12,84(11)
.Lrestore_4:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 10,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_text_held_4_resume
.size _gz_rando_text_held_4,.-_gz_rando_text_held_4

.section .text._gz_rando_text_held_5,"ax",@progbits
.globl _gz_rando_text_held_5
.type _gz_rando_text_held_5,@function
_gz_rando_text_held_5:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loriginal_5
    lwz 9,80(30)
    b .Lrestore_5
.Loriginal_5:
    lwz 9,84(30)
.Lrestore_5:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_text_held_5_resume
.size _gz_rando_text_held_5,.-_gz_rando_text_held_5

.section .text._gz_rando_text_held_6,"ax",@progbits
.globl _gz_rando_text_held_6
.type _gz_rando_text_held_6,@function
_gz_rando_text_held_6:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loriginal_6
    lwz 9,80(30)
    b .Lrestore_6
.Loriginal_6:
    lwz 9,84(30)
.Lrestore_6:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_text_held_6_resume
.size _gz_rando_text_held_6,.-_gz_rando_text_held_6

.section .text._gz_rando_text_held_7,"ax",@progbits
.globl _gz_rando_text_held_7
.type _gz_rando_text_held_7,@function
_gz_rando_text_held_7:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loriginal_7
    lwz 8,80(30)
    b .Lrestore_7
.Loriginal_7:
    lwz 8,84(30)
.Lrestore_7:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_text_held_7_resume
.size _gz_rando_text_held_7,.-_gz_rando_text_held_7

.section .text._gz_rando_text_held_8,"ax",@progbits
.globl _gz_rando_text_held_8
.type _gz_rando_text_held_8,@function
_gz_rando_text_held_8:
    stwu 1,-16(1)
    stw 11,4(1)
    stw 12,8(1)
    mfcr 11
    stw 11,12(1)
    lis 12,_gz_rando_tools_flags@ha
    lwz 11,_gz_rando_tools_flags@l(12)
    andi. 11,11,8192

    beq .Loriginal_8
    lwz 0,80(3)
    b .Lrestore_8
.Loriginal_8:
    lwz 0,84(3)
.Lrestore_8:
    lwz 11,12(1)
    mtcrf 255,11
    lwz 12,8(1)
    lwz 11,4(1)
    addi 1,1,16

    b _gz_rando_text_held_8_resume
.size _gz_rando_text_held_8,.-_gz_rando_text_held_8

.section .text._gz_rando_text_icon_0,"ax",@progbits
.globl _gz_rando_text_icon_0
.type _gz_rando_text_icon_0,@function
_gz_rando_text_icon_0:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    bne .Lforce_icon_0
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    beq .Lskip_icon_0
    b _gz_rando_text_icon_0_resume
.Lskip_icon_0:
    b _gz_rando_text_icon_0_skip
.Lforce_icon_0:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_text_icon_0_resume
.size _gz_rando_text_icon_0,.-_gz_rando_text_icon_0

.section .text._gz_rando_text_icon_1,"ax",@progbits
.globl _gz_rando_text_icon_1
.type _gz_rando_text_icon_1,@function
_gz_rando_text_icon_1:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    bne .Lforce_icon_1
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    beq .Lskip_icon_1
    b _gz_rando_text_icon_1_resume
.Lskip_icon_1:
    b _gz_rando_text_icon_1_skip
.Lforce_icon_1:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_text_icon_1_resume
.size _gz_rando_text_icon_1,.-_gz_rando_text_icon_1

.type _gz_rando_tools_flags,@object
.type _gz_rando_text_held_0_resume,@function
.type _gz_rando_text_held_1_resume,@function
.type _gz_rando_text_held_2_resume,@function
.type _gz_rando_text_held_3_resume,@function
.type _gz_rando_text_held_4_resume,@function
.type _gz_rando_text_held_5_resume,@function
.type _gz_rando_text_held_6_resume,@function
.type _gz_rando_text_held_7_resume,@function
.type _gz_rando_text_held_8_resume,@function
.type _gz_rando_text_icon_0_resume,@function
.type _gz_rando_text_icon_0_skip,@function
.type _gz_rando_text_icon_1_resume,@function
.type _gz_rando_text_icon_1_skip,@function
