/* Triforce ownership TXT page. Native numbering is zero-based 0..7.
 * dSv_player_c.mCollect +0xB4; mTriforce +0x0A => save +0xBE.
 * Input changes trainer metadata only. The one ownership store is reached
 * through the existing guarded, live/current-save inventory controller.
 */
.section .text,"ax",@progbits
.balign 4
.global _gz_shards_open
_gz_shards_open:
lis 9, _gz_page@ha
li 10, 57
stw 10, _gz_page@l(9)
li 10, 0
stw 10, 8(29)
b _gz_mask

.global _gz_shards_inventory_input
_gz_shards_inventory_input:
/* Called for selection >=27 before the existing health/Magic clamp. */
li 10, 27
stw 10, 8(29)
lwz 8, 0x10(1)
lis 9, _gz_bind_masks@ha
addi 9, 9, _gz_bind_masks@l
lwz 9, 20(9)
and. 9, 8, 9
beq _gz_shards_inventory_input_done
b _gz_shards_open
_gz_shards_inventory_input_done:
b _gz_mask

.global _gz_shards_input
_gz_shards_input:
/* A retained closed page must not consume native gameplay input. */
lwz 9, 0(29)
cmpwi 9, 0
bne _gz_shards_input_active
b _gz_input_done
_gz_shards_input_active:
bl _gz_inventory_dpad_read
lwz 8, 0x10(1)
lis 11, _gz_bind_masks@ha
addi 11, 11, _gz_bind_masks@l
lwz 9, 24(11)
and. 9, 8, 9
beq _gz_shards_navigation
lis 9, _gz_page@ha
li 10, 8
stw 10, _gz_page@l(9)
li 10, 27
stw 10, 8(29)
b _gz_mask
_gz_shards_navigation:
lwz 9, 0x1C(1)
or 8, 8, 9
lis 9, _gz_inventory_dpad_repeat@ha
lwz 9, _gz_inventory_dpad_repeat@l(9)
or 8, 8, 9
lwz 10, 8(29)
lwz 9, 12(11)
ori 9, 9, 0x0200
and. 9, 8, 9
beq _gz_shards_down
addi 10, 10, -1
_gz_shards_down:
lwz 9, 16(11)
ori 9, 9, 0x0100
and. 9, 8, 9
beq _gz_shards_lower
addi 10, 10, 1
_gz_shards_lower:
cmpwi 10, 0
bge _gz_shards_upper
li 10, 7
_gz_shards_upper:
cmpwi 10, 7
ble _gz_shards_select
li 10, 0
_gz_shards_select:
stw 10, 8(29)
/* Toggle only on a fresh A edge; directional repeat never repeats writes. */
lwz 8, 0x10(1)
lwz 9, 20(11)
and. 9, 8, 9
beq _gz_shards_input_done
addi 10, 10, 940
b _gz_inv_queue
_gz_shards_input_done:
b _gz_mask

/* BEGIN PURE SHARD PREDICATES: also assembled unchanged for host checks. */
.global _gz_shards_toggle
_gz_shards_toggle:
/* r3 = byte mask; r4 = zero-based index. Invalid indices leave it alone. */
cmplwi 4, 7
bgtlr
li 5, 1
slw 5, 5, 4
xor 3, 3, 5
blr

.global _gz_shards_count
_gz_shards_count:
/* Count exactly the eight ownership bits; never synthesize a prefix mask. */
li 4, 0
li 5, 8
_gz_shards_count_loop:
andi. 6, 3, 1
add 4, 4, 6
srwi 3, 3, 1
addi 5, 5, -1
cmpwi 5, 0
bne _gz_shards_count_loop
mr 3, 4
blr
/* END PURE SHARD PREDICATES */

.global _gz_shards_apply
_gz_shards_apply:
/* Only caller: authenticated inventory controller, after live/current root.
 * Request 940..947 toggles one bit. No chart/chest/event stores are added.
 */
addi 4, 27, -940
cmplwi 4, 7
bgtlr
stwu 1, -0x10(1)
mflr 0
stw 0, 0x14(1)
lbz 3, 0xBE(28)
bl _gz_shards_toggle
stb 3, 0xBE(28)
lwz 0, 0x14(1)
mtlr 0
addi 1, 1, 0x10
blr

.global _gz_shards_count_text
_gz_shards_count_text:
stwu 1, -0x10(1)
mflr 0
stw 0, 0x14(1)
bl _gz_shards_count
addi 3, 3, 48
lis 6, _gz_shards_count_buffer@ha
addi 6, 6, _gz_shards_count_buffer@l
stb 3, 8(6)
lwz 0, 0x14(1)
mtlr 0
addi 1, 1, 0x10
blr

.global _gz_shards_inventory_draw
_gz_shards_inventory_draw:
/* Parent draw has already validated its current save in r28. */
stwu 1, -0x10(1)
mflr 0
stw 0, 0x14(1)
lis 6, _gz_shards_entry@ha
addi 6, 6, _gz_shards_entry@l
li 4, 885
li 5, 560
bl _gz_inv_text_small
lbz 3, 0xBE(28)
bl _gz_shards_count_text
li 4, 885
li 5, 594
bl _gz_inv_text_small
lwz 0, 0x14(1)
mtlr 0
addi 1, 1, 0x10
blr

.global _gz_shards_draw
_gz_shards_draw:
stwu 1, -0x40(1)
mflr 0
stw 0, 0x44(1)
stmw 26, 0x28(1)
li 3, 2
li 4, 120
li 5, 128
lis 6, _gz_shards_title@ha
addi 6, 6, _gz_shards_title@l
bl _gz_inv_text_shadow
bl _gz_route_heap_check
cmpwi 3, 0
bne _gz_shards_draw_done
lis 8, _gz_route_heap@ha
lwz 28, _gz_route_heap@l(8)
lwz 26, 8(29)
li 27, 0
_gz_shards_draw_row:
lis 8, _gz_shards_names@ha
addi 8, 8, _gz_shards_names@l
slwi 9, 27, 2
lwzx 6, 8, 9
li 4, 150
mulli 5, 27, 43
addi 5, 5, 218
bl _gz_inv_text_small
li 8, 1
slw 8, 8, 27
lbz 9, 0xBE(28)
and. 8, 8, 9
lis 6, _gz_shards_missing@ha
addi 6, 6, _gz_shards_missing@l
beq _gz_shards_draw_status
lis 6, _gz_shards_owned@ha
addi 6, 6, _gz_shards_owned@l
_gz_shards_draw_status:
li 4, 750
mulli 5, 27, 43
addi 5, 5, 218
bl _gz_inv_text_small
cmpw 26, 27
bne _gz_shards_draw_next
lis 6, _gz_marker@ha
addi 6, 6, _gz_marker@l
li 4, 120
mulli 5, 27, 43
addi 5, 5, 218
bl _gz_inv_cursor
_gz_shards_draw_next:
addi 27, 27, 1
cmpwi 27, 8
blt _gz_shards_draw_row
lbz 3, 0xBE(28)
bl _gz_shards_count_text
li 4, 150
li 5, 580
bl _gz_inv_text_small
lis 6, _gz_shards_help@ha
addi 6, 6, _gz_shards_help@l
li 4, 120
li 5, 635
bl _gz_inv_text_small
_gz_shards_draw_done:
lmw 26, 0x28(1)
lwz 0, 0x44(1)
mtlr 0
addi 1, 1, 0x40
blr

.section .data,"aw",@progbits
.balign 4
_gz_shards_names:
.long _gz_shards_name0, _gz_shards_name1, _gz_shards_name2, _gz_shards_name3
.long _gz_shards_name4, _gz_shards_name5, _gz_shards_name6, _gz_shards_name7
_gz_shards_title:
.string "Inventory / Triforce Shards"
_gz_shards_entry:
.string "Triforce shards"
_gz_shards_name0:
.string "Shard 1"
_gz_shards_name1:
.string "Shard 2"
_gz_shards_name2:
.string "Shard 3"
_gz_shards_name3:
.string "Shard 4"
_gz_shards_name4:
.string "Shard 5"
_gz_shards_name5:
.string "Shard 6"
_gz_shards_name6:
.string "Shard 7"
_gz_shards_name7:
.string "Shard 8"
_gz_shards_owned:
.string "Owned"
_gz_shards_missing:
.string "Not owned"
_gz_shards_help:
.string "A: Toggle shard    B: Inventory"
_gz_shards_count_buffer:
.string "Shards: 0 / 8"
