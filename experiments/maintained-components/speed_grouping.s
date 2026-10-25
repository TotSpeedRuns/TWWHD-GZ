/* Speed-only presentation. The existing float formatter remains authoritative
 * for precision, unavailable and scientific values. No gameplay stores. */
.section .text,"ax",@progbits
.balign 4
.global _gz_speed_text
_gz_speed_text:
stwu 1,-0x20(1)
mflr 0
stw 0,0x24(1)
bl _gz_float_text
mr 3,6
bl _gz_group_speed_text
mr 6,3
lwz 0,0x24(1)
mtlr 0
addi 1,1,0x20
blr

.global _gz_group_speed_text
_gz_group_speed_text:
/* r3 is an existing terminated string. Reject malformed/long input before
 * writing. Only volatile integer registers are used; sign/fraction survive. */
mr 5,3
li 7,0
lbz 8,0(5)
cmpwi 8,45
bne _gz_sg_whole
addi 5,5,1
_gz_sg_whole:
lbz 8,0(5)
cmpwi 8,0
beq _gz_sg_whole_done
cmpwi 8,46
beq _gz_sg_fraction
cmpwi 8,48
blt _gz_sg_return
cmpwi 8,57
bgt _gz_sg_return
addi 7,7,1
cmplwi 7,10
bgt _gz_sg_return
addi 5,5,1
b _gz_sg_whole
_gz_sg_fraction:
mr 4,5
li 9,0
_gz_sg_fraction_scan:
addi 4,4,1
lbz 8,0(4)
cmpwi 8,0
beq _gz_sg_whole_done
cmpwi 8,48
blt _gz_sg_return
cmpwi 8,57
bgt _gz_sg_return
addi 9,9,1
cmplwi 9,10
bgt _gz_sg_return
b _gz_sg_fraction_scan
_gz_sg_whole_done:
cmplwi 7,4
blt _gz_sg_return
lis 4,_gz_speed_grouped@ha
addi 4,4,_gz_speed_grouped@l
mr 5,3
lbz 8,0(5)
cmpwi 8,45
bne _gz_sg_copy_start
stb 8,0(4)
addi 4,4,1
addi 5,5,1
_gz_sg_copy_start:
li 9,0
li 10,3
_gz_sg_copy_digit:
cmpwi 9,0
beq _gz_sg_no_comma
divwu 11,7,10
mullw 11,11,10
subf 11,11,7
cmpwi 11,0
bne _gz_sg_no_comma
li 8,44
stb 8,0(4)
addi 4,4,1
_gz_sg_no_comma:
lbz 8,0(5)
stb 8,0(4)
addi 4,4,1
addi 5,5,1
addi 9,9,1
addi 7,7,-1
cmpwi 7,0
bne _gz_sg_copy_digit
_gz_sg_copy_tail:
lbz 8,0(5)
stb 8,0(4)
cmpwi 8,0
beq _gz_sg_done
addi 4,4,1
addi 5,5,1
b _gz_sg_copy_tail
_gz_sg_done:
lis 3,_gz_speed_grouped@ha
addi 3,3,_gz_speed_grouped@l
_gz_sg_return:
blr

.section .data,"aw",@progbits
.balign 4
.global _gz_speed_grouped
_gz_speed_grouped:
.space 32
