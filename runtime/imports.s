.section .data,"aw",@progbits
.balign 4
_native_gx2_handle:
.long 0
_native_gx2_name:
.string "gx2"
.balign 4
_native_ptr_GX2Flush:
.long 0
_native_name_GX2Flush:
.string "GX2Flush"
.balign 4
_native_ptr_GX2GetLastSubmittedTimeStamp:
.long 0
_native_name_GX2GetLastSubmittedTimeStamp:
.string "GX2GetLastSubmittedTimeStamp"
.balign 4
_native_ptr_GX2GetRetiredTimeStamp:
.long 0
_native_name_GX2GetRetiredTimeStamp:
.string "GX2GetRetiredTimeStamp"
.balign 4
_native_ptr_GX2SetCullOnlyControl:
.long 0
_native_name_GX2SetCullOnlyControl:
.string "GX2SetCullOnlyControl"
.balign 4
_native_ptr_GX2SetDepthOnlyControl:
.long 0
_native_name_GX2SetDepthOnlyControl:
.string "GX2SetDepthOnlyControl"
.balign 4
_native_ptr_GX2SetShaderMode:
.long 0
_native_name_GX2SetShaderMode:
.string "GX2SetShaderMode"
.section .text,"ax",@progbits
.balign 4
.global GX2Flush
GX2Flush:
lis 12, _native_ptr_GX2Flush@ha
lwz 12, _native_ptr_GX2Flush@l(12)
cmpwi 12, 0
bne _native_ready_GX2Flush
stwu 1, -0x100(1)
mflr 0
stw 0, 0x104(1)
mfcr 0
stw 0, 8(1)
stw 3, 12(1)
stw 4, 16(1)
stw 5, 20(1)
stw 6, 24(1)
stw 7, 28(1)
stw 8, 32(1)
stw 9, 36(1)
stw 10, 40(1)
stw 11, 44(1)
stfd 1, 56(1)
stfd 2, 64(1)
stfd 3, 72(1)
stfd 4, 80(1)
stfd 5, 88(1)
stfd 6, 96(1)
stfd 7, 104(1)
stfd 8, 112(1)
stfd 9, 120(1)
stfd 10, 128(1)
stfd 11, 136(1)
stfd 12, 144(1)
stfd 13, 152(1)
lis 3, _native_gx2_name@ha
addi 3, 3, _native_gx2_name@l
lis 4, _native_gx2_handle@ha
addi 4, 4, _native_gx2_handle@l
bl OSDynLoad_Acquire
lis 3, _native_gx2_handle@ha
lwz 3, _native_gx2_handle@l(3)
li 4, 0
lis 5, _native_name_GX2Flush@ha
addi 5, 5, _native_name_GX2Flush@l
lis 6, _native_ptr_GX2Flush@ha
addi 6, 6, _native_ptr_GX2Flush@l
bl OSDynLoad_FindExport
lfd 1, 56(1)
lfd 2, 64(1)
lfd 3, 72(1)
lfd 4, 80(1)
lfd 5, 88(1)
lfd 6, 96(1)
lfd 7, 104(1)
lfd 8, 112(1)
lfd 9, 120(1)
lfd 10, 128(1)
lfd 11, 136(1)
lfd 12, 144(1)
lfd 13, 152(1)
lwz 3, 12(1)
lwz 4, 16(1)
lwz 5, 20(1)
lwz 6, 24(1)
lwz 7, 28(1)
lwz 8, 32(1)
lwz 9, 36(1)
lwz 10, 40(1)
lwz 11, 44(1)
lwz 0, 8(1)
mtcrf 255, 0
lwz 0, 0x104(1)
mtlr 0
addi 1, 1, 0x100
lis 12, _native_ptr_GX2Flush@ha
lwz 12, _native_ptr_GX2Flush@l(12)
cmpwi 12, 0
beqlr
_native_ready_GX2Flush:
mtctr 12
bctr
.global GX2GetLastSubmittedTimeStamp
GX2GetLastSubmittedTimeStamp:
lis 12, _native_ptr_GX2GetLastSubmittedTimeStamp@ha
lwz 12, _native_ptr_GX2GetLastSubmittedTimeStamp@l(12)
cmpwi 12, 0
bne _native_ready_GX2GetLastSubmittedTimeStamp
stwu 1, -0x100(1)
mflr 0
stw 0, 0x104(1)
mfcr 0
stw 0, 8(1)
stw 3, 12(1)
stw 4, 16(1)
stw 5, 20(1)
stw 6, 24(1)
stw 7, 28(1)
stw 8, 32(1)
stw 9, 36(1)
stw 10, 40(1)
stw 11, 44(1)
stfd 1, 56(1)
stfd 2, 64(1)
stfd 3, 72(1)
stfd 4, 80(1)
stfd 5, 88(1)
stfd 6, 96(1)
stfd 7, 104(1)
stfd 8, 112(1)
stfd 9, 120(1)
stfd 10, 128(1)
stfd 11, 136(1)
stfd 12, 144(1)
stfd 13, 152(1)
lis 3, _native_gx2_name@ha
addi 3, 3, _native_gx2_name@l
lis 4, _native_gx2_handle@ha
addi 4, 4, _native_gx2_handle@l
bl OSDynLoad_Acquire
lis 3, _native_gx2_handle@ha
lwz 3, _native_gx2_handle@l(3)
li 4, 0
lis 5, _native_name_GX2GetLastSubmittedTimeStamp@ha
addi 5, 5, _native_name_GX2GetLastSubmittedTimeStamp@l
lis 6, _native_ptr_GX2GetLastSubmittedTimeStamp@ha
addi 6, 6, _native_ptr_GX2GetLastSubmittedTimeStamp@l
bl OSDynLoad_FindExport
lfd 1, 56(1)
lfd 2, 64(1)
lfd 3, 72(1)
lfd 4, 80(1)
lfd 5, 88(1)
lfd 6, 96(1)
lfd 7, 104(1)
lfd 8, 112(1)
lfd 9, 120(1)
lfd 10, 128(1)
lfd 11, 136(1)
lfd 12, 144(1)
lfd 13, 152(1)
lwz 3, 12(1)
lwz 4, 16(1)
lwz 5, 20(1)
lwz 6, 24(1)
lwz 7, 28(1)
lwz 8, 32(1)
lwz 9, 36(1)
lwz 10, 40(1)
lwz 11, 44(1)
lwz 0, 8(1)
mtcrf 255, 0
lwz 0, 0x104(1)
mtlr 0
addi 1, 1, 0x100
lis 12, _native_ptr_GX2GetLastSubmittedTimeStamp@ha
lwz 12, _native_ptr_GX2GetLastSubmittedTimeStamp@l(12)
cmpwi 12, 0
beqlr
_native_ready_GX2GetLastSubmittedTimeStamp:
mtctr 12
bctr
.global GX2GetRetiredTimeStamp
GX2GetRetiredTimeStamp:
lis 12, _native_ptr_GX2GetRetiredTimeStamp@ha
lwz 12, _native_ptr_GX2GetRetiredTimeStamp@l(12)
cmpwi 12, 0
bne _native_ready_GX2GetRetiredTimeStamp
stwu 1, -0x100(1)
mflr 0
stw 0, 0x104(1)
mfcr 0
stw 0, 8(1)
stw 3, 12(1)
stw 4, 16(1)
stw 5, 20(1)
stw 6, 24(1)
stw 7, 28(1)
stw 8, 32(1)
stw 9, 36(1)
stw 10, 40(1)
stw 11, 44(1)
stfd 1, 56(1)
stfd 2, 64(1)
stfd 3, 72(1)
stfd 4, 80(1)
stfd 5, 88(1)
stfd 6, 96(1)
stfd 7, 104(1)
stfd 8, 112(1)
stfd 9, 120(1)
stfd 10, 128(1)
stfd 11, 136(1)
stfd 12, 144(1)
stfd 13, 152(1)
lis 3, _native_gx2_name@ha
addi 3, 3, _native_gx2_name@l
lis 4, _native_gx2_handle@ha
addi 4, 4, _native_gx2_handle@l
bl OSDynLoad_Acquire
lis 3, _native_gx2_handle@ha
lwz 3, _native_gx2_handle@l(3)
li 4, 0
lis 5, _native_name_GX2GetRetiredTimeStamp@ha
addi 5, 5, _native_name_GX2GetRetiredTimeStamp@l
lis 6, _native_ptr_GX2GetRetiredTimeStamp@ha
addi 6, 6, _native_ptr_GX2GetRetiredTimeStamp@l
bl OSDynLoad_FindExport
lfd 1, 56(1)
lfd 2, 64(1)
lfd 3, 72(1)
lfd 4, 80(1)
lfd 5, 88(1)
lfd 6, 96(1)
lfd 7, 104(1)
lfd 8, 112(1)
lfd 9, 120(1)
lfd 10, 128(1)
lfd 11, 136(1)
lfd 12, 144(1)
lfd 13, 152(1)
lwz 3, 12(1)
lwz 4, 16(1)
lwz 5, 20(1)
lwz 6, 24(1)
lwz 7, 28(1)
lwz 8, 32(1)
lwz 9, 36(1)
lwz 10, 40(1)
lwz 11, 44(1)
lwz 0, 8(1)
mtcrf 255, 0
lwz 0, 0x104(1)
mtlr 0
addi 1, 1, 0x100
lis 12, _native_ptr_GX2GetRetiredTimeStamp@ha
lwz 12, _native_ptr_GX2GetRetiredTimeStamp@l(12)
cmpwi 12, 0
beqlr
_native_ready_GX2GetRetiredTimeStamp:
mtctr 12
bctr
.global GX2SetCullOnlyControl
GX2SetCullOnlyControl:
lis 12, _native_ptr_GX2SetCullOnlyControl@ha
lwz 12, _native_ptr_GX2SetCullOnlyControl@l(12)
cmpwi 12, 0
bne _native_ready_GX2SetCullOnlyControl
stwu 1, -0x100(1)
mflr 0
stw 0, 0x104(1)
mfcr 0
stw 0, 8(1)
stw 3, 12(1)
stw 4, 16(1)
stw 5, 20(1)
stw 6, 24(1)
stw 7, 28(1)
stw 8, 32(1)
stw 9, 36(1)
stw 10, 40(1)
stw 11, 44(1)
stfd 1, 56(1)
stfd 2, 64(1)
stfd 3, 72(1)
stfd 4, 80(1)
stfd 5, 88(1)
stfd 6, 96(1)
stfd 7, 104(1)
stfd 8, 112(1)
stfd 9, 120(1)
stfd 10, 128(1)
stfd 11, 136(1)
stfd 12, 144(1)
stfd 13, 152(1)
lis 3, _native_gx2_name@ha
addi 3, 3, _native_gx2_name@l
lis 4, _native_gx2_handle@ha
addi 4, 4, _native_gx2_handle@l
bl OSDynLoad_Acquire
lis 3, _native_gx2_handle@ha
lwz 3, _native_gx2_handle@l(3)
li 4, 0
lis 5, _native_name_GX2SetCullOnlyControl@ha
addi 5, 5, _native_name_GX2SetCullOnlyControl@l
lis 6, _native_ptr_GX2SetCullOnlyControl@ha
addi 6, 6, _native_ptr_GX2SetCullOnlyControl@l
bl OSDynLoad_FindExport
lfd 1, 56(1)
lfd 2, 64(1)
lfd 3, 72(1)
lfd 4, 80(1)
lfd 5, 88(1)
lfd 6, 96(1)
lfd 7, 104(1)
lfd 8, 112(1)
lfd 9, 120(1)
lfd 10, 128(1)
lfd 11, 136(1)
lfd 12, 144(1)
lfd 13, 152(1)
lwz 3, 12(1)
lwz 4, 16(1)
lwz 5, 20(1)
lwz 6, 24(1)
lwz 7, 28(1)
lwz 8, 32(1)
lwz 9, 36(1)
lwz 10, 40(1)
lwz 11, 44(1)
lwz 0, 8(1)
mtcrf 255, 0
lwz 0, 0x104(1)
mtlr 0
addi 1, 1, 0x100
lis 12, _native_ptr_GX2SetCullOnlyControl@ha
lwz 12, _native_ptr_GX2SetCullOnlyControl@l(12)
cmpwi 12, 0
beqlr
_native_ready_GX2SetCullOnlyControl:
mtctr 12
bctr
.global GX2SetDepthOnlyControl
GX2SetDepthOnlyControl:
lis 12, _native_ptr_GX2SetDepthOnlyControl@ha
lwz 12, _native_ptr_GX2SetDepthOnlyControl@l(12)
cmpwi 12, 0
bne _native_ready_GX2SetDepthOnlyControl
stwu 1, -0x100(1)
mflr 0
stw 0, 0x104(1)
mfcr 0
stw 0, 8(1)
stw 3, 12(1)
stw 4, 16(1)
stw 5, 20(1)
stw 6, 24(1)
stw 7, 28(1)
stw 8, 32(1)
stw 9, 36(1)
stw 10, 40(1)
stw 11, 44(1)
stfd 1, 56(1)
stfd 2, 64(1)
stfd 3, 72(1)
stfd 4, 80(1)
stfd 5, 88(1)
stfd 6, 96(1)
stfd 7, 104(1)
stfd 8, 112(1)
stfd 9, 120(1)
stfd 10, 128(1)
stfd 11, 136(1)
stfd 12, 144(1)
stfd 13, 152(1)
lis 3, _native_gx2_name@ha
addi 3, 3, _native_gx2_name@l
lis 4, _native_gx2_handle@ha
addi 4, 4, _native_gx2_handle@l
bl OSDynLoad_Acquire
lis 3, _native_gx2_handle@ha
lwz 3, _native_gx2_handle@l(3)
li 4, 0
lis 5, _native_name_GX2SetDepthOnlyControl@ha
addi 5, 5, _native_name_GX2SetDepthOnlyControl@l
lis 6, _native_ptr_GX2SetDepthOnlyControl@ha
addi 6, 6, _native_ptr_GX2SetDepthOnlyControl@l
bl OSDynLoad_FindExport
lfd 1, 56(1)
lfd 2, 64(1)
lfd 3, 72(1)
lfd 4, 80(1)
lfd 5, 88(1)
lfd 6, 96(1)
lfd 7, 104(1)
lfd 8, 112(1)
lfd 9, 120(1)
lfd 10, 128(1)
lfd 11, 136(1)
lfd 12, 144(1)
lfd 13, 152(1)
lwz 3, 12(1)
lwz 4, 16(1)
lwz 5, 20(1)
lwz 6, 24(1)
lwz 7, 28(1)
lwz 8, 32(1)
lwz 9, 36(1)
lwz 10, 40(1)
lwz 11, 44(1)
lwz 0, 8(1)
mtcrf 255, 0
lwz 0, 0x104(1)
mtlr 0
addi 1, 1, 0x100
lis 12, _native_ptr_GX2SetDepthOnlyControl@ha
lwz 12, _native_ptr_GX2SetDepthOnlyControl@l(12)
cmpwi 12, 0
beqlr
_native_ready_GX2SetDepthOnlyControl:
mtctr 12
bctr
.global GX2SetShaderMode
GX2SetShaderMode:
lis 12, _native_ptr_GX2SetShaderMode@ha
lwz 12, _native_ptr_GX2SetShaderMode@l(12)
cmpwi 12, 0
bne _native_ready_GX2SetShaderMode
stwu 1, -0x100(1)
mflr 0
stw 0, 0x104(1)
mfcr 0
stw 0, 8(1)
stw 3, 12(1)
stw 4, 16(1)
stw 5, 20(1)
stw 6, 24(1)
stw 7, 28(1)
stw 8, 32(1)
stw 9, 36(1)
stw 10, 40(1)
stw 11, 44(1)
stfd 1, 56(1)
stfd 2, 64(1)
stfd 3, 72(1)
stfd 4, 80(1)
stfd 5, 88(1)
stfd 6, 96(1)
stfd 7, 104(1)
stfd 8, 112(1)
stfd 9, 120(1)
stfd 10, 128(1)
stfd 11, 136(1)
stfd 12, 144(1)
stfd 13, 152(1)
lis 3, _native_gx2_name@ha
addi 3, 3, _native_gx2_name@l
lis 4, _native_gx2_handle@ha
addi 4, 4, _native_gx2_handle@l
bl OSDynLoad_Acquire
lis 3, _native_gx2_handle@ha
lwz 3, _native_gx2_handle@l(3)
li 4, 0
lis 5, _native_name_GX2SetShaderMode@ha
addi 5, 5, _native_name_GX2SetShaderMode@l
lis 6, _native_ptr_GX2SetShaderMode@ha
addi 6, 6, _native_ptr_GX2SetShaderMode@l
bl OSDynLoad_FindExport
lfd 1, 56(1)
lfd 2, 64(1)
lfd 3, 72(1)
lfd 4, 80(1)
lfd 5, 88(1)
lfd 6, 96(1)
lfd 7, 104(1)
lfd 8, 112(1)
lfd 9, 120(1)
lfd 10, 128(1)
lfd 11, 136(1)
lfd 12, 144(1)
lfd 13, 152(1)
lwz 3, 12(1)
lwz 4, 16(1)
lwz 5, 20(1)
lwz 6, 24(1)
lwz 7, 28(1)
lwz 8, 32(1)
lwz 9, 36(1)
lwz 10, 40(1)
lwz 11, 44(1)
lwz 0, 8(1)
mtcrf 255, 0
lwz 0, 0x104(1)
mtlr 0
addi 1, 1, 0x100
lis 12, _native_ptr_GX2SetShaderMode@ha
lwz 12, _native_ptr_GX2SetShaderMode@l(12)
cmpwi 12, 0
beqlr
_native_ready_GX2SetShaderMode:
mtctr 12
bctr
