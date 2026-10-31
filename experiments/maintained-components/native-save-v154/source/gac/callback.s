.section .text,"ax",@progbits
.balign 4
.global _gz_gac_callback
.type _gz_gac_callback,@function
/* The existing NBD outer bridge preserves complete native context. Only the
 * exact original saved return words from that actual frame reach this seam.
 * No supplied stack pointer or source/native address becomes read authority.
 */
_gz_gac_callback:
 stwu 1,-48(1)
 mflr 0
 stw 0,52(1)
 lis 9,_gz_gac_native_frame_return@ha
 addi 9,9,_gz_gac_native_frame_return@l
 cmplw 3,9
 bne .Lksr_done
 lis 9,(_gz_frame_done+8)@ha
 addi 9,9,(_gz_frame_done+8)@l
 cmplw 4,9
 bne .Lksr_done
 lis 9,_gz_gac_callback_latch@ha
 addi 9,9,_gz_gac_callback_latch@l
.Lksr_claim:
 lwarx 10,0,9
 cmpwi 10,0
 bne .Lksr_done
 li 10,1
 stwcx. 10,0,9
 bne .Lksr_claim
 isync
 lis 10,_gz_gac_callback_original_return@ha
 stw 3,_gz_gac_callback_original_return@l(10)
 lis 10,_gz_gac_callback_service_return@ha
 stw 4,_gz_gac_callback_service_return@l(10)
 sync
 bl _gz_gac_tick
 bl _gz_ghs_tick
 li 10,0
 lis 9,_gz_gac_callback_original_return@ha
 stw 10,_gz_gac_callback_original_return@l(9)
 lis 9,_gz_gac_callback_service_return@ha
 stw 10,_gz_gac_callback_service_return@l(9)
 sync
 lis 9,_gz_gac_callback_latch@ha
 stw 10,_gz_gac_callback_latch@l(9)
.Lksr_done:
 lwz 0,52(1)
 mtlr 0
 addi 1,1,48
 blr
.size _gz_gac_callback,.-_gz_gac_callback
