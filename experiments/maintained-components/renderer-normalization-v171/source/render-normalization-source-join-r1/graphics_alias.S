.section .text.gz_rcw_graphics_held,"ax",@progbits
.align 2
.global _gz_rcw_graphics_held
.type _gz_rcw_graphics_held,@function
_gz_rcw_graphics_held:
    b _gz_gfxq_owned
.size _gz_rcw_graphics_held,.-_gz_rcw_graphics_held
