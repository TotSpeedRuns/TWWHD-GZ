; Inventory-owned page51. Input only queues requests; the existing validated
; inventory tick applies them. mCollect.mTact is save+BD, bits0..5 (d_item).
_gz_song_open:
    lis r9, _gz_page@ha
    li r10, 51
    stw r10, _gz_page@l(r9)
    li r10, 0
    stw r10, 8(r29)
    b _gz_mask
_gz_song_input:
    bl _gz_inventory_dpad_read
    lwz r9, 0(r29)
    cmpwi r9, 0
    bne _gz_song_input_active
    b _gz_input_done
_gz_song_input_active:
    lwz r8, 0x10(r1)
    lis r11, _gz_bind_masks@ha
    addi r11, r11, _gz_bind_masks@l
    lwz r9, 24(r11)
    and. r9, r8, r9
    beq _gz_song_navigation
    lis r9, _gz_page@ha
    li r10, 8
    stw r10, _gz_page@l(r9)
    li r10, 4
    stw r10, 8(r29)
    b _gz_mask
_gz_song_navigation:
    lwz r9, 0x1C(r1)
    or r8, r8, r9
    lis r9, _gz_inventory_dpad_repeat@ha
    lwz r9, _gz_inventory_dpad_repeat@l(r9)
    or r8, r8, r9
    lwz r10, 8(r29)
    lwz r9, 12(r11)
    ori r9, r9, 0x0200
    and. r9, r8, r9
    beq _gz_song_down
    addi r10, r10, -1
_gz_song_down:
    lwz r9, 16(r11)
    ori r9, r9, 0x0100
    and. r9, r8, r9
    beq _gz_song_lower
    addi r10, r10, 1
_gz_song_lower:
    cmpwi r10, 0
    bge _gz_song_upper
    li r10, 5
_gz_song_upper:
    cmpwi r10, 5
    ble _gz_song_select
    li r10, 0
_gz_song_select:
    stw r10, 8(r29)
    lwz r8, 0x10(r1)
    lwz r9, 20(r11)
    and. r9, r8, r9
    beq _gz_song_remove
    addi r10, r10, 920
    b _gz_inv_queue
_gz_song_remove:
    andi. r9, r8, 0x2000
    beq _gz_song_input_done
    addi r10, r10, 930
    b _gz_inv_queue
_gz_song_input_done:
    b _gz_mask

; r27 request920..925 Give,930..935 Remove; r28 validated save base.
; Preserve every other song bit; granting also provides a usable Wind Waker.
_gz_song_apply:
    addi r9, r27, -920
    cmplwi r9, 5
    ble _gz_song_give
    addi r9, r27, -930
    cmplwi r9, 5
    bgtlr
    li r8, 1
    slw r8, r8, r9
    lbz r10, 0xBD(r28)
    andc r10, r10, r8
    stb r10, 0xBD(r28)
    blr
_gz_song_give:
    li r8, 1
    slw r8, r8, r9
    lbz r10, 0xBD(r28)
    or r10, r10, r8
    stb r10, 0xBD(r28)
    li r8, 0x22
    stb r8, 0x3E(r28)
    lbz r8, 0x53(r28)
    ori r8, r8, 1
    stb r8, 0x53(r28)
    blr

_gz_song_draw:
    stwu r1, -0x40(r1)
    mflr r0
    stw r0, 0x44(r1)
    stmw r26, 0x28(r1)
    li r3, 2
    li r4, 120
    li r5, 128
    lis r6, _gz_song_title@ha
    addi r6, r6, _gz_song_title@l
    bl _gz_inv_text_shadow
    bl _gz_route_heap_check
    cmpwi r3, 0
    bne _gz_song_draw_done
    lis r8, _gz_route_heap@ha
    lwz r28, _gz_route_heap@l(r8)
    lwz r26, 8(r29)
    li r27, 0
_gz_song_draw_row:
    lis r8, _gz_song_names@ha
    addi r8, r8, _gz_song_names@l
    slwi r9, r27, 2
    lwzx r6, r8, r9
    li r4, 150
    mulli r5, r27, 58
    addi r5, r5, 225
    bl _gz_inv_text_small
    li r8, 1
    slw r8, r8, r27
    lbz r9, 0xBD(r28)
    and. r8, r8, r9
    lis r6, _gz_song_missing@ha
    addi r6, r6, _gz_song_missing@l
    beq _gz_song_draw_status
    lis r6, _gz_song_learned@ha
    addi r6, r6, _gz_song_learned@l
_gz_song_draw_status:
    li r4, 750
    mulli r5, r27, 58
    addi r5, r5, 225
    bl _gz_inv_text_small
    cmpw r26, r27
    bne _gz_song_draw_next
    lis r6, _gz_marker@ha
    addi r6, r6, _gz_marker@l
    li r4, 120
    mulli r5, r27, 58
    addi r5, r5, 225
    bl _gz_inv_cursor
_gz_song_draw_next:
    addi r27, r27, 1
    cmpwi r27, 6
    blt _gz_song_draw_row
    lis r6, _gz_song_help@ha
    addi r6, r6, _gz_song_help@l
    li r4, 120
    li r5, 635
    bl _gz_inv_text_small
_gz_song_draw_done:
    lmw r26, 0x28(r1)
    lwz r0, 0x44(r1)
    mtlr r0
    addi r1, r1, 0x40
    blr
.align 4
_gz_song_names:
.int _gz_song_name0,_gz_song_name1,_gz_song_name2,_gz_song_name3,_gz_song_name4,_gz_song_name5
_gz_song_title:
.string "Inventory / Wind Waker Songs"
_gz_song_name0:
.string "Wind's Requiem"
_gz_song_name1:
.string "Ballad of Gales"
_gz_song_name2:
.string "Command Melody"
_gz_song_name3:
.string "Earth God's Lyric"
_gz_song_name4:
.string "Wind God's Aria"
_gz_song_name5:
.string "Song of Passing"
_gz_song_learned:
.string "Learned"
_gz_song_missing:
.string "Not learned"
_gz_song_help:
.string "A: Give Song    X: Remove Song    B: Inventory"
.align 4
