/* Synchronous kit inside the original Native inventory callback. The caller
 * supplies its current Save member in r28 after the original busy/live/route
 * checks. These checks are not newly acquired locks or physical holds.
 * No retained borrow, pending writes, allocation, SDK call or mode store.
 * r3==0 only after Native row progression and final kit checks complete.
 */
.section .text.rando_grant_kit,"ax",@progbits
.globl _gz_rando_grant_kit
.type _gz_rando_grant_kit,@function
_gz_rando_grant_kit:
    stwu 1,-80(1)
    mflr 0
    stw 0,84(1)
    stmw 25,52(1)
    /* Game's item-get providers use gameInfo; bind them to this same borrow. */
    lis 8,_gz_rando_kit_game_info@ha
    lwz 8,_gz_rando_kit_game_info@l(8)
    addi 8,8,32
    cmplw 8,28
    bne .Lfail
    li 29,1
.Lrow:
    addi 8,29,-1
    mulli 8,8,20
    lis 30,_gz_rando_kit_native_records@ha
    addi 30,30,_gz_rando_kit_native_records@l
    add 30,30,8
    lwz 31,8(30)
    cmplwi 31,2
    blt .Lfail
    cmplwi 31,5
    bgt .Lfail
    mr 26,31
    addi 31,31,-1
.Llevel:
    mr 3,30
    mr 4,28
    bl _gz_rando_kit_native_level
    cmpw 3,31
    beq .Lnextrow
    bgt .Lfail
    cmpwi 26,0
    beq .Lfail
    addi 26,26,-1
    mr 27,29
    bl _gz_rando_kit_native_request
    b .Llevel
.Lnextrow:
    addi 29,29,1
    cmpwi 29,24
    ble .Lrow
    /* Genuine Song requests are additive, not the930..935 removals. */
    li 27,920
.Lsong:
    bl _gz_rando_kit_native_request
    addi 27,27,1
    cmpwi 27,925
    ble .Lsong
    li 29,0
.Ltriforce:
    li 8,1
    slw 8,8,29
    lbz 9,190(28)
    and. 8,8,9
    bne .Lnextpiece
    addi 27,29,940
    bl _gz_rando_kit_native_request
.Lnextpiece:
    addi 29,29,1
    cmpwi 29,8
    blt .Ltriforce
    /* Actual item-get table IDs: max wallet, bomb bag and quiver. */
    lbz 8,18(28)
    cmpwi 8,2
    beq .Lbombs
    li 3,172
    bl _gz_rando_kit_native_item_get
.Lbombs:
    lbz 8,112(28)
    cmpwi 8,99
    beq .Larrows
    li 3,174
    bl _gz_rando_kit_native_item_get
.Larrows:
    lbz 8,111(28)
    cmpwi 8,99
    beq .Lcharms
    li 3,176
    bl _gz_rando_kit_native_item_get
.Lcharms:
    lbz 8,183(28)
    andi. 8,8,1
    bne .Lhero
    li 3,66
    bl _gz_rando_kit_native_item_get
.Lhero:
    lbz 8,184(28)
    andi. 8,8,1
    bne .Lpearls
    li 3,67
    bl _gz_rando_kit_native_item_get
.Lpearls:
    li 29,0
.Lpearl:
    li 8,1
    slw 8,8,29
    lbz 9,191(28)
    and. 8,8,9
    bne .Lnextpearl
    addi 3,29,105
    bl _gz_rando_kit_native_item_get
.Lnextpearl:
    addi 29,29,1
    cmpwi 29,3
    blt .Lpearl
    /*903 clamps at32;905 cycles and is deliberately not used. */
    li 27,903
    bl _gz_rando_kit_native_request
    bl _gz_rando_kit_native_request
    /*127 grows max health by one heart and clamps at80 quarter-hearts. */
    li 25,20
    li 27,127
.Lhealth:
    bl _gz_rando_kit_native_request
    addic. 25,25,-1
    bne .Lhealth
    li 27,700
.Lrefill:
    bl _gz_rando_kit_native_request
    addi 27,27,1
    cmpwi 27,703
    ble .Lrefill
    lbz 8,189(28)
    andi. 8,8,63
    cmpwi 8,63
    bne .Lfail
    lbz 8,190(28)
    cmpwi 8,255
    bne .Lfail
    lbz 8,191(28)
    andi. 8,8,7
    cmpwi 8,7
    bne .Lfail
    lbz 8,18(28)
    cmpwi 8,2
    bne .Lfail
    lbz 8,111(28)
    cmpwi 8,99
    bne .Lfail
    lbz 8,112(28)
    cmpwi 8,99
    bne .Lfail
    lbz 8,183(28)
    andi. 8,8,1
    beq .Lfail
    lbz 8,184(28)
    andi. 8,8,1
    beq .Lfail
    lbz 8,19(28)
    cmpwi 8,32
    bne .Lfail
    lhz 8,0(28)
    cmpwi 8,80
    bne .Lfail
    li 3,0
    b .Ldone
.Lfail:
    li 3,1
.Ldone:
    lmw 25,52(1)
    lwz 0,84(1)
    mtlr 0
    addi 1,1,80
    blr
.size _gz_rando_grant_kit,.-_gz_rando_grant_kit

/* Build exactly the native inventory consumer's own48B frame, then enter its
 * unchanged post-admission Song/Magic/ordinary dispatcher. Native done restores
 * that frame and returns here to the caller; genuine native mutation bodies,
 * progression bits and equipped-button cache updates all execute unchanged.
 */
.section .text.rando_kit_native_request,"ax",@progbits
.globl _gz_rando_kit_native_request
.type _gz_rando_kit_native_request,@function
_gz_rando_kit_native_request:
    stwu 1,-48(1)
    mflr 0
    stw 0,52(1)
    stmw 27,28(1)
    b _gz_rando_kit_native_dispatch
.size _gz_rando_kit_native_request,.-_gz_rando_kit_native_request

.type _gz_rando_kit_native_records,@object
.type _gz_rando_kit_native_level,@function
.type _gz_rando_kit_native_dispatch,@function
.type _gz_rando_kit_native_item_get,@function
.type _gz_rando_kit_game_info,@object
