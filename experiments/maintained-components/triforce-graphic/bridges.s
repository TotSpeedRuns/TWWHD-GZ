.section .text,"ax",@progbits
.balign 4
.global _gz_tfg_inventory_bridge
_gz_tfg_inventory_bridge:
    stwu 1,-16(1)
    mflr 0
    stw 0,20(1)
    bl _gz_shards_inventory_draw
    lwz 3,12(29)
    lbz 4,0xBE(28)
    li 5,8
    li 6,0
    bl _gz_tfg_draw_parts
    lwz 0,20(1)
    mtlr 0
    addi 1,1,16
    blr

.global _gz_tfg_shards_details
_gz_tfg_shards_details:
    stwu 1,-16(1)
    mflr 0
    stw 0,20(1)
    lwz 3,12(29)
    lbz 4,0xBE(28)
    mr 5,26
    li 6,1
    bl _gz_tfg_draw_parts
    cmplwi 26,7
    bgt _gz_tfg_details_done
    lis 8,_gz_shards_names@ha
    addi 8,8,_gz_shards_names@l
    slwi 9,26,2
    lwzx 6,8,9
    li 4,680
    li 5,255
    bl _gz_inv_text_small
    li 8,1
    slw 8,8,26
    lbz 9,0xBE(28)
    and. 8,8,9
    lis 6,_gz_shards_missing@ha
    addi 6,6,_gz_shards_missing@l
    beq _gz_tfg_details_status
    lis 6,_gz_shards_owned@ha
    addi 6,6,_gz_shards_owned@l
_gz_tfg_details_status:
    li 4,680
    li 5,300
    bl _gz_inv_text_small
    lis 6,_gz_tfg_source_label@ha
    addi 6,6,_gz_tfg_source_label@l
    li 4,680
    li 5,375
    bl _gz_inv_text_small
    mr 3,26
    li 4,0
    bl _gz_tfg_source_line
    mr 6,3
    li 4,680
    li 5,415
    bl _gz_inv_text_small
    mr 3,26
    li 4,1
    bl _gz_tfg_source_line
    mr 6,3
    li 4,680
    li 5,455
    bl _gz_inv_text_small
_gz_tfg_details_done:
    lwz 0,20(1)
    mtlr 0
    addi 1,1,16
    blr

.section .rodata,"a",@progbits
_gz_tfg_source_label:
    .asciz "Source:"
