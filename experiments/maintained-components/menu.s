# Main menu grouping only. Existing feature controllers keep their ownership.
# Pages58/59 are presentation pages; entering them grants no native authority.
.section .text,"ax",@progbits
.balign 4
.global _gz_tmg_root_select
_gz_tmg_root_select:
 cmplwi 10,7
 bgt _gz_tmg_root_refuse
 cmpwi 10,6
 bge _gz_tmg_root_group
 # Retain the old destinations and their existing Select/inventory guards.
 cmpwi 10,2
 blt _gz_tmg_root_legacy
 addi 10,10,1
_gz_tmg_root_legacy:
 b _gz_tmg_root_selected_original
_gz_tmg_root_group:
 # New group rows obey the same configured Select edge+held chord.
 lis 9,_gz_bind_masks@ha
 addi 9,9,_gz_bind_masks@l
 lwz 9,20(9)
 and. 0,8,9
 beq _gz_tmg_root_refuse
 lwz 0,0(31)
 and 0,0,9
 cmpw 0,9
 bne _gz_tmg_root_refuse
 addi 10,10,52
 b _gz_enter_custom
_gz_tmg_root_refuse:
 b _gz_mask

.global _gz_tmg_custom_select
_gz_tmg_custom_select:
 cmpwi 12,58
 beq _gz_tmg_group_selected
 cmpwi 12,59
 beq _gz_tmg_group_selected
 b _gz_tmg_custom_selected_original
_gz_tmg_group_selected:
 cmplwi 10,1
 bgt _gz_tmg_group_refuse
 lis 9,_gz_bind_masks@ha
 addi 9,9,_gz_bind_masks@l
 lwz 9,20(9)
 and. 0,8,9
 beq _gz_tmg_group_refuse
 lwz 0,0(31)
 and 0,0,9
 cmpw 0,9
 bne _gz_tmg_group_refuse
 cmpwi 12,59
 beq _gz_tmg_misc_selected
 cmpwi 10,0
 bne _gz_tmg_movement_selected
 b _gz_fad_enter_menu
_gz_tmg_movement_selected:
 b _gz_mvt_enter_menu
_gz_tmg_misc_selected:
 cmpwi 10,0
 bne _gz_tmg_binds_selected
 b _gz_ccv_toggle_menu
_gz_tmg_binds_selected:
 b _gz_enter_binds
_gz_tmg_group_refuse:
 b _gz_mask

.global _gz_tmg_back
_gz_tmg_back:
 cmpwi 10,6
 beq _gz_tmg_binds_back
 cmpwi 10,7
 beq _gz_tmg_tools_back
 cmpwi 10,58
 beq _gz_tmg_testing_back
 cmpwi 10,59
 beq _gz_tmg_misc_back
 b _gz_tmg_custom_back_original
_gz_tmg_binds_back:
 li 10,59
 li 11,1
 b _gz_tmg_back_store
_gz_tmg_tools_back:
 li 10,0
 li 11,2
 b _gz_tmg_back_store
_gz_tmg_testing_back:
 li 10,0
 li 11,6
 b _gz_tmg_back_store
_gz_tmg_misc_back:
 li 10,0
 li 11,7
_gz_tmg_back_store:
 lis 9,_gz_page@ha
 stw 10,_gz_page@l(9)
 stw 11,8(29)
 b _gz_mask

# Draw only bounded static tables through the existing menu text/cursor API.
# Main rows3..10 are a strict reduction from the former rows3..12.
# Each new group has two rows4..5. The trainer Tools page is untouched.
.global _gz_tmg_draw_root
_gz_tmg_draw_root:
 lis 6,_gz_title@ha
 addi 6,6,_gz_title@l
 lis 8,_gz_tmg_root_names@ha
 addi 8,8,_gz_tmg_root_names@l
 li 9,8
 li 10,3
 li 11,0
 b _gz_tmg_draw_table
.global _gz_tmg_draw_group
_gz_tmg_draw_group:
 cmpwi 8,59
 beq _gz_tmg_draw_misc
 lis 6,_gz_tmg_testing_title@ha
 addi 6,6,_gz_tmg_testing_title@l
 lis 8,_gz_tmg_testing_names@ha
 addi 8,8,_gz_tmg_testing_names@l
 li 11,0
 b _gz_tmg_draw_group_ready
_gz_tmg_draw_misc:
 lis 6,_gz_tmg_misc_title@ha
 addi 6,6,_gz_tmg_misc_title@l
 lis 8,_gz_tmg_misc_names@ha
 addi 8,8,_gz_tmg_misc_names@l
 li 11,1
_gz_tmg_draw_group_ready:
 li 9,2
 li 10,4
_gz_tmg_draw_table:
 stwu 1,-0x40(1)
 mflr 0
 stw 0,0x44(1)
 stw 27,0x20(1)
 stw 28,0x24(1)
 stw 30,0x28(1)
 stw 31,0x2c(1)
 stw 10,8(1)
 stw 11,12(1)
 mr 28,8
 mr 30,9
 li 3,0
 li 4,2
 li 5,1
 bl _gz_game_text
 li 27,0
_gz_tmg_draw_row:
 slwi 8,27,2
 lwzx 6,28,8
 lwz 8,12(1)
 cmpwi 8,0
 beq _gz_tmg_draw_label_ready
 cmpwi 27,0
 bne _gz_tmg_draw_label_ready
 # Creator Code is a display toggle, not an extra page/controller.
 lis 8,_gz_ccv_visible@ha
 lwz 8,_gz_ccv_visible@l(8)
 lis 6,_gz_tmg_creator_off@ha
 addi 6,6,_gz_tmg_creator_off@l
 cmpwi 8,0
 beq _gz_tmg_draw_label_ready
 lis 6,_gz_tmg_creator_on@ha
 addi 6,6,_gz_tmg_creator_on@l
_gz_tmg_draw_label_ready:
 li 3,0
 li 4,4
 lwz 5,8(1)
 add 5,5,27
 bl _gz_game_text
 lwz 8,8(29)
 cmpw 8,27
 bne _gz_tmg_draw_next
 li 3,0
 li 4,2
 lwz 5,8(1)
 add 5,5,27
 lis 6,_gz_marker@ha
 addi 6,6,_gz_marker@l
 bl _gz_game_text
_gz_tmg_draw_next:
 addi 27,27,1
 cmpw 27,30
 blt _gz_tmg_draw_row
 lwz 27,0x20(1)
 lwz 28,0x24(1)
 lwz 30,0x28(1)
 lwz 31,0x2c(1)
 lwz 0,0x44(1)
 mtlr 0
 addi 1,1,0x40
 blr

.section .data,"aw",@progbits
.balign 4
.global _gz_tmg_root_names
_gz_tmg_root_names:
 .long _gz_warps_label,_gz_routes_label,_gz_tools_label,_gz_inventory_title
 .long _gz_mht_title,_gz_tmg_favorites_title,_gz_tmg_testing_title,_gz_tmg_misc_title
.global _gz_tmg_testing_names
_gz_tmg_testing_names:
 .long _gz_tmg_frame_title,_gz_tmg_movement_title
.global _gz_tmg_misc_names
_gz_tmg_misc_names:
 .long _gz_tmg_creator_off,_gz_binds_label
_gz_tmg_testing_title: .string "Testing Tools"
_gz_tmg_misc_title: .string "Misc"
_gz_tmg_favorites_title: .string "Favorites"
_gz_tmg_frame_title: .string "Frame Advance"
_gz_tmg_movement_title: .string "Movement"
_gz_tmg_creator_on: .string "Show Creator Code: On"
_gz_tmg_creator_off: .string "Show Creator Code: Off"
.balign 4
