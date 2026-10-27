# Called only by the existing ROH/live/current-save inventory apply path.
# r28=current owned save, r30=Delivery descriptor, r31=item, r9=decoded mode.
# No retail API is called; C plans detached byte copies only.
.section .text,"ax",@progbits
.balign 4
.global _gz_del_apply_bridge
_gz_del_apply_bridge:
 stwu 1,-128(1)
 mflr 0
 stw 0,132(1)
 stw 9,96(1)
 li 8,0
 addi 10,1,8
 addi 11,28,0x86
_gz_del_copy_slots_in:
 lbzx 9,11,8
 stbx 9,10,8
 addi 8,8,1
 cmpwi 8,8
 blt _gz_del_copy_slots_in
 lwz 9,0x90(28)
 stw 9,16(1)
 li 8,0
 addi 10,1,20
 addi 11,28,9
 lis 12,_gz_equip_store@ha
 addi 12,12,_gz_equip_store@l
_gz_del_copy_bindings_in:
 lbzx 9,11,8
 stbx 9,10,8
 lbzx 9,12,8
 addi 7,10,5
 stbx 9,7,8
 addi 8,8,1
 cmpwi 8,5
 blt _gz_del_copy_bindings_in
 li 8,0
 sth 8,30(1)
 addi 3,1,8
 mr 4,31
 lwz 5,96(1)
 addi 6,1,32
 bl _gz_del_plan
 li 8,0
 lis 9,_gz_bag_full@ha
 stw 8,_gz_bag_full@l(9)
 cmpwi 3,1
 beq _gz_del_apply_full
 cmpwi 3,0
 bne _gz_del_apply_done
 lwz 8,68(1)
 cmpwi 8,0
 beq _gz_del_apply_done
 li 8,0
 addi 10,1,32
 addi 11,28,0x86
_gz_del_copy_slots_out:
 lbzx 9,10,8
 stbx 9,11,8
 addi 8,8,1
 cmpwi 8,8
 blt _gz_del_copy_slots_out
 lwz 9,40(1)
 stw 9,0x90(28)
 li 8,0
 lwz 7,76(1)
 addi 10,1,44
 addi 11,28,9
 lis 12,_gz_equip_store@ha
 addi 12,12,_gz_equip_store@l
_gz_del_copy_bindings_out:
 li 9,1
 slw 9,9,8
 and. 9,9,7
 beq _gz_del_next_binding
 lbzx 9,10,8
 stbx 9,11,8
 addi 6,10,5
 lbzx 9,6,8
 stbx 9,12,8
_gz_del_next_binding:
 addi 8,8,1
 cmpwi 8,5
 blt _gz_del_copy_bindings_out
 lwz 8,72(1)
 cmpwi 8,0
 bne _gz_del_apply_grant
 b _gz_del_apply_done
_gz_del_apply_full:
 li 8,1
 lis 9,_gz_bag_full@ha
 stw 8,_gz_bag_full@l(9)
_gz_del_apply_done:
 lwz 0,132(1)
 mtlr 0
 addi 1,1,128
 b _gz_inv_apply_done
_gz_del_apply_grant:
 lwz 0,132(1)
 mtlr 0
 addi 1,1,128
 b _gz_bag_grant
