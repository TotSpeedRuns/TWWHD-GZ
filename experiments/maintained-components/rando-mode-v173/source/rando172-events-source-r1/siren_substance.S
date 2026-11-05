/* Exact existing getMySubstance provider call ABI, not a new event owner. */
.section .text.rando_siren_substance,"ax",@progbits
.globl _gz_rando_siren_substance
.type _gz_rando_siren_substance,@function
_gz_rando_siren_substance:

    stwu 1,-32(1)
    stw 0,4(1)
    stw 8,8(1)
    stw 9,12(1)
    mfcr 0
    stw 0,16(1)

    lis 8,_gz_rando_tools_flags@ha
    lwz 9,_gz_rando_tools_flags@l(8)
    andi. 9,9,8192
    beq .Levent_original
    lis 8,_gz_rando_static_event_manager@ha
    addi 8,8,_gz_rando_static_event_manager@l
    cmpw 3,8
    bne .Levent_original
    lis 8,_gz_rando_current_stage@ha
    lwz 9,_gz_rando_current_stage@l(8)
    lis 0,0x5369
    ori 0,0,0x7265
    cmpw 9,0
    bne .Levent_original
    lwz 9,_gz_rando_current_stage+4@l(8)
    lis 0,0x6e00
    cmpw 9,0
    bne .Levent_original

    lwz 0,16(1)
    mtcrf 255,0
    lwz 9,12(1)
    lwz 8,8(1)
    lwz 0,4(1)
    addi 1,1,32

    b .Levent_siren
.Levent_original:

    lwz 0,16(1)
    mtcrf 255,0
    lwz 9,12(1)
    lwz 8,8(1)
    lwz 0,4(1)
    addi 1,1,32

    b _gz_rando_original_substance
.Levent_siren:
    stwu 1,-96(1)
    stw 3,8(1)
    stw 4,12(1)
    stw 5,16(1)
    mflr 0
    stw 0,20(1)
    bl _gz_rando_original_substance
    /* Genuine provider result first; never invent successful validation. */
    stw 3,28(1)
    stw 4,32(1)
    stw 5,36(1)
    stw 6,40(1)
    stw 7,44(1)
    stw 8,48(1)
    stw 9,52(1)
    stw 10,56(1)
    stw 11,60(1)
    stw 12,64(1)
    mfctr 0
    stw 0,68(1)
    mfcr 0
    stw 0,72(1)
    /* Every native positive return leaves r0=its incoming savedLR. The
       nested call has a different rawLR; restore the real caller's one. */
    lwz 0,20(1)
    stw 0,24(1)
    cmpwi 3,0
    beq .Levent_done
    lwz 4,12(1)
    lwz 3,32(4)
    cmplwi 3,1029
    bge .Levent_done
    lwz 8,8(1)
    lwz 8,16(8)
    slwi 9,3,6
    add 8,8,9
    cmpw 8,4
    bne .Levent_done
    lwz 5,40(4)
    lwz 6,44(4)
    cmpwi 6,1
    bne .Levent_done
    lwz 7,0(4)
    lwz 8,4(4)
    lwz 4,36(4)
    cmpwi 4,0
    beq .Levent_scalar
    cmpwi 4,3
    bne .Levent_done
.Levent_scalar:
    lwz 9,28(1)
    lwz 9,0(9)
    bl _gz_rando_siren_value
    cmpwi 3,0
    beq .Levent_done
    stw 3,28(1)
.Levent_done:
    lwz 0,72(1)
    mtcrf 255,0
    lwz 0,68(1)
    mtctr 0
    lwz 0,20(1)
    mtlr 0
    lwz 12,64(1)
    lwz 11,60(1)
    lwz 10,56(1)
    lwz 9,52(1)
    lwz 8,48(1)
    lwz 7,44(1)
    lwz 6,40(1)
    lwz 5,36(1)
    lwz 4,32(1)
    lwz 3,28(1)
    lwz 0,24(1)
    addi 1,1,96
    blr
.size _gz_rando_siren_substance,.-_gz_rando_siren_substance
.type _gz_rando_tools_flags,@object
.type _gz_rando_static_event_manager,@object
.type _gz_rando_current_stage,@object
.type _gz_rando_original_substance,@function
.type _gz_rando_siren_value,@function

