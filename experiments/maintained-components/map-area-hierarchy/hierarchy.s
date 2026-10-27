# SPDX-License-Identifier: MIT
# Authored area-folder menu module. See RUNTIME_CONTRACT.md before linking.


.section .text,"ax",@progbits
.balign 4
.global _gz_mah_descriptor
.type _gz_mah_descriptor,@function
# Return the active descriptor in r12; r9 and CR0 are volatile.
_gz_mah_descriptor:
    lis 12, _gz_mah_state@ha
    lwz 9, _gz_mah_state@l(12)
    cmplwi 9, 14
    bge .Lmah_root_descriptor
    mulli 9, 9, 12
    lis 12, _gz_mah_folders@ha
    addi 12, 12, _gz_mah_folders@l
    add 12, 12, 9
    blr
.Lmah_root_descriptor:
    lis 9, _gz_map_sector@ha
    lwz 9, _gz_map_sector@l(9)
    cmplwi 9, 48
    ble .Lmah_valid_sector
    li 9, 0
.Lmah_valid_sector:
    mulli 9, 9, 12
    lis 12, _gz_mah_roots@ha
    addi 12, 12, _gz_mah_roots@l
    add 12, 12, 9
    blr
.size _gz_mah_descriptor,.-_gz_mah_descriptor
.global _gz_mah_open_folder
.type _gz_mah_open_folder,@function
# r10 is a tagged folder ID; r29 is the existing menu-state base.
# This tail path exits through the existing input mask.
_gz_mah_open_folder:
    clrlwi 10, 10, 1
    cmplwi 10, 14
    blt .Lmah_folder_valid
    b _gz_mask
.Lmah_folder_valid:
    lis 9, _gz_mah_state@ha
    addi 9, 9, _gz_mah_state@l
    stw 10, 0(9)
    lwz 10, 8(29)
    stw 10, 4(9)
    li 10, 0
    stw 10, 8(29)
    b _gz_mask
.size _gz_mah_open_folder,.-_gz_mah_open_folder
.global _gz_mah_back
.type _gz_mah_back,@function
# Folder -> parent row; sector list -> grid; grid -> menu root.
_gz_mah_back:
    lis 9, _gz_page@ha
    lwz 10, _gz_page@l(9)
    cmpwi 10, 14
    beq .Lmah_back_list
    li 10, 0
    stw 10, _gz_page@l(9)
    stw 10, 8(29)
    b _gz_mask
.Lmah_back_list:
    lis 11, _gz_mah_state@ha
    addi 11, 11, _gz_mah_state@l
    lwz 10, 0(11)
    cmplwi 10, 14
    bge .Lmah_back_map
    li 10, -1
    stw 10, 0(11)
    lwz 10, 4(11)
    stw 10, 8(29)
    b _gz_mask
.Lmah_back_map:
    li 10, 13
    stw 10, _gz_page@l(9)
    li 10, 0
    stw 10, 8(29)
    b _gz_mask
.size _gz_mah_back,.-_gz_mah_back
.section .data,"aw",@progbits
.balign 4
.global _gz_mah_state
# Initialized state: active folder (-1 at sector root), parent cursor.
_gz_mah_state:
    .long -1, 0
.section .rodata,"a",@progbits
.balign 4
.global _gz_mah_roots
# 49 descriptors: title pointer, first row index, row count.
_gz_mah_roots:
    .long _gz_map_quadrant_name0, 0, 4
    .long _gz_map_quadrant_name1, 17, 2
    .long _gz_map_quadrant_name2, 19, 3
    .long _gz_map_quadrant_name3, 22, 3
    .long _gz_map_quadrant_name4, 36, 2
    .long _gz_map_quadrant_name5, 38, 1
    .long _gz_map_quadrant_name6, 39, 2
    .long _gz_map_quadrant_name7, 41, 1
    .long _gz_map_quadrant_name8, 42, 0
    .long _gz_map_quadrant_name9, 42, 1
    .long _gz_map_quadrant_name10, 43, 4
    .long _gz_map_quadrant_name11, 57, 3
    .long _gz_map_quadrant_name12, 60, 7
    .long _gz_map_quadrant_name13, 79, 3
    .long _gz_map_quadrant_name14, 82, 2
    .long _gz_map_quadrant_name15, 84, 2
    .long _gz_map_quadrant_name16, 86, 2
    .long _gz_map_quadrant_name17, 88, 1
    .long _gz_map_quadrant_name18, 89, 2
    .long _gz_map_quadrant_name19, 91, 2
    .long _gz_map_quadrant_name20, 93, 1
    .long _gz_map_quadrant_name21, 94, 1
    .long _gz_map_quadrant_name22, 95, 1
    .long _gz_map_quadrant_name23, 96, 1
    .long _gz_map_quadrant_name24, 97, 2
    .long _gz_map_quadrant_name25, 99, 13
    .long _gz_map_quadrant_name26, 134, 1
    .long _gz_map_quadrant_name27, 135, 2
    .long _gz_map_quadrant_name28, 137, 2
    .long _gz_map_quadrant_name29, 139, 2
    .long _gz_map_quadrant_name30, 141, 3
    .long _gz_map_quadrant_name31, 144, 1
    .long _gz_map_quadrant_name32, 145, 3
    .long _gz_map_quadrant_name33, 148, 4
    .long _gz_map_quadrant_name34, 152, 2
    .long _gz_map_quadrant_name35, 154, 2
    .long _gz_map_quadrant_name36, 156, 1
    .long _gz_map_quadrant_name37, 157, 2
    .long _gz_map_quadrant_name38, 159, 2
    .long _gz_map_quadrant_name39, 161, 3
    .long _gz_map_quadrant_name40, 164, 9
    .long _gz_map_quadrant_name41, 187, 3
    .long _gz_map_quadrant_name42, 190, 2
    .long _gz_map_quadrant_name43, 192, 8
    .long _gz_map_quadrant_name44, 214, 3
    .long _gz_map_quadrant_name45, 224, 1
    .long _gz_map_quadrant_name46, 225, 2
    .long _gz_map_quadrant_name47, 227, 3
    .long _gz_map_quadrant_name48, 230, 2
.global _gz_mah_folders
# 14 area descriptors using the same 12-byte layout.
# FF1=0, FF2=12 and FF3=13; all other folder IDs retain their meanings.
_gz_mah_folders:
    .long _gz_mah_label0, 4, 7
    .long _gz_mah_label2, 25, 11
    .long _gz_mah_label4, 47, 8
    .long _gz_mah_label5, 55, 2
    .long _gz_mah_label8, 67, 12
    .long _gz_mah_label10, 112, 3
    .long _gz_mah_label11, 115, 19
    .long _gz_mah_label14, 173, 6
    .long _gz_mah_label15, 179, 8
    .long _gz_mah_label18, 200, 6
    .long _gz_mah_label19, 206, 8
    .long _gz_mah_label22, 217, 7
    .long _gz_mah_label24, 11, 5
    .long _gz_mah_label26, 16, 1
.global _gz_mah_rows
# 232 rows: label pointer, original warp ID or 0x80000000 | folder ID.
_gz_mah_rows:
    .long _gz_mah_label28, 36
    .long _gz_mah_label1, 0x80000000
    .long _gz_mah_label25, 0x8000000c
    .long _gz_mah_label27, 0x8000000d
    .long _gz_map_destination_name1, 109
    .long _gz_map_destination_name2, 110
    .long _gz_tw_label22, 145
    .long _gz_tw_label124, 196
    .long _gz_tw_label126, 197
    .long _gz_tw_label128, 198
    .long _gz_tw_label130, 199
    .long _gz_map_destination_name3, 111
    .long _gz_map_destination_name4, 112
    .long _gz_map_destination_name5, 113
    .long _gz_tw_label18, 143
    .long _gz_tw_label158, 213
    .long _gz_tw_label20, 144
    .long _gz_map_destination_name6, 75
    .long _gz_map_destination_name7, 76
    .long _gz_map_destination_name8, 86
    .long _gz_map_destination_name9, 87
    .long _gz_map_destination_name10, 88
    .long _gz_map_destination_name11, 38
    .long _gz_map_destination_name12, 39
    .long _gz_mah_label3, 0x80000001
    .long _gz_map_destination_name13, 126
    .long _gz_map_destination_name14, 127
    .long _gz_map_destination_name15, 128
    .long _gz_map_destination_name16, 129
    .long _gz_map_destination_name17, 130
    .long _gz_map_destination_name18, 131
    .long _gz_map_destination_name19, 132
    .long _gz_map_destination_name20, 133
    .long _gz_map_destination_name21, 134
    .long _gz_map_destination_name22, 135
    .long _gz_tw_label164, 216
    .long _gz_map_destination_name23, 14
    .long _gz_map_destination_name24, 15
    .long _gz_map_destination_name25, 68
    .long _gz_map_destination_name26, 61
    .long _gz_map_destination_name27, 62
    .long _gz_map_destination_name28, 37
    .long _gz_map_destination_name29, 73
    .long _gz_map_destination_name30, 84
    .long _gz_map_destination_name31, 85
    .long _gz_mah_label6, 0x80000002
    .long _gz_mah_label7, 0x80000003
    .long _gz_tw_label106, 187
    .long _gz_tw_label108, 188
    .long _gz_tw_label110, 189
    .long _gz_tw_label112, 190
    .long _gz_tw_label114, 191
    .long _gz_tw_label116, 192
    .long _gz_tw_label118, 193
    .long _gz_tw_label120, 194
    .long _gz_map_destination_name32, 119
    .long _gz_tw_label90, 179
    .long _gz_map_destination_name33, 63
    .long _gz_map_destination_name34, 64
    .long _gz_tw_label88, 178
    .long _gz_map_destination_name35, 19
    .long _gz_map_destination_name36, 20
    .long _gz_map_destination_name38, 21
    .long _gz_tw_label6, 137
    .long _gz_tw_label8, 138
    .long _gz_tw_label132, 200
    .long _gz_mah_label9, 0x80000004
    .long _gz_map_destination_name39, 98
    .long _gz_map_destination_name40, 99
    .long _gz_map_destination_name41, 100
    .long _gz_tw_label134, 201
    .long _gz_tw_label136, 202
    .long _gz_tw_label138, 203
    .long _gz_tw_label140, 204
    .long _gz_tw_label142, 205
    .long _gz_tw_label144, 206
    .long _gz_tw_label146, 207
    .long _gz_tw_label148, 208
    .long _gz_tw_label150, 209
    .long _gz_map_destination_name42, 28
    .long _gz_map_destination_name43, 29
    .long _gz_map_destination_name44, 30
    .long _gz_map_destination_name45, 89
    .long _gz_map_destination_name46, 90
    .long _gz_map_destination_name47, 66
    .long _gz_map_destination_name48, 67
    .long _gz_map_destination_name49, 81
    .long _gz_tw_label100, 184
    .long _gz_map_destination_name50, 51
    .long _gz_map_destination_name51, 91
    .long _gz_map_destination_name52, 92
    .long _gz_map_destination_name53, 23
    .long _gz_map_destination_name54, 24
    .long _gz_map_destination_name55, 74
    .long _gz_map_destination_name56, 80
    .long _gz_map_destination_name57, 40
    .long _gz_map_destination_name58, 16
    .long _gz_map_destination_name59, 70
    .long _gz_map_destination_name60, 71
    .long _gz_map_destination_name61, 82
    .long _gz_map_destination_name66, 120
    .long _gz_map_destination_name67, 121
    .long _gz_map_destination_name68, 122
    .long _gz_map_destination_name69, 123
    .long _gz_map_destination_name70, 124
    .long _gz_map_destination_name71, 125
    .long _gz_tw_label102, 185
    .long _gz_tw_label104, 186
    .long _gz_tw_label154, 211
    .long _gz_tw_label156, 212
    .long _gz_mah_label12, 0x80000005
    .long _gz_mah_label13, 0x80000006
    .long _gz_map_destination_name65, 118
    .long _gz_tw_label64, 166
    .long _gz_tw_label66, 167
    .long _gz_map_destination_name62, 114
    .long _gz_map_destination_name63, 115
    .long _gz_map_destination_name64, 116
    .long _gz_tw_label38, 153
    .long _gz_tw_label40, 154
    .long _gz_tw_label42, 155
    .long _gz_tw_label44, 156
    .long _gz_tw_label46, 157
    .long _gz_tw_label48, 158
    .long _gz_tw_label50, 159
    .long _gz_tw_label52, 160
    .long _gz_tw_label54, 161
    .long _gz_tw_label56, 162
    .long _gz_tw_label58, 163
    .long _gz_tw_label60, 164
    .long _gz_tw_label62, 165
    .long _gz_tw_label168, 218
    .long _gz_tw_label170, 219
    .long _gz_tw_label172, 220
    .long _gz_map_destination_name72, 22
    .long _gz_map_destination_name73, 93
    .long _gz_map_destination_name74, 94
    .long _gz_map_destination_name75, 49
    .long _gz_map_destination_name76, 50
    .long _gz_map_destination_name77, 47
    .long _gz_map_destination_name78, 48
    .long _gz_map_destination_name79, 77
    .long _gz_tw_label1, 78
    .long _gz_tw_label98, 183
    .long _gz_map_destination_name82, 72
    .long _gz_map_destination_name83, 65
    .long _gz_tw_label70, 169
    .long _gz_tw_label72, 170
    .long _gz_map_destination_name84, 7
    .long _gz_map_destination_name85, 8
    .long _gz_map_destination_name86, 9
    .long _gz_map_destination_name87, 10
    .long _gz_map_destination_name88, 3
    .long _gz_map_destination_name89, 4
    .long _gz_map_destination_name90, 17
    .long _gz_map_destination_name91, 18
    .long _gz_map_destination_name92, 25
    .long _gz_map_destination_name93, 69
    .long _gz_tw_label96, 182
    .long _gz_map_destination_name94, 95
    .long _gz_map_destination_name95, 96
    .long _gz_map_destination_name96, 45
    .long _gz_map_destination_name97, 46
    .long _gz_tw_label68, 168
    .long _gz_map_destination_name98, 31
    .long _gz_map_destination_name99, 32
    .long _gz_map_destination_name100, 33
    .long _gz_map_destination_name101, 34
    .long _gz_tw_label14, 141
    .long _gz_tw_label16, 142
    .long _gz_tw_label152, 210
    .long _gz_mah_label16, 0x80000007
    .long _gz_mah_label17, 0x80000008
    .long _gz_map_destination_name103, 104
    .long _gz_map_destination_name104, 105
    .long _gz_map_destination_name105, 106
    .long _gz_map_destination_name106, 107
    .long _gz_map_destination_name107, 108
    .long _gz_tw_label166, 217
    .long _gz_map_destination_name102, 35
    .long _gz_tw_label24, 146
    .long _gz_tw_label26, 147
    .long _gz_tw_label28, 148
    .long _gz_tw_label30, 149
    .long _gz_tw_label32, 150
    .long _gz_tw_label34, 151
    .long _gz_tw_label36, 152
    .long _gz_map_destination_name108, 11
    .long _gz_map_destination_name109, 12
    .long _gz_map_destination_name110, 13
    .long _gz_map_destination_name111, 43
    .long _gz_map_destination_name112, 44
    .long _gz_map_destination_name113, 52
    .long _gz_map_destination_name114, 53
    .long _gz_map_destination_name115, 54
    .long _gz_map_destination_name123, 97
    .long _gz_tw_label78, 173
    .long _gz_tw_label122, 195
    .long _gz_mah_label20, 0x80000009
    .long _gz_mah_label21, 0x8000000a
    .long _gz_tw_label74, 171
    .long _gz_tw_label76, 172
    .long _gz_tw_label80, 174
    .long _gz_tw_label82, 175
    .long _gz_tw_label84, 176
    .long _gz_tw_label86, 177
    .long _gz_map_destination_name116, 55
    .long _gz_map_destination_name117, 56
    .long _gz_map_destination_name118, 57
    .long _gz_map_destination_name119, 58
    .long _gz_map_destination_name120, 59
    .long _gz_map_destination_name121, 60
    .long _gz_tw_label92, 180
    .long _gz_tw_label94, 181
    .long _gz_map_destination_name124, 41
    .long _gz_map_destination_name125, 42
    .long _gz_mah_label23, 0x8000000b
    .long _gz_map_destination_name126, 101
    .long _gz_map_destination_name127, 102
    .long _gz_map_destination_name128, 103
    .long _gz_tw_label10, 139
    .long _gz_tw_label12, 140
    .long _gz_tw_label160, 214
    .long _gz_tw_label162, 215
    .long _gz_map_destination_name129, 83
    .long _gz_map_destination_name130, 0
    .long _gz_tw_label2, 2
    .long _gz_map_destination_name131, 5
    .long _gz_map_destination_name132, 6
    .long _gz_tw_label4, 136
    .long _gz_map_destination_name133, 26
    .long _gz_map_destination_name134, 27
.balign 4
.global _gz_mah_label0
_gz_mah_label0:
    .string "FF1"
.balign 4
.global _gz_mah_label1
_gz_mah_label1:
    .string "FF1 >"
.balign 4
.global _gz_mah_label2
_gz_mah_label2:
    .string "Wind Temple"
.balign 4
.global _gz_mah_label3
_gz_mah_label3:
    .string "Wind Temple >"
.balign 4
.global _gz_mah_label4
_gz_mah_label4:
    .string "Buildings"
.balign 4
.global _gz_mah_label5
_gz_mah_label5:
    .string "Pirate Ship"
.balign 4
.global _gz_mah_label6
_gz_mah_label6:
    .string "Buildings >"
.balign 4
.global _gz_mah_label7
_gz_mah_label7:
    .string "Pirate Ship >"
.balign 4
.global _gz_mah_label8
_gz_mah_label8:
    .string "Dragon Roost Cavern"
.balign 4
.global _gz_mah_label9
_gz_mah_label9:
    .string "Dragon Roost Cavern >"
.balign 4
.global _gz_mah_label10
_gz_mah_label10:
    .string "Hyrule"
.balign 4
.global _gz_mah_label11
_gz_mah_label11:
    .string "Ganon's Tower"
.balign 4
.global _gz_mah_label12
_gz_mah_label12:
    .string "Hyrule >"
.balign 4
.global _gz_mah_label13
_gz_mah_label13:
    .string "Ganon's Tower >"
.balign 4
.global _gz_mah_label14
_gz_mah_label14:
    .string "Forbidden Woods"
.balign 4
.global _gz_mah_label15
_gz_mah_label15:
    .string "Figurine Gallery"
.balign 4
.global _gz_mah_label16
_gz_mah_label16:
    .string "Forbidden Woods >"
.balign 4
.global _gz_mah_label17
_gz_mah_label17:
    .string "Figurine Gallery >"
.balign 4
.global _gz_mah_label18
_gz_mah_label18:
    .string "Houses"
.balign 4
.global _gz_mah_label19
_gz_mah_label19:
    .string "Savage Labyrinth"
.balign 4
.global _gz_mah_label20
_gz_mah_label20:
    .string "Houses >"
.balign 4
.global _gz_mah_label21
_gz_mah_label21:
    .string "Savage Labyrinth >"
.balign 4
.global _gz_mah_label22
_gz_mah_label22:
    .string "Earth Temple"
.balign 4
.global _gz_mah_label23
_gz_mah_label23:
    .string "Earth Temple >"
.balign 4
.global _gz_mah_label24
_gz_mah_label24:
    .string "FF2"
.balign 4
.global _gz_mah_label25
_gz_mah_label25:
    .string "FF2 >"
.balign 4
.global _gz_mah_label26
_gz_mah_label26:
    .string "FF3"
.balign 4
.global _gz_mah_label27
_gz_mah_label27:
    .string "FF3 >"
.balign 4
.global _gz_mah_label28
_gz_mah_label28:
    .string "Exterior (FF2/FF3)"
