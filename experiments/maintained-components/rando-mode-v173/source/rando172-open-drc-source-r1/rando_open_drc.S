.section .text._gz_rando_drc_rock_create,"ax",@progbits
.globl _gz_rando_drc_rock_create
.type _gz_rando_drc_rock_create,@function
_gz_rando_drc_rock_create:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_rock_create
    lis 11,_gz_rando_drc_stage@ha
    addi 11,11,_gz_rando_drc_stage@l
    lwz 0,0(11)
    xoris 0,0,0x4164
    xori 0,0,0x616e
    cmpwi 0,0
    bne .Loff_rock_create
    lwz 0,4(11)
    xoris 0,0,0x6d61
    xori 0,0,0x6500
    cmpwi 0,0
    bne .Loff_rock_create
    lbz 0,0xb3(30)
    cmpwi 0,8
    bne .Loff_rock_create
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_drc_rock_create_resume
.Loff_rock_create:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    bne .Lresume_rock_create
    b _gz_rando_drc_rock_create_target
.Lresume_rock_create:
    b _gz_rando_drc_rock_create_resume
.size _gz_rando_drc_rock_create,.-_gz_rando_drc_rock_create
.section .text._gz_rando_drc_water_create,"ax",@progbits
.globl _gz_rando_drc_water_create
.type _gz_rando_drc_water_create,@function
_gz_rando_drc_water_create:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_water_create
    lis 11,_gz_rando_drc_stage@ha
    addi 11,11,_gz_rando_drc_stage@l
    lwz 0,0(11)
    xoris 0,0,0x4164
    xori 0,0,0x616e
    cmpwi 0,0
    bne .Loff_water_create
    lwz 0,4(11)
    xoris 0,0,0x6d61
    xori 0,0,0x6500
    cmpwi 0,0
    bne .Loff_water_create
    lwz 0,0x51c(31)
    cmpwi 0,8
    bne .Loff_water_create
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_drc_water_create_resume
.Loff_water_create:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    beq .Lresume_water_create
    b _gz_rando_drc_water_create_target
.Lresume_water_create:
    b _gz_rando_drc_water_create_resume
.size _gz_rando_drc_water_create,.-_gz_rando_drc_water_create
.type _gz_rando_tools_flags,@object
.type _gz_rando_drc_stage,@object
.type _gz_rando_drc_rock_create_resume,@function
.type _gz_rando_drc_rock_create_target,@function
.type _gz_rando_drc_water_create_resume,@function
.type _gz_rando_drc_water_create_target,@function
