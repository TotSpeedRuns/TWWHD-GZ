.section .text,"ax",@progbits
.balign 4
.global _gz_mss_scan_sequence
_gz_mss_scan_sequence:
stwu 1,-0x30(1)
mflr 0
stw 0,0x34(1)
stw 8,8(1)
stw 9,12(1)
stw 12,16(1)
mr 3,8
mr 4,12
lis 5,_gz_mss_frame_counter@ha
lwz 5,_gz_mss_frame_counter@l(5)
bl _gz_mss_order_input
lwz 8,8(1)
lwz 9,12(1)
lwz 12,16(1)
lwz 0,0x34(1)
mtlr 0
addi 1,1,0x30
blr
