# Small ABI/control-flow bridges; all native writes stay behind inventory guards.
.section .text,"ax",@progbits
.balign 4
.global _gz_btl_inventory_hook
_gz_btl_inventory_hook:
 stwu 1,-64(1)
 mflr 0
 stw 0,68(1)
 stw 3,8(1)
 stw 4,12(1)
 stw 5,16(1)
 stw 6,20(1)
 stw 7,24(1)
 stw 8,28(1)
 stw 9,32(1)
 stw 10,36(1)
 stw 11,40(1)
 stw 12,44(1)
 mr 3,8
 lwz 4,0(31)
 bl _gz_btl_inventory_input
 cmpwi 3,0
 lwz 3,8(1)
 lwz 4,12(1)
 lwz 5,16(1)
 lwz 6,20(1)
 lwz 7,24(1)
 lwz 8,28(1)
 lwz 9,32(1)
 lwz 10,36(1)
 lwz 11,40(1)
 lwz 12,44(1)
 lwz 0,68(1)
 mtlr 0
 addi 1,1,64
 bne _gz_btl_inventory_consumed
 b _gz_btl_inventory_original
_gz_btl_inventory_consumed:
 b _gz_mask

.global _gz_btl_page_input
_gz_btl_page_input:
 lwz 9,0(29)
 cmpwi 9,0
 bne _gz_btl_page_active
 b _gz_input_done
_gz_btl_page_active:
 bl _gz_inventory_dpad_read
 lwz 3,0x10(1)
 lwz 4,0(31)
 lis 5,_gz_inventory_dpad_repeat@ha
 lwz 5,_gz_inventory_dpad_repeat@l(5)
 bl _gz_btl_contents_input
 b _gz_mask

.global _gz_btl_dispatch_apply
_gz_btl_dispatch_apply:
 cmplwi 27,960
 bge _gz_btl_dispatch_lower_ok
 b _gz_btl_apply_ordinary
_gz_btl_dispatch_lower_ok:
 cmplwi 27,1003
 ble _gz_btl_dispatch_range_ok
 b _gz_btl_apply_ordinary
_gz_btl_dispatch_range_ok:
 mr 3,27
 mr 4,28
 bl _gz_btl_apply
 b _gz_inv_apply_done
