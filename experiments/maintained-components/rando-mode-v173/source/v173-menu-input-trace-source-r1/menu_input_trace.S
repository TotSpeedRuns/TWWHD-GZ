/* Context after the real ProcessPad guard. No guard or input mutation.
 * r31 is the original nonnull normalized controller sample; r3 is the
 * genuine Native guard return. Original CMPWI is replayed after restoration.
 * Observational last-any / last-menu triples are not an ownership grant.
 */
.section .text._gz_menu_input_trace_context,"ax",@progbits
.globl _gz_menu_input_trace_context
.type _gz_menu_input_trace_context,@function
_gz_menu_input_trace_context:
    stwu 1,-32(1)
    stw 0,8(1)
    stw 8,12(1)
    stw 9,16(1)
    stw 10,20(1)
    mfcr 0
    stw 0,24(1)
    lwz 8,0(31)
    cmpwi 8,0
    beq .Lrestore
    lis 9,_gz_menu_input_trace@ha
    addi 9,9,_gz_menu_input_trace@l
    stw 8,0(9)
    stw 3,4(9)
    lis 10,_gz_menu_input_trace_frame@ha
    lwz 10,_gz_menu_input_trace_frame@l(10)
    lwsync
    stw 10,8(9)
    lis 10,_gz_menu_input_trace_bind@ha
    lwz 10,_gz_menu_input_trace_bind@l(10)
    cmpwi 10,0
    beq .Lrestore
    and 0,8,10
    cmplw 0,10
    bne .Lrestore
    stw 8,12(9)
    stw 3,16(9)
    lis 10,_gz_menu_input_trace_frame@ha
    lwz 10,_gz_menu_input_trace_frame@l(10)
    lwsync
    stw 10,20(9)
.Lrestore:
    lwz 0,24(1)
    mtcrf 255,0
    lwz 10,20(1)
    lwz 9,16(1)
    lwz 8,12(1)
    lwz 0,8(1)
    addi 1,1,32
    cmpwi 3,0
    b _gz_menu_input_trace_resume
.size _gz_menu_input_trace_context,.-_gz_menu_input_trace_context
.section .bss._gz_menu_input_trace,"aw",@nobits
.balign 4
.globl _gz_menu_input_trace
.type _gz_menu_input_trace,@object
_gz_menu_input_trace:
    .space 24
.size _gz_menu_input_trace,24
.type _gz_menu_input_trace_resume,@function
.type _gz_menu_input_trace_frame,@object
.type _gz_menu_input_trace_bind,@object
