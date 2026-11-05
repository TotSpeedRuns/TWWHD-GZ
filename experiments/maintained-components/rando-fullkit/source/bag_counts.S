/* Separate extension: call only under the same admitted save borrow as core.
 * Native Spoils/Bait descriptors and mutation bodies own count/list mechanics.
 * No Delivery Bag replacement. Temporary page/full UI controls are restored.
 * r28 guarded save; r3=0 complete,1 refused. No persistent state or allocation.
 */
.section .text.rando_grant_bag_counts,"ax",@progbits
.globl _gz_rando_grant_bag_counts
.type _gz_rando_grant_bag_counts,@function
_gz_rando_grant_bag_counts:
    stwu 1,-80(1)
    mflr 0
    stw 0,84(1)
    stmw 24,48(1)
    lis 8,_gz_rando_bag_request_page@ha
    lwz 24,_gz_rando_bag_request_page@l(8)
    lis 8,_gz_rando_bag_full@ha
    lwz 25,_gz_rando_bag_full@l(8)
    li 29,9
    addi 30,28,156
    li 31,8
.Lpage:
    lis 8,_gz_rando_bag_request_page@ha
    stw 29,_gz_rando_bag_request_page@l(8)
    li 26,0
.Lrow:
    li 27,10
.Lcount:
    /* Every path, including an initially99 count, executes the genuine grant
     * and then the SAME ID-membership verification. Count is not ownership. */
    cmpwi 27,0
    beq .Lfail
    addi 27,27,-1
    stw 27,40(1)
    addi 27,26,500
    bl _gz_rando_kit_native_request
    addi 8,29,-9
    mulli 8,8,36
    lis 9,_gz_rando_bag_descriptors@ha
    addi 9,9,_gz_rando_bag_descriptors@l
    add 9,9,8
    lwz 10,12(9)
    slwi 8,26,2
    lwzx 10,10,8
    lwz 9,20(9)
    add 9,28,9
    li 8,0
.Lfind:
    lbzx 11,9,8
    cmpw 11,10
    beq .Lverified
    addi 8,8,1
    cmpwi 8,8
    blt .Lfind
    b .Lfail
.Lverified:
    lwz 27,40(1)
    lbzx 8,30,26
    cmpwi 8,99
    bne .Lcount
    addi 26,26,1
    cmpw 26,31
    blt .Lrow
    cmpwi 29,10
    beq .Lok
    li 29,10
    addi 30,28,164
    li 31,2
    b .Lpage
.Lok:
    li 3,0
    b .Ldone
.Lfail:
    li 3,1
.Ldone:
    lis 8,_gz_rando_bag_request_page@ha
    stw 24,_gz_rando_bag_request_page@l(8)
    lis 8,_gz_rando_bag_full@ha
    stw 25,_gz_rando_bag_full@l(8)
    lmw 24,48(1)
    lwz 0,84(1)
    mtlr 0
    addi 1,1,80
    blr
.size _gz_rando_grant_bag_counts,.-_gz_rando_grant_bag_counts
.type _gz_rando_bag_request_page,@object
.type _gz_rando_bag_full,@object
.type _gz_rando_bag_descriptors,@object
.type _gz_rando_kit_native_request,@function
