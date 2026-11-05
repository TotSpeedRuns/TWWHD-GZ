/* Authored current-EUR create contexts. Tables from pinned MIT HD randomizer
 * cb7b1c979ac031c8eae91e23fb00fc5f4d042efa/make_game_nonlinear.asm.
 * No item/entrance shuffle, new actor allocation or story eligibility bypass.
 */
.section .text.rando_companion_row,"ax",@progbits
.globl _gz_rando_companion_row
.type _gz_rando_companion_row,@function
/* Ordinary scalar ABI: r3stageWord0,r4stageWord1,r5spawn,r6kind -> r3row/0.
 * No caller pointer, mode word, Game owner or SDK provider is accepted here.
 */
_gz_rando_companion_row:
    cmpwi 6,1
    beq .Lmakar_table
    cmpwi 6,2
    bne .Lrow_none
    lis 0,0x4d5f
    ori 0,0,0x4461
    cmpw 3,0
    bne .Lrow_none
    lis 0,0x6900
    cmpw 4,0
    bne .Lrow_none
    lis 9,_gz_rando_medli_positions@ha
    addi 9,9,_gz_rando_medli_positions@l
    b .Lrow_search
.Lmakar_table:
    lis 0,0x6b61
    ori 0,0,0x7a65
    cmpw 3,0
    bne .Lrow_none
    cmpwi 4,0
    bne .Lrow_none
    lis 9,_gz_rando_makar_positions@ha
    addi 9,9,_gz_rando_makar_positions@l
.Lrow_search:
    li 0,7
    mtctr 0
.Lrow_loop:
    lbz 8,0(9)
    cmpw 8,5
    beq .Lrow_found
    addi 9,9,16
    bdnz .Lrow_loop
.Lrow_none:
    li 3,0
    blr
.Lrow_found:
    mr 3,9
    blr
.size _gz_rando_companion_row,.-_gz_rando_companion_row

.section .text._gz_rando_makar_gate,"ax",@progbits
.globl _gz_rando_makar_gate
.type _gz_rando_makar_gate,@function
_gz_rando_makar_gate:
    stwu 1,-64(1)
    stw 0,4(1)
    stw 3,8(1)
    stw 4,12(1)
    stw 5,16(1)
    stw 6,20(1)
    stw 7,24(1)
    stw 8,28(1)
    stw 9,32(1)
    stw 10,36(1)
    mflr 0
    stw 0,40(1)
    mfctr 0
    stw 0,44(1)
    mfcr 0
    stw 0,48(1)

    lis 8,_gz_rando_tools_flags@ha
    lwz 9,_gz_rando_tools_flags@l(8)
    andi. 9,9,8192

    beq .Lgate_original_makar
    lis 8,_gz_rando_current_stage@ha
    lwz 3,_gz_rando_current_stage@l(8)
    lwz 4,_gz_rando_current_stage+4@l(8)
    lha 5,_gz_rando_last_spawn@l(8)
    li 6,1
    bl _gz_rando_companion_row
    cmpwi 3,0

    beq .Lgate_original_makar
    lwz 0,48(1)
    mtcrf 255,0
    lwz 0,44(1)
    mtctr 0
    lwz 0,40(1)
    mtlr 0
    lwz 10,36(1)
    lwz 9,32(1)
    lwz 8,28(1)
    lwz 7,24(1)
    lwz 6,20(1)
    lwz 5,16(1)
    lwz 4,12(1)
    lwz 3,8(1)
    lwz 0,4(1)
    addi 1,1,64

    b _gz_rando_makar_gate_fallthrough
.Lgate_original_makar:
    lwz 0,48(1)
    mtcrf 255,0
    lwz 0,44(1)
    mtctr 0
    lwz 0,40(1)
    mtlr 0
    lwz 10,36(1)
    lwz 9,32(1)
    lwz 8,28(1)
    lwz 7,24(1)
    lwz 6,20(1)
    lwz 5,16(1)
    lwz 4,12(1)
    lwz 3,8(1)
    lwz 0,4(1)
    addi 1,1,64

    bne .Lgate_skip_makar
    b _gz_rando_makar_gate_fallthrough
.Lgate_skip_makar:
    b _gz_rando_makar_gate_skip
.size _gz_rando_makar_gate,.-_gz_rando_makar_gate

.section .text._gz_rando_makar_restart,"ax",@progbits
.globl _gz_rando_makar_restart
.type _gz_rando_makar_restart,@function
_gz_rando_makar_restart:
    stwu 1,-64(1)
    stw 0,4(1)
    stw 3,8(1)
    stw 4,12(1)
    stw 5,16(1)
    stw 6,20(1)
    stw 7,24(1)
    stw 8,28(1)
    stw 9,32(1)
    stw 10,36(1)
    mflr 0
    stw 0,40(1)
    mfctr 0
    stw 0,44(1)
    mfcr 0
    stw 0,48(1)

    lis 8,_gz_rando_tools_flags@ha
    lwz 9,_gz_rando_tools_flags@l(8)
    andi. 9,9,8192

    beq .Lrestart_original_makar
    lis 8,_gz_rando_current_stage@ha
    lwz 3,_gz_rando_current_stage@l(8)
    lwz 4,_gz_rando_current_stage+4@l(8)
    lha 5,_gz_rando_last_spawn@l(8)
    li 6,1
    bl _gz_rando_companion_row
    cmpwi 3,0

    beq .Lrestart_original_makar
    lwz 5,16(1)
    lwz 8,4(3)
    stw 8,0(5)
    lwz 8,8(3)
    stw 8,4(5)
    lwz 8,12(3)
    stw 8,8(5)
    lha 6,2(3)
    sth 6,12(5)
    lbz 7,1(3)
    stb 7,14(5)
    stw 6,20(1)
    stw 7,24(1)
    mr 25,6
    mr 26,7
.Lrestart_original_makar:

    lwz 0,48(1)
    mtcrf 255,0
    lwz 0,44(1)
    mtctr 0
    lwz 0,40(1)
    mtlr 0
    lwz 10,36(1)
    lwz 9,32(1)
    lwz 8,28(1)
    lwz 7,24(1)
    lwz 6,20(1)
    lwz 5,16(1)
    lwz 4,12(1)
    lwz 3,8(1)
    lwz 0,4(1)
    addi 1,1,64

    b _gz_rando_original_set_restart

.size _gz_rando_makar_restart,.-_gz_rando_makar_restart

.section .text._gz_rando_medli_gate,"ax",@progbits
.globl _gz_rando_medli_gate
.type _gz_rando_medli_gate,@function
_gz_rando_medli_gate:
    stwu 1,-64(1)
    stw 0,4(1)
    stw 3,8(1)
    stw 4,12(1)
    stw 5,16(1)
    stw 6,20(1)
    stw 7,24(1)
    stw 8,28(1)
    stw 9,32(1)
    stw 10,36(1)
    mflr 0
    stw 0,40(1)
    mfctr 0
    stw 0,44(1)
    mfcr 0
    stw 0,48(1)

    lis 8,_gz_rando_tools_flags@ha
    lwz 9,_gz_rando_tools_flags@l(8)
    andi. 9,9,8192

    beq .Lgate_original_medli
    lis 8,_gz_rando_current_stage@ha
    lwz 3,_gz_rando_current_stage@l(8)
    lwz 4,_gz_rando_current_stage+4@l(8)
    lha 5,_gz_rando_last_spawn@l(8)
    li 6,2
    bl _gz_rando_companion_row
    cmpwi 3,0

    beq .Lgate_original_medli
    lwz 0,48(1)
    mtcrf 255,0
    lwz 0,44(1)
    mtctr 0
    lwz 0,40(1)
    mtlr 0
    lwz 10,36(1)
    lwz 9,32(1)
    lwz 8,28(1)
    lwz 7,24(1)
    lwz 6,20(1)
    lwz 5,16(1)
    lwz 4,12(1)
    lwz 3,8(1)
    lwz 0,4(1)
    addi 1,1,64

    b _gz_rando_medli_gate_fallthrough
.Lgate_original_medli:
    lwz 0,48(1)
    mtcrf 255,0
    lwz 0,44(1)
    mtctr 0
    lwz 0,40(1)
    mtlr 0
    lwz 10,36(1)
    lwz 9,32(1)
    lwz 8,28(1)
    lwz 7,24(1)
    lwz 6,20(1)
    lwz 5,16(1)
    lwz 4,12(1)
    lwz 3,8(1)
    lwz 0,4(1)
    addi 1,1,64

    bne .Lgate_skip_medli
    b _gz_rando_medli_gate_fallthrough
.Lgate_skip_medli:
    b _gz_rando_medli_gate_skip
.size _gz_rando_medli_gate,.-_gz_rando_medli_gate

.section .text._gz_rando_medli_restart,"ax",@progbits
.globl _gz_rando_medli_restart
.type _gz_rando_medli_restart,@function
_gz_rando_medli_restart:
    stwu 1,-64(1)
    stw 0,4(1)
    stw 3,8(1)
    stw 4,12(1)
    stw 5,16(1)
    stw 6,20(1)
    stw 7,24(1)
    stw 8,28(1)
    stw 9,32(1)
    stw 10,36(1)
    mflr 0
    stw 0,40(1)
    mfctr 0
    stw 0,44(1)
    mfcr 0
    stw 0,48(1)

    lis 8,_gz_rando_tools_flags@ha
    lwz 9,_gz_rando_tools_flags@l(8)
    andi. 9,9,8192

    beq .Lrestart_original_medli
    lis 8,_gz_rando_current_stage@ha
    lwz 3,_gz_rando_current_stage@l(8)
    lwz 4,_gz_rando_current_stage+4@l(8)
    lha 5,_gz_rando_last_spawn@l(8)
    li 6,2
    bl _gz_rando_companion_row
    cmpwi 3,0

    beq .Lrestart_original_medli
    lwz 5,16(1)
    lwz 8,4(3)
    stw 8,0(5)
    lwz 8,8(3)
    stw 8,4(5)
    lwz 8,12(3)
    stw 8,8(5)
    lha 6,2(3)
    sth 6,12(5)
    lbz 7,1(3)
    stb 7,14(5)
    stw 6,20(1)
    stw 7,24(1)
    mr 26,6
    mr 28,7
.Lrestart_original_medli:

    lwz 0,48(1)
    mtcrf 255,0
    lwz 0,44(1)
    mtctr 0
    lwz 0,40(1)
    mtlr 0
    lwz 10,36(1)
    lwz 9,32(1)
    lwz 8,28(1)
    lwz 7,24(1)
    lwz 6,20(1)
    lwz 5,16(1)
    lwz 4,12(1)
    lwz 3,8(1)
    lwz 0,4(1)
    addi 1,1,64

    b _gz_rando_original_set_restart

.size _gz_rando_medli_restart,.-_gz_rando_medli_restart

.type _gz_rando_tools_flags,@object
.type _gz_rando_original_set_restart,@function
.type _gz_rando_current_stage,@object
.type _gz_rando_last_spawn,@object
.type _gz_rando_makar_gate_fallthrough,@function
.type _gz_rando_makar_gate_skip,@function
.type _gz_rando_medli_gate_fallthrough,@function
.type _gz_rando_medli_gate_skip,@function
.section .rodata._gz_rando_makar_positions,"a",@progbits
.balign 4
.globl _gz_rando_makar_positions
.type _gz_rando_makar_positions,@object
_gz_rando_makar_positions:
    .byte 0x0f,0x0f,0x94,0xa0,0xc5,0x64,0x30,0x52,0x44,0xc2,0xb5,0x71,0x46,0x4e,0xcc,0xcd
    .byte 0x16,0x00,0x40,0x00,0xc5,0x83,0x22,0xa4,0x44,0x3c,0xa1,0x27,0x45,0xe8,0xc4,0x00
    .byte 0x17,0x02,0xb0,0x00,0x44,0x27,0x06,0xd9,0x44,0xc1,0xc0,0x00,0x45,0x0f,0xac,0x00
    .byte 0x18,0x0c,0xc0,0x00,0x46,0x5d,0xec,0x66,0xc5,0x9e,0x33,0xec,0x46,0x0b,0xd0,0x33
    .byte 0x45,0x03,0x40,0x00,0xc5,0x81,0x95,0x33,0x44,0x89,0x80,0x00,0x42,0x3f,0x85,0x1f
    .byte 0x14,0x02,0x00,0x00,0xc2,0xde,0x94,0x7b,0xc5,0x61,0x00,0x00,0xc4,0xda,0x74,0x7b
    .byte 0x46,0x0c,0x00,0x00,0x46,0x5c,0x50,0x00,0xc5,0x9e,0x33,0xec,0x46,0x11,0x50,0x00
.size _gz_rando_makar_positions,.-_gz_rando_makar_positions

.section .rodata._gz_rando_medli_positions,"a",@progbits
.balign 4
.globl _gz_rando_medli_positions
.type _gz_rando_medli_positions,@object
_gz_rando_medli_positions:
    .byte 0x00,0x00,0x80,0x00,0xc5,0xe1,0x79,0xae,0xc3,0x48,0x00,0x00,0x45,0xa4,0x56,0x52
    .byte 0x16,0x02,0xe0,0x00,0xc4,0xfb,0xa3,0x85,0x43,0x48,0x00,0x00,0xc4,0x9d,0xdf,0x0a
    .byte 0x17,0x06,0x80,0x00,0x45,0x94,0x70,0x00,0x43,0xaf,0x00,0x00,0xc5,0x0c,0xb0,0xf6
    .byte 0x18,0x0f,0xe0,0x00,0xc5,0x14,0x36,0x14,0xc4,0xfa,0x00,0x00,0x46,0x04,0x5e,0x29
    .byte 0x45,0x01,0x00,0x00,0xc5,0xfa,0x50,0x00,0x44,0x7a,0x00,0x00,0xc4,0xbc,0x9e,0x14
    .byte 0x09,0x07,0x80,0x00,0x45,0xc4,0xe8,0x7b,0x44,0xaf,0x00,0x00,0x45,0x13,0x34,0xa4
    .byte 0x11,0x0f,0x80,0x00,0xc5,0xb4,0xa0,0x00,0xc5,0x0f,0xc0,0x00,0x46,0x17,0x90,0x00
.size _gz_rando_medli_positions,.-_gz_rando_medli_positions

