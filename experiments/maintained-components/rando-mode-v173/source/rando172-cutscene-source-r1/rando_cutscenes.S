/* These are native context fragments, not C ABI callbacks. */
.section .text._gz_rando_boss_warp,"ax",@progbits
.globl _gz_rando_boss_warp
.type _gz_rando_boss_warp,@function
_gz_rando_boss_warp:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_boss_warp
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 3,1
    blr
.Loff__gz_rando_boss_warp:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    mflr 0
    b _gz_rando_boss_warp_native
.size _gz_rando_boss_warp,.-_gz_rando_boss_warp
.section .text._gz_rando_song_message,"ax",@progbits
.globl _gz_rando_song_message
.type _gz_rando_song_message,@function
_gz_rando_song_message:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_song_message
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_song_message_seen
.Loff__gz_rando_song_message:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    beq .Lnative_fall__gz_rando_song_message
    b _gz_rando_song_message_seen
.Lnative_fall__gz_rando_song_message:
    b _gz_rando_song_message_unseen
.size _gz_rando_song_message,.-_gz_rando_song_message
.section .text._gz_rando_song_replay,"ax",@progbits
.globl _gz_rando_song_replay
.type _gz_rando_song_replay,@function
_gz_rando_song_replay:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_song_replay
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_song_replay_short
.Loff__gz_rando_song_replay:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    bne .Lnative_fall__gz_rando_song_replay
    b _gz_rando_song_replay_long
.Lnative_fall__gz_rando_song_replay:
    b _gz_rando_song_replay_short
.size _gz_rando_song_replay,.-_gz_rando_song_replay
.section .text._gz_rando_tott_dance,"ax",@progbits
.globl _gz_rando_tott_dance
.type _gz_rando_tott_dance,@function
_gz_rando_tott_dance:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_tott_dance
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 0,1
    b _gz_rando_tott_dance_resume
.Loff__gz_rando_tott_dance:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    li 0,2
    b _gz_rando_tott_dance_resume
.size _gz_rando_tott_dance,.-_gz_rando_tott_dance
.section .text._gz_rando_tingle_menu,"ax",@progbits
.globl _gz_rando_tingle_menu
.type _gz_rando_tingle_menu,@function
_gz_rando_tingle_menu:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_tingle_menu
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_tingle_menu_resume
.Loff__gz_rando_tingle_menu:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    stb 0,0x243(12)
    b _gz_rando_tingle_menu_resume
.size _gz_rando_tingle_menu,.-_gz_rando_tingle_menu
.section .text._gz_rando_totg_rise,"ax",@progbits
.globl _gz_rando_totg_rise
.type _gz_rando_totg_rise,@function
_gz_rando_totg_rise:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_totg_rise
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_totg_rise_finish
.Loff__gz_rando_totg_rise:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    lha 4,0xb96(30)
    b _gz_rando_totg_rise_native
.size _gz_rando_totg_rise,.-_gz_rando_totg_rise
.section .text._gz_rando_chest_buildup,"ax",@progbits
.globl _gz_rando_chest_buildup
.type _gz_rando_chest_buildup,@function
_gz_rando_chest_buildup:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_chest_buildup
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_chest_buildup_short
.Loff__gz_rando_chest_buildup:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    bne .Lnative_fall__gz_rando_chest_buildup
    b _gz_rando_chest_buildup_short
.Lnative_fall__gz_rando_chest_buildup:
    b _gz_rando_chest_buildup_long
.size _gz_rando_chest_buildup,.-_gz_rando_chest_buildup
.section .text._gz_rando_chest_event,"ax",@progbits
.globl _gz_rando_chest_event
.type _gz_rando_chest_event,@function
_gz_rando_chest_event:
    stwu 1,-16(1)
    stw 0,4(1)
    stw 11,8(1)
    mfcr 0
    stw 0,12(1)
    lis 11,_gz_rando_tools_flags@ha
    lwz 0,_gz_rando_tools_flags@l(11)
    andi. 0,0,8192

    beq .Loff__gz_rando_chest_event
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    b _gz_rando_chest_event_short
.Loff__gz_rando_chest_event:
    lwz 0,12(1)
    mtcrf 255,0
    lwz 11,8(1)
    lwz 0,4(1)
    addi 1,1,16

    beq .Lnative_fall__gz_rando_chest_event
    b _gz_rando_chest_event_long
.Lnative_fall__gz_rando_chest_event:
    b _gz_rando_chest_event_short
.size _gz_rando_chest_event,.-_gz_rando_chest_event
.type _gz_rando_tools_flags,@object
.type _gz_rando_boss_warp_native,@function
.type _gz_rando_song_message_seen,@function
.type _gz_rando_song_message_unseen,@function
.type _gz_rando_song_replay_short,@function
.type _gz_rando_song_replay_long,@function
.type _gz_rando_tott_dance_resume,@function
.type _gz_rando_tingle_menu_resume,@function
.type _gz_rando_totg_rise_finish,@function
.type _gz_rando_totg_rise_native,@function
.type _gz_rando_chest_buildup_short,@function
.type _gz_rando_chest_buildup_long,@function
.type _gz_rando_chest_event_short,@function
.type _gz_rando_chest_event_long,@function
