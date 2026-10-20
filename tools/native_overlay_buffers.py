"""Fence a three-canvas ring without blocking gameplay on GX2DrawDone."""
def helpers():
 return '''
.section .data,"aw",@progbits
.balign 8
.global _native_canvas_base
_native_canvas_base:
.long 0
.global _native_canvas_index
_native_canvas_index:
.long 0
.global _native_canvas_fences
_native_canvas_fences:
.long 0,0,0,0,0,0
.section .text,"ax",@progbits
.balign 4
.global _native_acquire_canvas
_native_acquire_canvas:
stwu 1,-0x20(1)
mflr 0
stw 0,0x24(1)
lis 8,_native_canvas_base@ha
lwz 9,_native_canvas_base@l(8)
cmpwi 9,0
bne _native_canvas_initialized
lis 10,_gz_state@ha
addi 10,10,_gz_state@l
lwz 9,12(10)
stw 9,_native_canvas_base@l(8)
_native_canvas_initialized:
bl GX2GetRetiredTimeStamp
lis 8,_native_canvas_index@ha
lwz 9,_native_canvas_index@l(8)
slwi 9,9,3
lis 8,_native_canvas_fences@ha
addi 8,8,_native_canvas_fences@l
add 8,8,9
lwz 9,0(8)
cmplw 3,9
bgt _native_canvas_available
blt _native_canvas_busy
lwz 9,4(8)
cmplw 4,9
blt _native_canvas_busy
_native_canvas_available:
lis 8,_native_canvas_index@ha
lwz 9,_native_canvas_index@l(8)
lis 10,0x38
ori 10,10,0x4000
mullw 9,9,10
lis 8,_native_canvas_base@ha
lwz 10,_native_canvas_base@l(8)
add 9,9,10
lis 8,_gz_state@ha
addi 8,8,_gz_state@l
stw 9,12(8)
lis 3,_gz_texture@ha
addi 3,3,_gz_texture@l
stw 9,0x24(3)
bl GX2InitTextureRegs
li 3,1
b _native_canvas_acquire_return
_native_canvas_busy:
li 3,0
_native_canvas_acquire_return:
lwz 0,0x24(1)
mtlr 0
addi 1,1,0x20
blr
.global _native_submit_canvas
_native_submit_canvas:
stwu 1,-0x20(1)
mflr 0
stw 0,0x24(1)
bl GX2Flush
bl GX2GetLastSubmittedTimeStamp
lis 8,_native_canvas_index@ha
addi 8,8,_native_canvas_index@l
lwz 9,0(8)
slwi 10,9,3
lis 11,_native_canvas_fences@ha
addi 11,11,_native_canvas_fences@l
add 11,11,10
stw 3,0(11)
stw 4,4(11)
addi 9,9,1
cmpwi 9,3
blt _native_canvas_index_ready
li 9,0
_native_canvas_index_ready:
stw 9,0(8)
lwz 0,0x24(1)
mtlr 0
addi 1,1,0x20
blr
'''
