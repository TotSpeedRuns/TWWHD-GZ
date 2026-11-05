/* Context fragment at the last clean GHW boundary (native171:02b0bd54).
 * r31=request, r30=frame. Not a stand-alone C entry point.
 * All original state is restored before replay/continuation or refusal.
 */
.section .text.gz_rando_held_config,"ax",@progbits
.balign 4
.global _gz_rando_held_config_hook
.type _gz_rando_held_config_hook,@function
.type _gz_rns_held_config,@function
.type _gz_rando_ghw_resume,@function
.type _gz_rando_ghw_refuse,@function
_gz_rando_held_config_hook:
 stwu 1,-160(1)
 stw 0,64(1)
 stw 3,68(1)
 stw 4,72(1)
 stw 5,76(1)
 stw 6,80(1)
 stw 7,84(1)
 stw 8,88(1)
 stw 9,92(1)
 stw 10,96(1)
 stw 11,100(1)
 stw 12,104(1)
 mfcr 0
 stw 0,108(1)
 mflr 0
 stw 0,112(1)
 mfctr 0
 stw 0,116(1)
 mfxer 0
 stw 0,120(1)
 mr 3,31
 mr 4,30
 bl _gz_rns_held_config
 cmpwi 3,0
 bne .Lrefuse
.macro restore
 lwz 0,120(1)
 mtxer 0
 lwz 0,116(1)
 mtctr 0
 lwz 0,112(1)
 mtlr 0
 lwz 0,108(1)
 mtcrf 255,0
 lwz 3,68(1)
 lwz 4,72(1)
 lwz 5,76(1)
 lwz 6,80(1)
 lwz 7,84(1)
 lwz 8,88(1)
 lwz 9,92(1)
 lwz 10,96(1)
 lwz 11,100(1)
 lwz 12,104(1)
 lwz 0,64(1)
 addi 1,1,160
.endm
 restore
 li 7,1
 b _gz_rando_ghw_resume
.Lrefuse:
 restore
 li 4,11
 li 3,0xb4e
 b _gz_rando_ghw_refuse
.size _gz_rando_held_config_hook,.-_gz_rando_held_config_hook
