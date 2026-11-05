.section .text._gz_rando_drc_wave_growth,"ax",@progbits
.globl _gz_rando_drc_wave_growth
.type _gz_rando_drc_wave_growth,@function
_gz_rando_drc_wave_growth:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_wave_growth
    lis 11,_gz_rando_drc_wave_growth_fast@ha
    lfs 13,_gz_rando_drc_wave_growth_fast@l(11)
    b .Lrestore_wave_growth
.Loff_wave_growth:
    lis 11,_gz_rando_drc_wave_growth_original@ha
    lfs 13,_gz_rando_drc_wave_growth_original@l(11)
.Lrestore_wave_growth:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_drc_wave_growth_resume
.size _gz_rando_drc_wave_growth,.-_gz_rando_drc_wave_growth
.section .text._gz_rando_drc_water_growth,"ax",@progbits
.globl _gz_rando_drc_water_growth
.type _gz_rando_drc_water_growth,@function
_gz_rando_drc_water_growth:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_water_growth
    lis 11,_gz_rando_drc_water_growth_fast@ha
    lfs 0,_gz_rando_drc_water_growth_fast@l(11)
    b .Lrestore_water_growth
.Loff_water_growth:
    lis 11,_gz_rando_drc_water_growth_original@ha
    lfs 0,_gz_rando_drc_water_growth_original@l(11)
.Lrestore_water_growth:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_drc_water_growth_resume
.size _gz_rando_drc_water_growth,.-_gz_rando_drc_water_growth
.section .text._gz_rando_drc_water_rise,"ax",@progbits
.globl _gz_rando_drc_water_rise
.type _gz_rando_drc_water_rise,@function
_gz_rando_drc_water_rise:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_water_rise
    lis 11,_gz_rando_drc_water_rise_fast@ha
    lfs 0,_gz_rando_drc_water_rise_fast@l(11)
    b .Lrestore_water_rise
.Loff_water_rise:
    lis 11,_gz_rando_drc_water_rise_original@ha
    lfs 0,_gz_rando_drc_water_rise_original@l(11)
.Lrestore_water_rise:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_drc_water_rise_resume
.size _gz_rando_drc_water_rise,.-_gz_rando_drc_water_rise
.section .text._gz_rando_drc_btk_speed,"ax",@progbits
.globl _gz_rando_drc_btk_speed
.type _gz_rando_drc_btk_speed,@function
_gz_rando_drc_btk_speed:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_btk_speed
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    stfs 13,0x490(31)
    b _gz_rando_drc_btk_speed_resume
.Loff_btk_speed:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    stfs 0,0x490(31)
    b _gz_rando_drc_btk_speed_resume
.size _gz_rando_drc_btk_speed,.-_gz_rando_drc_btk_speed
.section .text._gz_rando_drc_bck_speed,"ax",@progbits
.globl _gz_rando_drc_bck_speed
.type _gz_rando_drc_bck_speed,@function
_gz_rando_drc_bck_speed:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_bck_speed
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    stfs 13,0x41c(31)
    b _gz_rando_drc_bck_speed_resume
.Loff_bck_speed:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    stfs 0,0x41c(31)
    b _gz_rando_drc_bck_speed_resume
.size _gz_rando_drc_bck_speed,.-_gz_rando_drc_bck_speed
.section .text._gz_rando_drc_geyser_sound,"ax",@progbits
.globl _gz_rando_drc_geyser_sound
.type _gz_rando_drc_geyser_sound,@function
_gz_rando_drc_geyser_sound:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_geyser_sound
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 12,62
    b _gz_rando_drc_geyser_sound_resume
.Loff_geyser_sound:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 12,310
    b _gz_rando_drc_geyser_sound_resume
.size _gz_rando_drc_geyser_sound,.-_gz_rando_drc_geyser_sound
.section .rodata.rando_drc_water,"a",@progbits
.balign 4
.globl _gz_rando_drc_wave_growth_fast
.type _gz_rando_drc_wave_growth_fast,@object
_gz_rando_drc_wave_growth_fast:
.long 0xbf06bca2
.size _gz_rando_drc_wave_growth_fast,4
.globl _gz_rando_drc_water_growth_fast
.type _gz_rando_drc_water_growth_fast,@object
_gz_rando_drc_water_growth_fast:
.long 0x3f06bca2
.size _gz_rando_drc_water_growth_fast,4
.globl _gz_rando_drc_water_rise_fast
.type _gz_rando_drc_water_rise_fast,@object
_gz_rando_drc_water_rise_fast:
.long 0x41063822
.size _gz_rando_drc_water_rise_fast,4
.type _gz_rando_tools_flags,@object
.type _gz_rando_drc_wave_growth_original,@object
.type _gz_rando_drc_wave_growth_resume,@function
.type _gz_rando_drc_water_growth_original,@object
.type _gz_rando_drc_water_growth_resume,@function
.type _gz_rando_drc_water_rise_original,@object
.type _gz_rando_drc_water_rise_resume,@function
.type _gz_rando_drc_btk_speed_resume,@function
.type _gz_rando_drc_bck_speed_resume,@function
.type _gz_rando_drc_geyser_sound_resume,@function
