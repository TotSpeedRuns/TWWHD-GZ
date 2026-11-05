.section .text.rando_type3,"ax",@progbits
.globl _gz_rando_selected_type3
.type _gz_rando_selected_type3,@function
_gz_rando_selected_type3:

    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)

    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Lnative
    lhz 0,0(6)
    cmpwi 0,14
    bne .Lnative
    lhz 0,2(6)
    cmpwi 0,1
    bne .Lnative
    lhz 0,4(6)
    cmpwi 0,3
    bne .Lnative
    lhz 0,6(6)
    cmpwi 0,2
    bne .Lnative
    lwz 11,0(3)
    lwz 0,0x11c(11)
    clrlwi 0,0,16

    cmpwi 0,2034
    beq .Lskip
    cmpwi 0,2039
    beq .Lskip
    cmpwi 0,2040
    beq .Lskip
    cmpwi 0,3522
    beq .Lskip
    cmpwi 0,4316
    beq .Lskip
    cmpwi 0,3956
    beq .Lskip
    cmpwi 0,5162
    beq .Lskip
    cmpwi 0,7416
    beq .Lskip
    cmpwi 0,9935
    beq .Lskip
    cmpwi 0,12920
    beq .Lskip
    cmpwi 0,10930
    beq .Lskip
    b .Lnative
.Lskip:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 3,0
    blr
.Lnative:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_selected_type3_native
.size _gz_rando_selected_type3,.-_gz_rando_selected_type3
.type _gz_rando_tools_flags,@object
.type _gz_rando_selected_type3_native,@function
