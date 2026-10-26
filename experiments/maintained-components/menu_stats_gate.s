# Menu text owns the bounded native text queue while a GZ menu is open.
# Debug/Input Viewer flags are retained and their HUD resumes on menu close.
.section .text,"ax",@progbits
.balign 4
.global _gz_mqp_stats
_gz_mqp_stats:
 lis 8,_gz_state@ha
 lwz 8,_gz_state@l(8)
 cmpwi 8,0
 bnelr
 b _gz_draw_stats
