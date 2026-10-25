/* Fixed native pause predicate entry. No native state is changed. */
.section .text,"ax",@progbits
.balign 4
.global _gz_fad_native_pause
_gz_fad_native_pause:
lis 12,_gz_fad_forced@ha
lwz 12,_gz_fad_forced@l(12)
cmpwi 12,0
beq _gz_fad_native_original
li 3,1
blr
.global _gz_fad_native_original
_gz_fad_native_original:
/* Exact displaced first instruction supplied by the pinned preparation. */
.long FAD_NATIVE_FIRST
b _gz_fad_native_resume

.global _gz_fad_input_gate
_gz_fad_input_gate:
stwu 1,-0x60(1)
mflr 0
stw 0,0x64(1)
mfcr 0
stw 0,0x30(1)
stw 3,8(1)
stw 4,12(1)
stw 5,16(1)
stw 6,20(1)
stw 7,24(1)
stw 8,28(1)
stw 9,32(1)
stw 10,36(1)
stw 11,40(1)
stw 12,44(1)
mr 3,31
lwz 4,0x70(1)
lwz 5,0x7c(1)
bl _gz_fad_input_transport
stw 3,0x38(1)
lwz 3,8(1)
lwz 4,12(1)
lwz 5,16(1)
lwz 6,20(1)
lwz 7,24(1)
lwz 8,28(1)
lwz 9,32(1)
lwz 10,36(1)
lwz 11,40(1)
lwz 12,44(1)
lwz 0,0x30(1)
mtcrf 255,0
lwz 0,0x64(1)
mtlr 0
lwz 0,0x38(1)
addi 1,1,0x60
cmpwi 0,0
beq _gz_fad_input_not_owned
cmpwi 0,2
beq _gz_fad_input_dpad
cmpwi 0,3
beq _gz_fad_input_replayed
b _gz_fad_input_consumed
_gz_fad_input_not_owned:
b _gz_fav_input_gate
_gz_fad_input_consumed:
b _gz_mask
_gz_fad_input_dpad:
li 9,0x300
lwz 8,0(31)
andc 8,8,9
stw 8,0(31)
lwz 8,4(31)
andc 8,8,9
stw 8,4(31)
lwz 8,8(31)
andc 8,8,9
stw 8,8(31)
li 30,2
b _gz_input_done
_gz_fad_input_replayed:
li 9,0x300
lwz 8,0(31)
andc 8,8,9
stw 8,0(31)
lwz 8,4(31)
andc 8,8,9
stw 8,4(31)
lwz 8,8(31)
andc 8,8,9
stw 8,8(31)
li 30,3
b _gz_input_done
.global _gz_fad_enter_menu
_gz_fad_enter_menu:
bl _gz_fad_open
b _gz_mask
