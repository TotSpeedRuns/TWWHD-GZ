/* Authored PPC context fragments, not ordinary C entry points. */
.section .text._gz_rando_movement_misc1,"ax",@progbits
.globl _gz_rando_movement_misc1
.type _gz_rando_movement_misc1,@function
_gz_rando_movement_misc1:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_misc1
    lis 11,_gz_rando_movement_misc1_fast@ha
    lfs 1,_gz_rando_movement_misc1_fast@l(11)
    b .Lrestore_misc1
.Loff_misc1:
    lis 11,_gz_rando_movement_misc1_old@ha
    lfs 1,_gz_rando_movement_misc1_old@l(11)
.Lrestore_misc1:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_movement_misc1_resume
.size _gz_rando_movement_misc1,.-_gz_rando_movement_misc1

.section .text._gz_rando_movement_misc2,"ax",@progbits
.globl _gz_rando_movement_misc2
.type _gz_rando_movement_misc2,@function
_gz_rando_movement_misc2:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_misc2
    lis 11,_gz_rando_movement_misc2_fast@ha
    lfs 0,_gz_rando_movement_misc2_fast@l(11)
    b .Lrestore_misc2
.Loff_misc2:
    lis 11,_gz_rando_movement_misc2_old@ha
    lfs 0,_gz_rando_movement_misc2_old@l(11)
.Lrestore_misc2:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_movement_misc2_resume
.size _gz_rando_movement_misc2,.-_gz_rando_movement_misc2

.section .text._gz_rando_movement_misc3,"ax",@progbits
.globl _gz_rando_movement_misc3
.type _gz_rando_movement_misc3,@function
_gz_rando_movement_misc3:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_misc3
    lis 11,_gz_rando_movement_misc3_fast@ha
    lfs 3,_gz_rando_movement_misc3_fast@l(11)
    b .Lrestore_misc3
.Loff_misc3:
    lis 11,_gz_rando_movement_misc3_old@ha
    lfs 3,_gz_rando_movement_misc3_old@l(11)
.Lrestore_misc3:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_movement_misc3_resume
.size _gz_rando_movement_misc3,.-_gz_rando_movement_misc3

.section .text._gz_rando_movement_misc4,"ax",@progbits
.globl _gz_rando_movement_misc4
.type _gz_rando_movement_misc4,@function
_gz_rando_movement_misc4:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_misc4
    lis 11,_gz_rando_movement_misc4_fast@ha
    lfs 1,_gz_rando_movement_misc4_fast@l(11)
    b .Lrestore_misc4
.Loff_misc4:
    lis 11,_gz_rando_movement_misc4_old@ha
    lfs 1,_gz_rando_movement_misc4_old@l(11)
.Lrestore_misc4:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_movement_misc4_resume
.size _gz_rando_movement_misc4,.-_gz_rando_movement_misc4

.section .text._gz_rando_movement_misc5,"ax",@progbits
.globl _gz_rando_movement_misc5
.type _gz_rando_movement_misc5,@function
_gz_rando_movement_misc5:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff_misc5
    lis 11,_gz_rando_movement_misc5_fast@ha
    lfs 2,_gz_rando_movement_misc5_fast@l(11)
    b .Lrestore_misc5
.Loff_misc5:
    lis 11,_gz_rando_movement_misc5_old@ha
    lfs 2,_gz_rando_movement_misc5_old@l(11)
.Lrestore_misc5:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_movement_misc5_resume
.size _gz_rando_movement_misc5,.-_gz_rando_movement_misc5

.section .text._gz_rando_block_pull_timer,"ax",@progbits
.globl _gz_rando_block_pull_timer
.type _gz_rando_block_pull_timer,@function
_gz_rando_block_pull_timer:
    .long 0xa92b000a
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Lrestore_pull_timer
    cmpwi 9,20
    bne .Lrestore_pull_timer
    li 9,12
.Lrestore_pull_timer:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_block_pull_timer_resume
.size _gz_rando_block_pull_timer,.-_gz_rando_block_pull_timer

.section .text._gz_rando_block_pull_divisor,"ax",@progbits
.globl _gz_rando_block_pull_divisor
.type _gz_rando_block_pull_divisor,@function
_gz_rando_block_pull_divisor:
    .long 0xa98c000a
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Lrestore_pull_divisor
    cmpwi 12,20
    bne .Lrestore_pull_divisor
    li 12,12
.Lrestore_pull_divisor:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_block_pull_divisor_resume
.size _gz_rando_block_pull_divisor,.-_gz_rando_block_pull_divisor

.section .text._gz_rando_block_push_timer,"ax",@progbits
.globl _gz_rando_block_push_timer
.type _gz_rando_block_push_timer,@function
_gz_rando_block_push_timer:
    .long 0xa9450004
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Lrestore_push_timer
    cmpwi 10,20
    bne .Lrestore_push_timer
    li 10,12
.Lrestore_push_timer:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_block_push_timer_resume
.size _gz_rando_block_push_timer,.-_gz_rando_block_push_timer

.section .text._gz_rando_block_push_divisor,"ax",@progbits
.globl _gz_rando_block_push_divisor
.type _gz_rando_block_push_divisor,@function
_gz_rando_block_push_divisor:
    .long 0xa8c60004
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Lrestore_push_divisor
    cmpwi 6,20
    bne .Lrestore_push_divisor
    li 6,12
.Lrestore_push_divisor:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_block_push_divisor_resume
.size _gz_rando_block_push_divisor,.-_gz_rando_block_push_divisor

.type _gz_rando_tools_flags,@object
.type _gz_rando_movement_misc1_old,@object
.type _gz_rando_movement_misc1_fast,@object
.type _gz_rando_movement_misc1_resume,@function
.type _gz_rando_movement_misc2_old,@object
.type _gz_rando_movement_misc2_fast,@object
.type _gz_rando_movement_misc2_resume,@function
.type _gz_rando_movement_misc3_old,@object
.type _gz_rando_movement_misc3_fast,@object
.type _gz_rando_movement_misc3_resume,@function
.type _gz_rando_movement_misc4_old,@object
.type _gz_rando_movement_misc4_fast,@object
.type _gz_rando_movement_misc4_resume,@function
.type _gz_rando_movement_misc5_old,@object
.type _gz_rando_movement_misc5_fast,@object
.type _gz_rando_movement_misc5_resume,@function
.type _gz_rando_block_pull_timer_resume,@function
.type _gz_rando_block_pull_divisor_resume,@function
.type _gz_rando_block_push_timer_resume,@function
.type _gz_rando_block_push_divisor_resume,@function
