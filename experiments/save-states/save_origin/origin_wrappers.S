/* Compile-only fixed nonlink entries. No RPX hooks are installed. */
#include "abi_macros.inc"
.macro ARM_CALL site,note
 lis 12,_gz_spo_private@ha
 addi 12,12,_gz_spo_private@l
 lwz 0,24(12)
 cmpwi 0,0
 bne 991f
 li 0,\site
 stw 0,24(12)
 bl \note
 b 992f
991:
 bl _gz_spo_reentry_note
992:
.endm
.balign 16
.globl _gz_spo_outer
.type _gz_spo_outer,@function
_gz_spo_outer:
 stw 3,0x126c(11)
 SAVE_NATIVE
 ARM_CALL 1,_gz_spo_outer_note
 RESTORE_NATIVE
 b _gz_spo_outer_continue
.size _gz_spo_outer,.-_gz_spo_outer
.balign 16
.globl _gz_spo_pre
.type _gz_spo_pre,@function
_gz_spo_pre:
 stw 10,8(1)
 SAVE_NATIVE
 mr 3,5
 lwz 4,128(14)
 ARM_CALL 2,_gz_spo_pre_note
 RESTORE_NATIVE
 b _gz_spo_pre_continue
.size _gz_spo_pre,.-_gz_spo_pre
.balign 16
.globl _gz_spo_factory
.type _gz_spo_factory,@function
_gz_spo_factory:
 SAVE_NATIVE
 mr 3,31
 mr 4,27
 mr 5,29
 mr 6,30
 lwz 7,128(14)
 ARM_CALL 3,_gz_spo_factory_note
 RESTORE_NATIVE
 b _gz_spo_factory_continue
.size _gz_spo_factory,.-_gz_spo_factory
.balign 16
.globl _gz_spo_return
.type _gz_spo_return,@function
_gz_spo_return:
 SAVE_NATIVE
 lwz 4,128(14)
 lwz 5,116(14)
 ARM_CALL 4,_gz_spo_return_note
 RESTORE_NATIVE
 mr 29,3
 b _gz_spo_return_continue
.size _gz_spo_return,.-_gz_spo_return
.balign 16
.globl _gz_spo_ctor
.type _gz_spo_ctor,@function
_gz_spo_ctor:
 stw 31,0(29)
 SAVE_NATIVE
 mr 3,31
 mr 4,30
 mr 5,29
 lwz 6,128(14)
 ARM_CALL 5,_gz_spo_ctor_note
 RESTORE_NATIVE
 b _gz_spo_ctor_continue
.size _gz_spo_ctor,.-_gz_spo_ctor
.balign 16
.globl _gz_spo_initpre
.type _gz_spo_initpre,@function
_gz_spo_initpre:
 SAVE_NATIVE
 mr 3,31
 lwz 4,128(14)
 lwz 5,116(14)
 ARM_CALL 6,_gz_spo_initpre_note
 RESTORE_NATIVE
 mr 3,31
 b _gz_spo_initpre_continue
.size _gz_spo_initpre,.-_gz_spo_initpre
.balign 16
.globl _gz_spo_initpost
.type _gz_spo_initpost,@function
_gz_spo_initpost:
 SAVE_NATIVE
 mr 3,31
 lwz 4,128(14)
 lwz 5,116(14)
 ARM_CALL 7,_gz_spo_initpost_note
 RESTORE_NATIVE
 li 3,0
 b _gz_spo_initpost_continue
.size _gz_spo_initpost,.-_gz_spo_initpost
.balign 16
.globl _gz_spo_dtor
.type _gz_spo_dtor,@function
_gz_spo_dtor:
 SAVE_NATIVE
 ARM_CALL 8,_gz_spo_dtor_note
 RESTORE_NATIVE
 mflr 0
 b _gz_spo_dtor_continue
.size _gz_spo_dtor,.-_gz_spo_dtor
