/* Source-only logical entry; no Native patch or owner/cache activation.
 * Keep the C callee's outgoing linkage/argument area SP+0..39 available.
 * The saved fourth scalar is the original optional-probe flag, unchanged.
 */
.section .text._gz_jwa_common_after_native_frame,"ax",@progbits
.balign 4
.global _gz_jwa_common_after_native_frame
.type _gz_jwa_common_after_native_frame,@function
.global _gz_jwa_init_after_native_frame
.type _gz_jwa_init_after_native_frame,@function
.global _gz_gfo_native_after_return_adapter
.type _gz_gfo_native_after_return_adapter,@function
_gz_jwa_common_after_native_frame:
    stwu 1,-64(1)
    mflr 0
    stw 0,68(1)
    stw 3,40(1)
    stw 4,44(1)
    stw 5,48(1)
    stw 6,52(1)
    bl _gz_jwa_init_after_native_frame
    lwz 3,40(1)
    lwz 4,44(1)
    lwz 5,48(1)
    lwz 6,52(1)
    lwz 0,68(1)
    mtlr 0
    addi 1,1,64
    b _gz_gfo_native_after_return_adapter
.size _gz_jwa_common_after_native_frame,.-_gz_jwa_common_after_native_frame

.section .note.GNU-stack,"",@progbits
