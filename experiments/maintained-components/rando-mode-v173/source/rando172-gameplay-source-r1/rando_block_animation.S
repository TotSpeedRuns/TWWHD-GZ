/* Existing borrowed-actor callback tail. Not a C function ABI. */
.section .text.rando_block_callback,"ax",@progbits
.globl _gz_rando_block_callback
.type _gz_rando_block_callback,@function
_gz_rando_block_callback:
    sth 10,26906(30)
    stwu 1,-32(1)
    stw 0,4(1)
    stw 11,8(1)
    stw 12,12(1)
    mfcr 0
    stw 0,16(1)
    lwz 11,68(1)
    lis 12,_gz_rando_push_callback_return@ha
    addi 12,12,_gz_rando_push_callback_return@l
    cmpw 11,12
    beq .Lallowed_callback
    lis 12,_gz_rando_pull_callback_return@ha
    addi 12,12,_gz_rando_pull_callback_return@l
    cmpw 11,12
    bne .Lcallback_restore
.Lallowed_callback:
    lis 12,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(12)
    andi. 0,0,8192
    beq .Lcallback_restore
    lwz 11,180(29)
    lis 12,_gz_rando_movebox_vtable@ha
    addi 12,12,_gz_rando_movebox_vtable@l
    cmpw 11,12
    bne .Lcallback_restore
    li 3,2
.Lcallback_restore:
    lwz 0,16(1)
    mtcrf 255,0
    lwz 12,12(1)
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,32
    b _gz_rando_callback_resume
.size _gz_rando_block_callback,.-_gz_rando_block_callback

.section .text._gz_rando_push_tag,"ax",@progbits
.globl _gz_rando_push_tag
.type _gz_rando_push_tag,@function
_gz_rando_push_tag:
    mr 30,3
    cmpwi 3,0
    b _gz_rando_push_tag_resume
.size _gz_rando_push_tag,.-_gz_rando_push_tag

.section .text._gz_rando_pull_tag,"ax",@progbits
.globl _gz_rando_pull_tag
.type _gz_rando_pull_tag,@function
_gz_rando_pull_tag:
    mr 30,3
    cmpwi 3,0
    b _gz_rando_pull_tag_resume
.size _gz_rando_pull_tag,.-_gz_rando_pull_tag

.section .text._gz_rando_push_animation,"ax",@progbits
.globl _gz_rando_push_animation
.type _gz_rando_push_animation,@function
_gz_rando_push_animation:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Lold_push
    cmpwi 30,2
    bne .Lold_push
    lis 11,_gz_rando_push_animation_fast@ha
    lfs 1,_gz_rando_push_animation_fast@l(11)
    b .Lrestore_push
.Lold_push:
    lis 11,_gz_rando_push_animation_old@ha
    lfs 1,_gz_rando_push_animation_old@l(11)
.Lrestore_push:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16
    b _gz_rando_push_animation_resume
.size _gz_rando_push_animation,.-_gz_rando_push_animation

.section .text._gz_rando_pull_animation,"ax",@progbits
.globl _gz_rando_pull_animation
.type _gz_rando_pull_animation,@function
_gz_rando_pull_animation:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192
    beq .Lold_pull
    cmpwi 30,2
    bne .Lold_pull
    lis 11,_gz_rando_pull_animation_fast@ha
    lfs 1,_gz_rando_pull_animation_fast@l(11)
    b .Lrestore_pull
.Lold_pull:
    lis 11,_gz_rando_pull_animation_old@ha
    lfs 1,_gz_rando_pull_animation_old@l(11)
.Lrestore_pull:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16
    b _gz_rando_pull_animation_resume
.size _gz_rando_pull_animation,.-_gz_rando_pull_animation

.type _gz_rando_tools_flags,@object
.type _gz_rando_movebox_vtable,@object
.type _gz_rando_push_callback_return,@function
.type _gz_rando_pull_callback_return,@function
.type _gz_rando_callback_resume,@function
.type _gz_rando_push_tag_resume,@function
.type _gz_rando_pull_tag_resume,@function
.type _gz_rando_push_animation_old,@object
.type _gz_rando_push_animation_fast,@object
.type _gz_rando_push_animation_resume,@function
.type _gz_rando_pull_animation_old,@object
.type _gz_rando_pull_animation_fast,@object
.type _gz_rando_pull_animation_resume,@function
