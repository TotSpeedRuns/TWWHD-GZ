/* Pure helpers only inside these112B frames. Pop before any Native C++ call,
 * cleanup, filesystem error or throw. Replay each displaced instruction.
 */
.macro ENTER
 stwu 1,-112(1)
 stw 0,80(1)
 stw 3,40(1)
 stw 4,44(1)
 stw 5,48(1)
 stw 6,52(1)
 stw 7,56(1)
 stw 8,60(1)
 stw 9,64(1)
 stw 10,68(1)
 stw 11,72(1)
 stw 12,76(1)
 mfcr 0
 stw 0,84(1)
 mflr 0
 stw 0,88(1)
 mfctr 0
 stw 0,92(1)
 mfxer 0
 stw 0,96(1)
.endm
.macro EXIT
 lwz 0,96(1)
 mtxer 0
 lwz 0,92(1)
 mtctr 0
 lwz 0,88(1)
 mtlr 0
 lwz 0,84(1)
 mtcrf 255,0
 lwz 3,40(1)
 lwz 4,44(1)
 lwz 5,48(1)
 lwz 6,52(1)
 lwz 7,56(1)
 lwz 8,60(1)
 lwz 9,64(1)
 lwz 10,68(1)
 lwz 11,72(1)
 lwz 12,76(1)
 lwz 0,80(1)
 addi 1,1,112
.endm
.section .text.mig_owned_cmp,"ax",@progbits
.globl _gz_mig_owned_cmp
.type _gz_mig_owned_cmp,@function
_gz_mig_owned_cmp:
 ENTER
 cmpwi 3,0
 bne .Lowned_done
 cmpwi 29,0
 beq .Lowned_done
 /* Native owned336B frame digest std::string at+228, alive before cleanup. */
 lwz 3,340(1)
 lwz 4,344(1)
 li 5,0
 bl _gz_setup_role_known
 stw 3,40(1)
.Lowned_done:
 EXIT
 cmpwi 3,0
 b _gz_mig_owned_resume
.size _gz_mig_owned_cmp,.-_gz_mig_owned_cmp
.section .text.mig_prepare_cmp,"ax",@progbits
.globl _gz_mig_prepare_cmp
.type _gz_mig_prepare_cmp,@function
_gz_mig_prepare_cmp:
 ENTER
 cmpwi 3,0
 bne .Lprepare_done
 /* Real prepare2560B frame fresh RPX digest at+2180. */
 lwz 3,2292(1)
 lwz 4,2296(1)
 li 5,0
 bl _gz_setup_role_known
 stw 3,40(1)
.Lprepare_done:
 EXIT
 cmpwi 3,0
 b _gz_mig_prepare_resume
.size _gz_mig_prepare_cmp,.-_gz_mig_prepare_cmp
.section .text.mig_probe_unknown,"ax",@progbits
.globl _gz_mig_probe_unknown
.type _gz_mig_probe_unknown,@function
_gz_mig_probe_unknown:
 ENTER
 /* Only original Unknown result reaches20106e4; prior classifications bypass. */
 mr 3,31
 lwz 4,152(1)
 li 5,1
 bl _gz_setup_role_known
 li 9,3
 cmpwi 3,0
 beq .Lprobe_done
 li 9,1
.Lprobe_done:
 stw 9,64(1)
 EXIT
 b _gz_mig_probe_resume
.size _gz_mig_probe_unknown,.-_gz_mig_probe_unknown
.type _gz_mig_owned_resume,@function
.type _gz_mig_prepare_resume,@function
.type _gz_mig_probe_resume,@function
