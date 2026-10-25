/* Text-only Debug/MSS/IL overlays reuse the native font batch. Menu, input
 * graphics, other trainers and any unavailable font keep the normal path. */
.section .text,"ax",@progbits
.balign 4
.global _gz_lite_hud_try
_gz_lite_hud_try:
stwu 1,-0x30(1)
mflr 0
stw 0,0x34(1)
stw 31,0x2c(1)
lwz 8,0(29)
cmpwi 8,0
bne _gz_lite_hud_fallback
lis 8,_gz_tools_flags@ha
lwz 8,_gz_tools_flags@l(8)
andi. 8,8,4599
andi. 9,8,17
cmpwi 9,0
beq _gz_lite_hud_fallback
xor 8,8,9
cmpwi 8,0
bne _gz_lite_hud_fallback
lis 8,_gz_toast_active@ha
lwz 8,_gz_toast_active@l(8)
cmpwi 8,0
bne _gz_lite_hud_fallback
lis 8,_gz_lite_hud_report@ha
addi 8,8,_gz_lite_hud_report@l
lwz 9,8(8)
addi 9,9,1
stw 9,8(8)
li 9,1
lis 8,_gz_lite_hud_only@ha
stw 9,_gz_lite_hud_only@l(8)
li 9,0
lis 8,_gz_lite_hud_failed@ha
stw 9,_gz_lite_hud_failed@l(8)
lis 8,_gz_native_action_queue_count@ha
stw 9,_gz_native_action_queue_count@l(8)
bl _gz_draw_stats
bl _gz_mss_draw
bl _gz_il_draw
li 9,0
lis 8,_gz_lite_hud_only@ha
stw 9,_gz_lite_hud_only@l(8)
lis 8,_gz_lite_hud_failed@ha
lwz 9,_gz_lite_hud_failed@l(8)
cmpwi 9,0
bne _gz_lite_hud_rejected
lis 8,_gz_native_action_queue_count@ha
lwz 9,_gz_native_action_queue_count@l(8)
cmpwi 9,0
beq _gz_lite_hud_rejected
lis 8,_gz_lite_hud_report@ha
addi 8,8,_gz_lite_hud_report@l
stw 9,20(8)
lwz 9,20(29)
lis 8,_gz_native_action_render_context@ha
stw 9,_gz_native_action_render_context@l(8)
mr 31,30
bl _gz_native_action_tv_batch
lwz 3,20(29)
bl GX2SetContextState
lis 8,_gz_lite_hud_report@ha
addi 8,8,_gz_lite_hud_report@l
lwz 9,12(8)
addi 9,9,1
stw 9,12(8)
li 3,1
lwz 31,0x2c(1)
lwz 0,0x34(1)
mtlr 0
addi 1,1,0x30
blr
_gz_lite_hud_rejected:
lis 8,_gz_lite_hud_report@ha
addi 8,8,_gz_lite_hud_report@l
lwz 9,16(8)
addi 9,9,1
stw 9,16(8)
li 9,0
lis 8,_gz_native_action_queue_count@ha
stw 9,_gz_native_action_queue_count@l(8)
_gz_lite_hud_fallback:
lwz 31,0x2c(1)
lwz 0,0x34(1)
mtlr 0
addi 1,1,0x30
b _gz_il_hud_try

.global _gz_lite_hud_font_fallback
_gz_lite_hud_font_fallback:
lis 8,_gz_lite_hud_only@ha
lwz 8,_gz_lite_hud_only@l(8)
cmpwi 8,0
bne _gz_lite_hud_font_abort
b _gz_font_draw_fill_px
_gz_lite_hud_font_abort:
li 9,1
lis 8,_gz_lite_hud_failed@ha
stw 9,_gz_lite_hud_failed@l(8)
blr
.section .data,"aw",@progbits
.balign 4
.global _gz_lite_hud_only
_gz_lite_hud_only:
.long 0
.global _gz_lite_hud_failed
_gz_lite_hud_failed:
.long 0
.global _gz_lite_hud_report
_gz_lite_hud_report:
.long 0x4c485531,32,0,0,0,0,0,0
