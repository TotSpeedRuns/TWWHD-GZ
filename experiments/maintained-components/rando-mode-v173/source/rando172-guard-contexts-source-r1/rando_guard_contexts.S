/* Caller contexts only; the boolean Rando config is authenticated separately. */
.section .text._gz_rando_guard_load_0,"ax",@progbits
.global _gz_rando_guard_load_0
.type _gz_rando_guard_load_0,@function
.type _gz_rando_guard_load_0_resume,@function
_gz_rando_guard_load_0:
 lwz 3,_gz_rando_tools_flags@l(9)
 rlwinm 3,3,0,19,17
 b _gz_rando_guard_load_0_resume
.size _gz_rando_guard_load_0,.-_gz_rando_guard_load_0
.section .text._gz_rando_guard_load_1,"ax",@progbits
.global _gz_rando_guard_load_1
.type _gz_rando_guard_load_1,@function
.type _gz_rando_guard_load_1_resume,@function
_gz_rando_guard_load_1:
 lwz 3,_gz_rando_tools_flags@l(9)
 rlwinm 3,3,0,19,17
 b _gz_rando_guard_load_1_resume
.size _gz_rando_guard_load_1,.-_gz_rando_guard_load_1
.section .text._gz_rando_guard_load_2,"ax",@progbits
.global _gz_rando_guard_load_2
.type _gz_rando_guard_load_2,@function
.type _gz_rando_guard_load_2_resume,@function
_gz_rando_guard_load_2:
 lwz 9,_gz_rando_tools_flags@l(9)
 rlwinm 9,9,0,19,17
 b _gz_rando_guard_load_2_resume
.size _gz_rando_guard_load_2,.-_gz_rando_guard_load_2
.section .text._gz_rando_guard_load_3,"ax",@progbits
.global _gz_rando_guard_load_3
.type _gz_rando_guard_load_3,@function
.type _gz_rando_guard_load_3_resume,@function
_gz_rando_guard_load_3:
 lwz 9,_gz_rando_tools_flags@l(9)
 rlwinm 9,9,0,19,17
 b _gz_rando_guard_load_3_resume
.size _gz_rando_guard_load_3,.-_gz_rando_guard_load_3
.section .text._gz_rando_guard_load_4,"ax",@progbits
.global _gz_rando_guard_load_4
.type _gz_rando_guard_load_4,@function
.type _gz_rando_guard_load_4_resume,@function
_gz_rando_guard_load_4:
 lwz 9,_gz_rando_tools_flags@l(9)
 rlwinm 9,9,0,19,17
 b _gz_rando_guard_load_4_resume
.size _gz_rando_guard_load_4,.-_gz_rando_guard_load_4
.section .text._gz_rando_guard_load_5,"ax",@progbits
.global _gz_rando_guard_load_5
.type _gz_rando_guard_load_5,@function
.type _gz_rando_guard_load_5_resume,@function
_gz_rando_guard_load_5:
 lwz 9,_gz_rando_tools_flags@l(9)
 rlwinm 9,9,0,19,17
 b _gz_rando_guard_load_5_resume
.size _gz_rando_guard_load_5,.-_gz_rando_guard_load_5
.section .text._gz_rando_guard_load_6,"ax",@progbits
.global _gz_rando_guard_load_6
.type _gz_rando_guard_load_6,@function
.type _gz_rando_guard_load_6_resume,@function
_gz_rando_guard_load_6:
 lwz 10,_gz_rando_tools_flags@l(10)
 rlwinm 10,10,0,19,17
 b _gz_rando_guard_load_6_resume
.size _gz_rando_guard_load_6,.-_gz_rando_guard_load_6
.section .text._gz_rando_guard_load_7,"ax",@progbits
.global _gz_rando_guard_load_7
.type _gz_rando_guard_load_7,@function
.type _gz_rando_guard_load_7_resume,@function
_gz_rando_guard_load_7:
 lwz 9,_gz_rando_tools_flags@l(9)
 rlwinm 9,9,0,19,17
 b _gz_rando_guard_load_7_resume
.size _gz_rando_guard_load_7,.-_gz_rando_guard_load_7
.section .text._gz_rando_guard_load_8,"ax",@progbits
.global _gz_rando_guard_load_8
.type _gz_rando_guard_load_8,@function
.type _gz_rando_guard_load_8_resume,@function
_gz_rando_guard_load_8:
 lwz 9,_gz_rando_tools_flags@l(9)
 rlwinm 9,9,0,19,17
 b _gz_rando_guard_load_8_resume
.size _gz_rando_guard_load_8,.-_gz_rando_guard_load_8
.section .text._gz_rando_guard_load_9,"ax",@progbits
.global _gz_rando_guard_load_9
.type _gz_rando_guard_load_9,@function
.type _gz_rando_guard_load_9_resume,@function
_gz_rando_guard_load_9:
 lwz 9,_gz_rando_tools_flags@l(9)
 rlwinm 9,9,0,19,17
 b _gz_rando_guard_load_9_resume
.size _gz_rando_guard_load_9,.-_gz_rando_guard_load_9
.section .text._gz_rando_guard_load_10,"ax",@progbits
.global _gz_rando_guard_load_10
.type _gz_rando_guard_load_10,@function
.type _gz_rando_guard_load_10_resume,@function
_gz_rando_guard_load_10:
 lwz 9,_gz_rando_tools_flags@l(9)
 rlwinm 9,9,0,19,17
 b _gz_rando_guard_load_10_resume
.size _gz_rando_guard_load_10,.-_gz_rando_guard_load_10
.type _gz_rando_tools_flags,@object
