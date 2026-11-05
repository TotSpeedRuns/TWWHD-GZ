/* Two unlinked hook contexts inside the ORIGINAL Native lookup frame.
 * No additional frame remains active while Native frees/closes/throws.
 */
.macro SAVE
 stwu 1,-160(1)
 stw 0,40(1)
 mflr 0
 stw 0,164(1)
 stw 3,44(1)
 stw 4,48(1)
 stw 5,52(1)
 stw 6,56(1)
 stw 7,60(1)
 stw 8,64(1)
 stw 9,68(1)
 stw 10,72(1)
 stw 11,76(1)
 stw 12,80(1)
 mfcr 0
 stw 0,84(1)
 mfxer 0
 stw 0,88(1)
 mfctr 0
 stw 0,92(1)
.endm
.macro RESTORE
 lwz 0,92(1)
 mtctr 0
 lwz 0,88(1)
 mtxer 0
 lwz 0,84(1)
 mtcrf 255,0
 lwz 0,164(1)
 mtlr 0
 lwz 3,44(1)
 lwz 4,48(1)
 lwz 5,52(1)
 lwz 6,56(1)
 lwz 7,60(1)
 lwz 8,64(1)
 lwz 9,68(1)
 lwz 10,72(1)
 lwz 11,76(1)
 lwz 12,80(1)
 lwz 0,40(1)
.endm
.section .text.gz_setup_usb_select_context,"ax",@progbits
.balign 4
.global gz_setup_usb_select_context
.type gz_setup_usb_select_context,@function
gz_setup_usb_select_context:
 SAVE
 /* Original lookup's saved caller LR, inside its genuine 56-byte frame. */
 lwz 11,220(1)
 lis 12,gz_setup_prepare_initial_return@ha
 addi 12,12,gz_setup_prepare_initial_return@l
 cmpw 11,12
 beq .Linitial
 lis 12,gz_setup_prepare_repair_return@ha
 addi 12,12,gz_setup_prepare_repair_return@l
 cmpw 11,12
 beq .Lvolume
 lis 12,gz_setup_prepare_copy_return@ha
 addi 12,12,gz_setup_prepare_copy_return@l
 cmpw 11,12
 beq .Lvolume
 lis 12,gz_setup_prepare_installed_return@ha
 addi 12,12,gz_setup_prepare_installed_return@l
 cmpw 11,12
 bne .Loriginal
 /* Same CR4 EQ bit consumed by Native's post-install storage check. */
 lwz 5,84(1)
 srwi 5,5,13
 andi. 5,5,1
 xori 5,5,1
 b .Lselect
.Linitial:
 /* Original saved r25 is the prepare entry choice at this exact call. */
 lwz 5,188(1)
 b .Lselect
.Lvolume:
 /* Original saved r29: same live string object passed to Native flush
  * immediately before this destination lookup. Its owner is that caller.
  */
 lwz 3,204(1)
 lwz 4,4(3)
 lwz 3,0(3)
 bl gz_setup_volume_choice
 mr 5,3
.Lselect:
 mr 3,30
 lwz 4,72(1)
 addi 6,1,104
 bl gz_setup_select_practice
 cmpwi 3,1
 beq .Lselected
 cmpwi 3,0
 beq .Labsent
 /* r27 no longer supplies titleId after this refusal. Native free/close
  * preserve it, and the original lookup unwind metadata restores its
  * caller value. No scratch flag crosses a foreign call.
  */
 stw 3,112(1)
 RESTORE
 lwz 27,112(1)
 addi 1,1,160
 b gz_setup_native_lookup_cleanup_error
.Lselected:
 lwz 4,104(1)
 mulli 4,4,97
 add 4,4,30
 stw 4,48(1)
 RESTORE
 addi 1,1,160
 b gz_setup_native_lookup_copy
.Labsent:
 RESTORE
 addi 1,1,160
 b gz_setup_native_lookup_absent
.Loriginal:
 RESTORE
 addi 1,1,160
 cmpwi 10,0
 b gz_setup_native_lookup_loop_continue
.size gz_setup_usb_select_context,.-gz_setup_usb_select_context

.section .text.gz_setup_usb_error_context,"ax",@progbits
.balign 4
.global gz_setup_usb_error_context
.type gz_setup_usb_error_context,@function
gz_setup_usb_error_context:
 SAVE
 lwz 11,220(1)
 lis 12,gz_setup_prepare_initial_return@ha
 addi 12,12,gz_setup_prepare_initial_return@l
 cmpw 11,12
 beq .Ldiagnostic
 lis 12,gz_setup_prepare_repair_return@ha
 addi 12,12,gz_setup_prepare_repair_return@l
 cmpw 11,12
 beq .Ldiagnostic
 lis 12,gz_setup_prepare_copy_return@ha
 addi 12,12,gz_setup_prepare_copy_return@l
 cmpw 11,12
 beq .Ldiagnostic
 lis 12,gz_setup_prepare_installed_return@ha
 addi 12,12,gz_setup_prepare_installed_return@l
 cmpw 11,12
 bne .Loldmessage
.Ldiagnostic:
 cmpwi 27,-1
 beq .Linput
 cmpwi 27,-2
 beq .Lother
 cmpwi 27,-3
 beq .Lambiguous
 cmpwi 27,-4
 bne .Loldmessage
 lis 4,.Lpath@ha
 addi 4,4,.Lpath@l
 b .Lmessage
.Linput:
 lis 4,.Lbadinput@ha
 addi 4,4,.Lbadinput@l
 b .Lmessage
.Lother:
 lis 4,.Lotherdevice@ha
 addi 4,4,.Lotherdevice@l
 b .Lmessage
.Lambiguous:
 lis 4,.Lduplicate@ha
 addi 4,4,.Lduplicate@l
.Lmessage:
 /* Subtract the ACTUAL relocated signed low operand of the genuine next
  * ADDI. This must also work with a non-64KiB-aligned DATA loader bias.
  */
 lis 11,gz_setup_native_lookup_default_message@ha
 addi 11,11,gz_setup_native_lookup_default_message@l
 extsh 11,11
 subf 4,11,4
 stw 4,48(1)
 RESTORE
 addi 1,1,160
 b gz_setup_native_lookup_error_continue
.Loldmessage:
 RESTORE
 addi 1,1,160
 lis 4,gz_setup_native_lookup_default_message@ha
 b gz_setup_native_lookup_error_continue
.size gz_setup_usb_error_context,.-gz_setup_usb_error_context
.section .rodata.gz_setup_usb_errors,"a",@progbits
.Lbadinput: .asciz "Practice storage selection invalid; stopped before file changes"
.Lotherdevice: .asciz "Practice is on the other storage device; select that device"
.Lduplicate: .asciz "Duplicate practice registrations on one device; refusing overwrite"
.Lpath: .asciz "Practice title path is unexpected; refusing overwrite"
