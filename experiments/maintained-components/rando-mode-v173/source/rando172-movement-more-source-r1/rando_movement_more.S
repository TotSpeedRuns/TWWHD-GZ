.section .text._gz_rando_crawl_first,"ax",@progbits
.globl _gz_rando_crawl_first
.type _gz_rando_crawl_first,@function
_gz_rando_crawl_first:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_crawl_first
    lis 11,_gz_rando_crawl_first_fast@ha
    lfs 0,_gz_rando_crawl_first_fast@l(11)
    b .Lrestore_crawl_first
.Loff_crawl_first:
    lis 11,_gz_rando_crawl_first_old@ha
    lfs 0,_gz_rando_crawl_first_old@l(11)
.Lrestore_crawl_first:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_crawl_first_resume
.size _gz_rando_crawl_first,.-_gz_rando_crawl_first
.section .text._gz_rando_crawl_second,"ax",@progbits
.globl _gz_rando_crawl_second
.type _gz_rando_crawl_second,@function
_gz_rando_crawl_second:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_crawl_second
    lis 11,_gz_rando_crawl_second_fast@ha
    lfs 0,_gz_rando_crawl_second_fast@l(11)
    b .Lrestore_crawl_second
.Loff_crawl_second:
    lis 11,_gz_rando_crawl_second_old@ha
    lfs 0,_gz_rando_crawl_second_old@l(11)
.Lrestore_crawl_second:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_crawl_second_resume
.size _gz_rando_crawl_second,.-_gz_rando_crawl_second
.section .text._gz_rando_grapple_motion,"ax",@progbits
.globl _gz_rando_grapple_motion
.type _gz_rando_grapple_motion,@function
_gz_rando_grapple_motion:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_grapple_motion
    lis 11,_gz_rando_grapple_motion_fast@ha
    lfs 6,_gz_rando_grapple_motion_fast@l(11)
    b .Lrestore_grapple_motion
.Loff_grapple_motion:
    lis 11,_gz_rando_grapple_motion_old@ha
    lfs 6,_gz_rando_grapple_motion_old@l(11)
.Lrestore_grapple_motion:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_grapple_motion_resume
.size _gz_rando_grapple_motion,.-_gz_rando_grapple_motion
.section .text._gz_rando_grapple_coil_scale,"ax",@progbits
.globl _gz_rando_grapple_coil_scale
.type _gz_rando_grapple_coil_scale,@function
_gz_rando_grapple_coil_scale:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_grapple_coil_scale
    lis 11,_gz_rando_scalar25@ha
    lfs 11,_gz_rando_scalar25@l(11)
    b .Lrestore_grapple_coil_scale
.Loff_grapple_coil_scale:
    lis 11,_gz_rando_grapple_coil_scale_old@ha
    lfs 11,_gz_rando_grapple_coil_scale_old@l(11)
.Lrestore_grapple_coil_scale:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_grapple_coil_scale_resume
.size _gz_rando_grapple_coil_scale,.-_gz_rando_grapple_coil_scale
.section .text._gz_rando_grapple_setup,"ax",@progbits
.globl _gz_rando_grapple_setup
.type _gz_rando_grapple_setup,@function
_gz_rando_grapple_setup:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_grapple_setup
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    addi 10,11,10
    b _gz_rando_grapple_setup_resume
.Loff_grapple_setup:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    addi 10,11,20
    b _gz_rando_grapple_setup_resume
.size _gz_rando_grapple_setup,.-_gz_rando_grapple_setup
.section .text._gz_rando_grapple_coil_step,"ax",@progbits
.globl _gz_rando_grapple_coil_step
.type _gz_rando_grapple_coil_step,@function
_gz_rando_grapple_coil_step:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_grapple_coil_step
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    addi 8,11,6
    b _gz_rando_grapple_coil_step_resume
.Loff_grapple_coil_step:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    addi 8,11,1
    b _gz_rando_grapple_coil_step_resume
.size _gz_rando_grapple_coil_step,.-_gz_rando_grapple_coil_step
.section .text._gz_rando_talk_timer,"ax",@progbits
.globl _gz_rando_talk_timer
.type _gz_rando_talk_timer,@function
_gz_rando_talk_timer:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_talk_timer
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 4,10
    b _gz_rando_talk_timer_resume
.Loff_talk_timer:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 4,20
    b _gz_rando_talk_timer_resume
.size _gz_rando_talk_timer,.-_gz_rando_talk_timer
.section .rodata.rando25,"a",@progbits
.balign 4
.globl _gz_rando_scalar25
.type _gz_rando_scalar25,@object
_gz_rando_scalar25:
    .long 0x41c80000
.size _gz_rando_scalar25,4
.type _gz_rando_tools_flags,@object
.type _gz_rando_crawl_first_old,@object
.type _gz_rando_crawl_first_fast,@object
.type _gz_rando_crawl_first_resume,@function
.type _gz_rando_crawl_second_old,@object
.type _gz_rando_crawl_second_fast,@object
.type _gz_rando_crawl_second_resume,@function
.type _gz_rando_grapple_motion_old,@object
.type _gz_rando_grapple_motion_fast,@object
.type _gz_rando_grapple_motion_resume,@function
.type _gz_rando_grapple_coil_scale_old,@object
.type _gz_rando_grapple_coil_scale_resume,@function
.type _gz_rando_grapple_setup_resume,@function
.type _gz_rando_grapple_coil_step_resume,@function
.type _gz_rando_talk_timer_resume,@function
