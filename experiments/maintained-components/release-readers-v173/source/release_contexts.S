.section .text,"ax",@progbits
.balign 4
/* The integer EABI epilogue requested by this compiler. r11 is the caller SP.
 * Source callers save r26..31 at-24..-4 and LR at+4 before reaching it.
 */
.global _restgpr_26_x
.hidden _restgpr_26_x
.type _restgpr_26_x,@function
.global _restgpr_22_x
.hidden _restgpr_22_x
.type _restgpr_22_x,@function
_restgpr_22_x:
 lwz 22,-40(11)
 lwz 23,-36(11)
 lwz 24,-32(11)
 lwz 25,-28(11)
_restgpr_26_x:
 lwz 26,-24(11)
 lwz 27,-20(11)
 lwz 28,-16(11)
 lwz 29,-12(11)
 lwz 30,-8(11)
 lwz 31,-4(11)
 lwz 0,4(11)
 mtlr 0
 mr 1,11
 blr
.size _restgpr_26_x,.-_restgpr_26_x
.size _restgpr_22_x,.-_restgpr_22_x
/* Native continuation live-ins only. Original begin32/return24 frames retain
 * their nonvolatile saves. C ABI preserves r2/r13..31, CR2..4 and f14..31.
 * The exact integer providers execute no FP instructions; complete seeded
 * FP preservation is checked by the Native tests. No downstream Native edge
 * reads other volatile scalar state.
 * EABI linkage/parameter area0..31 is untouched by our locals32..47.
 */
.global _gz_hx_release_begin_context
.type _gz_hx_release_begin_context,@function
_gz_hx_release_begin_context:
 stwu 1,-48(1)
 mflr 0
 stw 0,44(1)
 stw 7,40(1)
 mr 3,28
 mr 4,31
 li 5,0
 bl _gz_hx_select_allocation
 mr 9,3
 lwz 7,40(1)
 lwz 0,44(1)
 mtlr 0
 addi 1,1,48
 mr. 9,9
 beq 1f
 b _gz_hx_release_begin_found
1:
 b _gz_hx_release_begin_missing
.size _gz_hx_release_begin_context,.-_gz_hx_release_begin_context

.global _gz_hx_release_return_context
.type _gz_hx_release_return_context,@function
_gz_hx_release_return_context:
 stwu 1,-48(1)
 mflr 0
 stw 0,44(1)
 stw 4,32(1)
 stw 5,36(1)
 stw 6,40(1)
 mr 3,4
 mr 4,6
 mr 5,31
 bl _gz_hx_release_select
 mr 9,3
 lwz 4,32(1)
 lwz 5,36(1)
 lwz 6,40(1)
 lwz 0,44(1)
 mtlr 0
 addi 1,1,48
 mr. 9,9
 beq 1f
 lis 3,0x4000
 b _gz_hx_release_return_found
1:
 b _gz_hx_release_return_missing
.size _gz_hx_release_return_context,.-_gz_hx_release_return_context
