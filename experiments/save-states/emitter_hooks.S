/* Offline candidate overlays. Direct non-link native branches preserve the
 * interrupted LR. C is soft-float; no FP/paired-single operation is emitted.
 * All GPRs and CR/XER/CTR/LR are restored before the displaced instruction. */
.section .text
.macro SAVE
 stwu 1,-208(1)
 stw 0,40(1)
 mflr 0
 stw 0,44(1)
 mfcr 0
 stw 0,48(1)
 mfctr 0
 stw 0,52(1)
 mfxer 0
 stw 0,56(1)
 stmw 2,64(1)
.endm
.macro RESTORE
 lmw 2,64(1)
 lwz 0,56(1)
 mtxer 0
 lwz 0,52(1)
 mtctr 0
 lwz 0,48(1)
 mtcr 0
 lwz 0,44(1)
 mtlr 0
 lwz 0,40(1)
 addi 1,1,208
.endm
.global _gz_jel_manager_hook
_gz_jel_manager_hook:
 SAVE
 mr 3,31
 li 4,0
 cmpwi 3,0
 beq 1f
 /* r22 is clobbered by native lis r22,0x1017 before constructor return. */
 lwz 4,0x28(31)
1:
 bl _gz_jel_manager
 RESTORE
 mr 3,31
 b _gz_jel_manager_return
.global _gz_jel_birth_hook
_gz_jel_birth_hook:
 SAVE
 mr 3,24
 mr 4,31
 mr 5,26
 mr 6,27
 mr 7,28
 bl _gz_jel_birth
 RESTORE
 mr 3,31
 b _gz_jel_birth_return
.global _gz_jel_remove_hook
_gz_jel_remove_hook:
 SAVE
 bl _gz_jel_remove
 RESTORE
 mflr 0
 b _gz_jel_remove_return
.global _gz_jel_child_born_hook
_gz_jel_child_born_hook:
 SAVE
 mr 3,31
 bl _gz_jel_child_born
 RESTORE
 mr 3,31
 b _gz_jel_child_born_return
.global _gz_jel_child_ending_hook
_gz_jel_child_ending_hook:
 SAVE
 bl _gz_jel_child_ending
 RESTORE
 mflr 0
 b _gz_jel_child_ending_return
.global _gz_jel_edge_first_hook
_gz_jel_edge_first_hook:
 SAVE
 mr 4,3
 mr 3,31
 li 5,0x58
 bl _gz_jel_edge
 RESTORE
 stw 3,0x58(31)
 b _gz_jel_edge_first_return
.global _gz_jel_edge_second_hook
_gz_jel_edge_second_hook:
 SAVE
 mr 4,3
 mr 3,31
 li 5,0x5c
 bl _gz_jel_edge
 RESTORE
 stw 3,0x5c(31)
 b _gz_jel_edge_second_return

.global _gz_jel_constructing_hook
_gz_jel_constructing_hook:
 SAVE
 mr 3,31
 bl _gz_jel_constructing
 RESTORE
 mr. 3,31
 b _gz_jel_constructing_return
.global _gz_jel_pool_hook
_gz_jel_pool_hook:
 SAVE
 mr 4,3
 mr 3,31
 mr 5,30
 bl _gz_jel_pool
 RESTORE
 mr. 30,3
 b _gz_jel_pool_return
