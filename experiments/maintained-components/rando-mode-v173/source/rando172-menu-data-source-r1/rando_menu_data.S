.section .text.gz_rando_menu_data,"ax",@progbits
.balign 4
.global _gz_rando_tools_mode_name
.type _gz_rando_tools_mode_name,@object
_gz_rando_tools_mode_name:
.asciz "Rando Mode"
.size _gz_rando_tools_mode_name,.-_gz_rando_tools_mode_name
.balign 4
.global _gz_rando_tools_display_bits
.type _gz_rando_tools_display_bits,@object
_gz_rando_tools_display_bits:
.long 0,1,2,3,9,4,5,6,7,8,13,11
.size _gz_rando_tools_display_bits,.-_gz_rando_tools_display_bits
