/* Rando Mode: exact Siren west/north TGDR6/8 at use, no state mutation. */
.section .text._gz_rando_totg_switch,"ax",@progbits
.globl _gz_rando_totg_switch
.type _gz_rando_totg_switch,@function
_gz_rando_totg_switch:
    lbz 0,179(3)
    stwu 1,-32(1)
    stw 11,4(1)
    stw 12,8(1)
    mfcr 11
    stw 11,12(1)

    lis 11,_gz_rando_tools_flags@ha
    lwz 11,_gz_rando_tools_flags@l(11)
    andi. 11,11,8192
    beq .Lswitch_done
    lis 11,_gz_rando_current_stage@ha
    lwz 12,_gz_rando_current_stage@l(11)
    lis 11,0x5369
    ori 11,11,0x7265
    cmpw 12,11
    bne .Lswitch_done
    lis 11,_gz_rando_current_stage+4@ha
    lwz 12,_gz_rando_current_stage+4@l(11)
    lis 11,0x6e00
    cmpw 12,11
    bne .Lswitch_done
    cmpwi 3,0
    beq .Lswitch_done
    lwz 12,176(3)
    lis 11,0x0fff
    ori 11,11,0xf029
    cmpw 12,11
    beq .Lswitch_yes
    addi 11,11,1
    cmpw 12,11
    bne .Lswitch_done
    b .Lswitch_yes

.Lswitch_yes:
    li 0,255
.Lswitch_done:
    lwz 11,12(1)
    mtcrf 255,11
    lwz 12,8(1)
    lwz 11,4(1)
    addi 1,1,32

    b _gz_rando_totg_switch_resume
.size _gz_rando_totg_switch,.-_gz_rando_totg_switch
.section .text._gz_rando_totg_glow_west,"ax",@progbits
.globl _gz_rando_totg_glow_west
.type _gz_rando_totg_glow_west,@function
_gz_rando_totg_glow_west:
    stwu 1,-32(1)
    stw 11,4(1)
    stw 12,8(1)
    mfcr 11
    stw 11,12(1)

    lis 11,_gz_rando_tools_flags@ha
    lwz 11,_gz_rando_tools_flags@l(11)
    andi. 11,11,8192
    beq .Lwest_no
    lis 11,_gz_rando_current_stage@ha
    lwz 12,_gz_rando_current_stage@l(11)
    lis 11,0x5369
    ori 11,11,0x7265
    cmpw 12,11
    bne .Lwest_no
    lis 11,_gz_rando_current_stage+4@ha
    lwz 12,_gz_rando_current_stage+4@l(11)
    lis 11,0x6e00
    cmpw 12,11
    bne .Lwest_no
    cmpwi 31,0
    beq .Lwest_no
    lwz 12,176(31)
    lis 11,0x0fff
    ori 11,11,0xf029
    cmpw 12,11
    beq .Lwest_yes
    addi 11,11,1
    cmpw 12,11
    bne .Lwest_no
    b .Lwest_yes

.Lwest_yes:
    lwz 11,12(1)
    mtcrf 255,11
    lwz 12,8(1)
    lwz 11,4(1)
    addi 1,1,32

    b _gz_rando_totg_glow_west_resume
.Lwest_no:
    lwz 11,12(1)
    mtcrf 255,11
    lwz 12,8(1)
    lwz 11,4(1)
    addi 1,1,32

    beq .Lwest_skip
    b _gz_rando_totg_glow_west_resume
.Lwest_skip:
    b _gz_rando_totg_glow_west_skip
.size _gz_rando_totg_glow_west,.-_gz_rando_totg_glow_west
.section .text._gz_rando_totg_glow_north,"ax",@progbits
.globl _gz_rando_totg_glow_north
.type _gz_rando_totg_glow_north,@function
_gz_rando_totg_glow_north:
    stwu 1,-32(1)
    stw 11,4(1)
    stw 12,8(1)
    mfcr 11
    stw 11,12(1)

    lis 11,_gz_rando_tools_flags@ha
    lwz 11,_gz_rando_tools_flags@l(11)
    andi. 11,11,8192
    beq .Lnorth_no
    lis 11,_gz_rando_current_stage@ha
    lwz 12,_gz_rando_current_stage@l(11)
    lis 11,0x5369
    ori 11,11,0x7265
    cmpw 12,11
    bne .Lnorth_no
    lis 11,_gz_rando_current_stage+4@ha
    lwz 12,_gz_rando_current_stage+4@l(11)
    lis 11,0x6e00
    cmpw 12,11
    bne .Lnorth_no
    cmpwi 31,0
    beq .Lnorth_no
    lwz 12,176(31)
    lis 11,0x0fff
    ori 11,11,0xf029
    cmpw 12,11
    beq .Lnorth_yes
    addi 11,11,1
    cmpw 12,11
    bne .Lnorth_no
    b .Lnorth_yes

.Lnorth_yes:
    lwz 11,12(1)
    mtcrf 255,11
    lwz 12,8(1)
    lwz 11,4(1)
    addi 1,1,32

    b _gz_rando_totg_glow_north_resume
.Lnorth_no:
    lwz 11,12(1)
    mtcrf 255,11
    lwz 12,8(1)
    lwz 11,4(1)
    addi 1,1,32

    beq .Lnorth_skip
    b _gz_rando_totg_glow_north_resume
.Lnorth_skip:
    b _gz_rando_totg_glow_north_skip
.size _gz_rando_totg_glow_north,.-_gz_rando_totg_glow_north
.type _gz_rando_tools_flags,@object
.type _gz_rando_current_stage,@object
.type _gz_rando_totg_switch_resume,@function
.type _gz_rando_totg_glow_west_skip,@function
.type _gz_rando_totg_glow_west_resume,@function
.type _gz_rando_totg_glow_north_skip,@function
.type _gz_rando_totg_glow_north_resume,@function
