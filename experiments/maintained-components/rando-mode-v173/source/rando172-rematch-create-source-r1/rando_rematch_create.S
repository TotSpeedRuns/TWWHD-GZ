.section .text._gz_rando_trials_create,"ax",@progbits
.globl _gz_rando_trials_create
.type _gz_rando_trials_create,@function
_gz_rando_trials_create:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Lnative
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16
    b _gz_rando_trials_create_finished
.Lnative:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16
    beq .Lcontinue
    b _gz_rando_trials_create_finished
.Lcontinue:
    b _gz_rando_trials_create_continue
.size _gz_rando_trials_create,.-_gz_rando_trials_create
.type _gz_rando_tools_flags,@object
.type _gz_rando_trials_create_finished,@function
.type _gz_rando_trials_create_continue,@function
