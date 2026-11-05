/* Only entered after native requested==0x3a7780c4 has been proved.
 * r0/r11 are native scratch, original caller values and displaced cmplw CR
 * already reside in the unchanged native32B frame. r3=real sampled largest.
 * Native original_min at report+32 is already written. No extra memory write.
 */
.section .text._gz_sdk_srd_reserve,"ax",@progbits
.global _gz_sdk_srd_reserve
.type _gz_sdk_srd_reserve,@function
_gz_sdk_srd_reserve:
 lis 0,256
 cmplw 3,0
 blt .Linsufficient
 b _gz_sdk_srd_subtract
.Linsufficient:
 li 31,0
 li 11,4
 b _gz_sdk_srd_report
.size _gz_sdk_srd_reserve,.-_gz_sdk_srd_reserve
.type _gz_sdk_srd_subtract,@function
.type _gz_sdk_srd_report,@function
