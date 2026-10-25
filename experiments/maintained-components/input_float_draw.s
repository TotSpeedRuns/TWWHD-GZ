.section .text,"ax",@progbits
.balign 4
.global _gz_ivf_draw
_gz_ivf_draw:
stwu 1,-0x40(1)
mflr 0
stw 0,0x44(1)
stw 31,0x3c(1)
/* Copy the original four f32 components before formatting any text. */
lis 8,_gz_input_snapshot@ha
addi 8,8,_gz_input_snapshot@l
lwz 9,4(8)
stw 9,8(1)
lwz 9,8(8)
stw 9,12(1)
lwz 9,12(8)
stw 9,16(1)
lwz 9,16(8)
stw 9,20(1)
li 31,0
_gz_ivf_draw_next:
slwi 8,31,2
addi 9,1,8
lwzx 4,9,8
mr 3,31
bl _gz_ivf_text
mr 6,3
li 4,740
mulli 5,31,26
addi 5,5,520
bl _gz_pad_font_small
addi 31,31,1
cmpwi 31,4
blt _gz_ivf_draw_next
lwz 31,0x3c(1)
lwz 0,0x44(1)
mtlr 0
addi 1,1,0x40
blr
