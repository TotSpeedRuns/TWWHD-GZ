.section .text,"ax",@progbits
.balign 4
.global _gz_cs_original_ct
_gz_cs_original_ct:
.long 0x38000100
b _gz_cs_ct_continue
.balign 4
.global _gz_cs_original_dt
_gz_cs_original_dt:
b _gz_cs_ct
.balign 4
.global _gz_cs_original_move
_gz_cs_original_move:
.long 0x7c0802a6
b _gz_cs_move_continue
.balign 4
.global _gz_cs_original_clear
_gz_cs_original_clear:
.long 0x38000100
b _gz_cs_clear_continue
.balign 4
