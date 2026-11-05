/* Synchronous named-field extension to the original Save member borrow.
 * Native save initializer25B98F8 establishes16 records at save+380,stride24.
 * Native25B9058 adds map/compass/bosskey bits without replacing other flags.
 * Small keys use record+20. FF2 gets map/compass; DRC3,FW4,TotG5,ET6,WT7
 * get all3 and99keys. Matching active record at save+778 is updated too.
 * No switches, rooms, chests, clear flags or outside records are reset.
 */
.section .text.rando_grant_dungeon_items,"ax",@progbits
.globl _gz_rando_grant_dungeon_items
.type _gz_rando_grant_dungeon_items,@function
_gz_rando_grant_dungeon_items:
    stwu 1,-64(1)
    mflr 0
    stw 0,68(1)
    stmw 29,52(1)
    li 29,2
.Lsaved:
    mulli 30,29,36
    addi 30,30,896
    add 30,28,30
    bl .Lrecord
    addi 29,29,1
    cmpwi 29,7
    ble .Lsaved
    lbz 29,1948(28)
    cmplwi 29,2
    blt .Lok
    cmplwi 29,7
    bgt .Lok
    addi 30,28,1912
    bl .Lrecord
.Lok:
    li 3,0
    lmw 29,52(1)
    lwz 0,68(1)
    mtlr 0
    addi 1,1,64
    blr
.Lrecord:
    stwu 1,-16(1)
    mflr 0
    stw 0,20(1)
    li 31,0
.Lbit:
    mr 3,30
    mr 4,31
    bl _gz_rando_native_on_dungeon_item
    addi 31,31,1
    cmpwi 31,2
    blt .Lbit
    cmpwi 29,2
    beq .Lrecord_done
    mr 3,30
    li 4,2
    bl _gz_rando_native_on_dungeon_item
    li 8,99
    stb 8,32(30)
.Lrecord_done:
    lwz 0,20(1)
    mtlr 0
    addi 1,1,16
    blr
.size _gz_rando_grant_dungeon_items,.-_gz_rando_grant_dungeon_items
.type _gz_rando_native_on_dungeon_item,@function
