.section .text,"ax",@progbits
.balign 4
.global _gz_bg_original_regist
_gz_bg_original_regist:
.long 0x9421ffe0
b _gz_bg_regist_continue
.balign 4
.global _gz_bg_original_release
_gz_bg_original_release:
.long 0x7c0802a6
b _gz_bg_release_continue
.balign 4
.global _gz_bg_original_ct
_gz_bg_original_ct:
.long 0x7c0802a6
b _gz_bg_ct_continue
.balign 4
.global _gz_bg_original_dt
_gz_bg_original_dt:
.long 0x7c0802a6
b _gz_bg_dt_continue
.balign 4
