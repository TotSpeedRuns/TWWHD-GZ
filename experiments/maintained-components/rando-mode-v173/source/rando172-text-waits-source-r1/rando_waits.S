/* Native tag-case contexts. No resource writes or new object ownership. */
.section .text._gz_rando_wait_tag_2,"ax",@progbits
.globl _gz_rando_wait_tag_2
.type _gz_rando_wait_tag_2,@function
_gz_rando_wait_tag_2:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)

    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Loriginal_2
    lhz 0,0(6)
    cmpwi 0,14
    bne .Loriginal_2
    lhz 0,2(6)
    cmpwi 0,1
    bne .Loriginal_2
    lhz 0,4(6)
    cmpwi 0,2
    bne .Loriginal_2
    lhz 0,6(6)
    cmpwi 0,2
    bne .Loriginal_2

    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 3,0
    blr
.Loriginal_2:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_wait_tag_2_native
.size _gz_rando_wait_tag_2,.-_gz_rando_wait_tag_2
.section .text._gz_rando_wait_tag_6,"ax",@progbits
.globl _gz_rando_wait_tag_6
.type _gz_rando_wait_tag_6,@function
_gz_rando_wait_tag_6:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)

    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Loriginal_6
    lhz 0,0(6)
    cmpwi 0,14
    bne .Loriginal_6
    lhz 0,2(6)
    cmpwi 0,1
    bne .Loriginal_6
    lhz 0,4(6)
    cmpwi 0,6
    bne .Loriginal_6
    lhz 0,6(6)
    cmpwi 0,2
    bne .Loriginal_6

    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 3,0
    blr
.Loriginal_6:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_wait_tag_6_native
.size _gz_rando_wait_tag_6,.-_gz_rando_wait_tag_6
.type _gz_rando_tools_flags,@object
.type _gz_rando_wait_tag_2_native,@function
.type _gz_rando_wait_tag_6_native,@function
