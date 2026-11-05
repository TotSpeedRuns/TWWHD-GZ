/* One-shot composition inside the original synchronous inventory callback.
 * The original Save member includes the named dungeon/event fields. No new
 * physical hold or thread exclusion is supplied by root/stage equality checks.
 * Mode949 publishes bit13 only after r3==0 and skips this kit if already on.
 */
.section .text.rando_grant_all_items,"ax",@progbits
.globl _gz_rando_grant_all_items
.type _gz_rando_grant_all_items,@function
_gz_rando_grant_all_items:
    stwu 1,-64(1)
    mflr 0
    stw 0,68(1)
    stw 30,60(1)
    lis 8,_gz_rando_all_game_info@ha
    lwz 8,_gz_rando_all_game_info@l(8)
    addi 8,8,32
    cmplw 8,28
    bne .Lfail
    lbz 30,1948(28)
    cmplwi 30,15
    bgt .Lfail
    bl _gz_rando_grant_kit
    cmpwi 3,0
    bne .Ldone
    bl .Lsame
    cmpwi 3,0
    bne .Ldone
    bl _gz_rando_grant_bag_counts
    cmpwi 3,0
    bne .Ldone
    bl .Lsame
    cmpwi 3,0
    bne .Ldone
    bl _gz_rando_grant_charts
    cmpwi 3,0
    bne .Ldone
    bl .Lsame
    cmpwi 3,0
    bne .Ldone
    bl _gz_rando_grant_hurricane
    cmpwi 3,0
    bne .Ldone
    bl .Lsame
    cmpwi 3,0
    bne .Ldone
    bl _gz_rando_grant_dungeon_items
    cmpwi 3,0
    bne .Ldone
    bl .Lsame
    b .Ldone
.Lfail:
    li 3,1
.Ldone:
    lwz 30,60(1)
    lwz 0,68(1)
    mtlr 0
    addi 1,1,64
    blr
.Lsame:
    lis 8,_gz_rando_all_game_info@ha
    lwz 8,_gz_rando_all_game_info@l(8)
    addi 8,8,32
    cmplw 8,28
    bne .Ldifferent
    lbz 8,1948(28)
    cmplw 8,30
    bne .Ldifferent
    li 3,0
    blr
.Ldifferent:
    li 3,1
    blr
.size _gz_rando_grant_all_items,.-_gz_rando_grant_all_items
.type _gz_rando_grant_kit,@function
.type _gz_rando_grant_bag_counts,@function
.type _gz_rando_grant_charts,@function
.type _gz_rando_grant_hurricane,@function
.type _gz_rando_grant_dungeon_items,@function
.type _gz_rando_all_game_info,@object
