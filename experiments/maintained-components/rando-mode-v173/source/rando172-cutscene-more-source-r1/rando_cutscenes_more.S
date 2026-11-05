.section .text._gz_rando_gales_height,"ax",@progbits
.globl _gz_rando_gales_height
.type _gz_rando_gales_height,@function
_gz_rando_gales_height:

    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_gales_height
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_gales_height_ready
.Loff__gz_rando_gales_height:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    bgt .Lheight_ready
    b _gz_rando_gales_height_wait
.Lheight_ready:
    b _gz_rando_gales_height_ready
.size _gz_rando_gales_height,.-_gz_rando_gales_height
.section .text._gz_rando_gales_music,"ax",@progbits
.globl _gz_rando_gales_music
.type _gz_rando_gales_music,@function
_gz_rando_gales_music:

    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_gales_music
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_gales_music_resume
.Loff__gz_rando_gales_music:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    lis 11,_gz_rando_gales_music_resume@ha
    addi 11,11,_gz_rando_gales_music_resume@l
    mtlr 11
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16
    b _gz_rando_gales_music_native
.size _gz_rando_gales_music,.-_gz_rando_gales_music
.section .text._gz_rando_makar_postboss,"ax",@progbits
.globl _gz_rando_makar_postboss
.type _gz_rando_makar_postboss,@function
_gz_rando_makar_postboss:

    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_makar_postboss
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 3,1
    b _gz_rando_makar_postboss_resume
.Loff__gz_rando_makar_postboss:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    lis 11,_gz_rando_makar_postboss_resume@ha
    addi 11,11,_gz_rando_makar_postboss_resume@l
    mtlr 11
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16
    b _gz_rando_makar_postboss_native
.size _gz_rando_makar_postboss,.-_gz_rando_makar_postboss
.section .text._gz_rando_chest_animation,"ax",@progbits
.globl _gz_rando_chest_animation
.type _gz_rando_chest_animation,@function
_gz_rando_chest_animation:
    lha 5,2(24)
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Lchest_original
    cmpwi 5,8
    bne .Lchest_original

    lis 11,_gz_rando_chest_special_row1@ha
    addi 11,11,_gz_rando_chest_special_row1@l
    cmplw 24,11
    beq .Lchest_short
    addi 11,11,12
    cmplw 24,11
    beq .Lchest_short
    addi 11,11,12
    cmplw 24,11
    bne .Lchest_original
.Lchest_short:

    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 5,9
    b _gz_rando_chest_animation_resume
.Lchest_original:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_chest_animation_resume
.size _gz_rando_chest_animation,.-_gz_rando_chest_animation
.type _gz_rando_tools_flags,@object
.type _gz_rando_gales_height_ready,@function
.type _gz_rando_gales_height_wait,@function
.type _gz_rando_gales_music_resume,@function
.type _gz_rando_gales_music_native,@function
.type _gz_rando_makar_postboss_resume,@function
.type _gz_rando_makar_postboss_native,@function
.type _gz_rando_chest_special_row1,@object
.type _gz_rando_chest_animation_resume,@function
