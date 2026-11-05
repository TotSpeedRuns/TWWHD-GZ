.section .text._gz_rando_drc_wave_use,"ax",@progbits
.globl _gz_rando_drc_wave_use
.type _gz_rando_drc_wave_use,@function
_gz_rando_drc_wave_use:
    .long 0xc17e0540
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)

    lwz 0,0x540(30)
    lis 11,0xbdd7
    ori 11,11,0x9436
    cmplw 0,11
    beq .Lmatch_wave_use_wave
    lis 11,0xbf06
    ori 11,11,0xbca2
    cmplw 0,11
    beq .Lmatch_wave_use_wave
    b .Lrestore_wave_use
.Lmatch_wave_use_wave:
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Loff_wave_use_wave
    lis 11,_gz_rando_drc_wave_use_wave_fast@ha
    lfs 11,_gz_rando_drc_wave_use_wave_fast@l(11)
    b .Lrestore_wave_use
.Loff_wave_use_wave:
    lis 11,_gz_rando_drc_wave_use_wave_original@ha
    lfs 11,_gz_rando_drc_wave_use_wave_original@l(11)
    b .Lrestore_wave_use
.Lrestore_wave_use:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_drc_wave_use_resume
.size _gz_rando_drc_wave_use,.-_gz_rando_drc_wave_use
.section .text._gz_rando_drc_water_use,"ax",@progbits
.globl _gz_rando_drc_water_use
.type _gz_rando_drc_water_use,@function
_gz_rando_drc_water_use:
    .long 0xc19e0534
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)

    lwz 0,0x534(30)
    lis 11,0x3dd7
    ori 11,11,0x9436
    cmplw 0,11
    beq .Lmatch_water_use_growth
    lis 11,0x3f06
    ori 11,11,0xbca2
    cmplw 0,11
    beq .Lmatch_water_use_growth
    lis 11,0x3fd6
    ori 11,11,0xc036
    cmplw 0,11
    beq .Lmatch_water_use_rise
    lis 11,0x4106
    ori 11,11,0x3822
    cmplw 0,11
    beq .Lmatch_water_use_rise
    b .Lrestore_water_use
.Lmatch_water_use_growth:
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Loff_water_use_growth
    lis 11,_gz_rando_drc_water_use_growth_fast@ha
    lfs 12,_gz_rando_drc_water_use_growth_fast@l(11)
    b .Lrestore_water_use
.Loff_water_use_growth:
    lis 11,_gz_rando_drc_water_use_growth_original@ha
    lfs 12,_gz_rando_drc_water_use_growth_original@l(11)
    b .Lrestore_water_use
.Lmatch_water_use_rise:
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Loff_water_use_rise
    lis 11,_gz_rando_drc_water_use_rise_fast@ha
    lfs 12,_gz_rando_drc_water_use_rise_fast@l(11)
    b .Lrestore_water_use
.Loff_water_use_rise:
    lis 11,_gz_rando_drc_water_use_rise_original@ha
    lfs 12,_gz_rando_drc_water_use_rise_original@l(11)
    b .Lrestore_water_use
.Lrestore_water_use:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_drc_water_use_resume
.size _gz_rando_drc_water_use,.-_gz_rando_drc_water_use
.section .text._gz_rando_drc_animation_use,"ax",@progbits
.globl _gz_rando_drc_animation_use
.type _gz_rando_drc_animation_use,@function
_gz_rando_drc_animation_use:
    .long 0xc15f0000
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)

    mflr 0
    lis 11,_gz_rando_drc_animation_use_play_return@ha
    addi 11,11,_gz_rando_drc_animation_use_play_return@l
    cmplw 0,11
    bne .Lrestore_anim
    lwz 0,92(1)
    lis 11,_gz_rando_drc_animation_use_water_bck_return@ha
    addi 11,11,_gz_rando_drc_animation_use_water_bck_return@l
    cmplw 0,11
    beq .Lbck
    lis 11,_gz_rando_drc_animation_use_water_btk_return@ha
    addi 11,11,_gz_rando_drc_animation_use_water_btk_return@l
    cmplw 0,11
    bne .Lrestore_anim
    addi 0,30,0x490
    b .Lcontroller
.Lbck:
    addi 0,30,0x41c
.Lcontroller:
    cmplw 0,31
    bne .Lrestore_anim
    lha 0,0x520(30)
    cmpwi 0,0
    bne .Lrestore_anim
    lha 0,0x522(30)
    cmpwi 0,-1
    bne .Lrestore_anim
    lwz 0,0x524(30)
    lis 11,_gz_rando_drc_animation_use_phase_0@ha
    addi 11,11,_gz_rando_drc_animation_use_phase_0@l
    cmplw 0,11
    beq .Lrate_attribute
    lis 11,_gz_rando_drc_animation_use_phase_1@ha
    addi 11,11,_gz_rando_drc_animation_use_phase_1@l
    cmplw 0,11
    beq .Lrate_attribute
    lis 11,_gz_rando_drc_animation_use_phase_2@ha
    addi 11,11,_gz_rando_drc_animation_use_phase_2@l
    cmplw 0,11
    beq .Lrate_attribute
    b .Lrestore_anim
.Lrate_attribute:
    lbz 0,14(31)
    cmpwi 0,0
    bne .Lrestore_anim
    lwz 0,0(31)
    lis 11,0x3f80
    ori 11,11,0x0000
    cmplw 0,11
    beq .Lrate
    lis 11,0x40a0
    ori 11,11,0x0000
    cmplw 0,11
    beq .Lrate
    b .Lrestore_anim
.Lrate:
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Lrate1
    lis 11,_gz_rando_drc_animation_use_rate5@ha
    lfs 10,_gz_rando_drc_animation_use_rate5@l(11)
    b .Lrestore_anim
.Lrate1:
    lis 11,_gz_rando_drc_animation_use_rate1@ha
    lfs 10,_gz_rando_drc_animation_use_rate1@l(11)
.Lrestore_anim:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_drc_animation_use_resume
.size _gz_rando_drc_animation_use,.-_gz_rando_drc_animation_use
.section .rodata.rando_drc_consumers,"a",@progbits
.balign 4
.globl _gz_rando_drc_wave_use_wave_fast
.type _gz_rando_drc_wave_use_wave_fast,@object
_gz_rando_drc_wave_use_wave_fast:
.long 0xbf06bca2
.size _gz_rando_drc_wave_use_wave_fast,4
.globl _gz_rando_drc_water_use_growth_fast
.type _gz_rando_drc_water_use_growth_fast,@object
_gz_rando_drc_water_use_growth_fast:
.long 0x3f06bca2
.size _gz_rando_drc_water_use_growth_fast,4
.globl _gz_rando_drc_water_use_rise_fast
.type _gz_rando_drc_water_use_rise_fast,@object
_gz_rando_drc_water_use_rise_fast:
.long 0x41063822
.size _gz_rando_drc_water_use_rise_fast,4
.type _gz_rando_tools_flags,@object
.type _gz_rando_drc_wave_use_resume,@function
.type _gz_rando_drc_wave_use_wave_original,@object
.type _gz_rando_drc_water_use_resume,@function
.type _gz_rando_drc_water_use_growth_original,@object
.type _gz_rando_drc_water_use_rise_original,@object
.type _gz_rando_drc_animation_use_resume,@function
.type _gz_rando_drc_animation_use_play_return,@function
.type _gz_rando_drc_animation_use_water_bck_return,@function
.type _gz_rando_drc_animation_use_water_btk_return,@function
.type _gz_rando_drc_animation_use_rate1,@object
.type _gz_rando_drc_animation_use_rate5,@object
.type _gz_rando_drc_animation_use_phase_0,@function
.type _gz_rando_drc_animation_use_phase_1,@function
.type _gz_rando_drc_animation_use_phase_2,@function
