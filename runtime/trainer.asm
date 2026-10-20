.origin = codecave
.origin = codecave
.align 4
_gz_roll_latch_segment_distance2:
lfs f7,0(r3)
lis r9,_gz_roll_latch_LC0@ha
lfs f12,0(r4)
lfs f8,4(r3)
lfs f10,4(r4)
fsubs f12,f12,f7
lfs f0,_gz_roll_latch_LC0@l(r9)
fsubs f10,f10,f8
fmuls f9,f12,f12
fmuls f11,f10,f10
fadds f9,f9,f11
fcmpu cr0,f9,f0
bng- _gz_roll_latch_L11
fsubs f11,f1,f7
fsubs f6,f2,f8
fmuls f11,f11,f12
fmuls f6,f6,f10
fadds f11,f11,f6
fdivs f11,f11,f9
fcmpu cr0,f11,f0
blt- _gz_roll_latch_L4
lis r9,_gz_roll_latch_LC1@ha
lfs f0,_gz_roll_latch_LC1@l(r9)
fcmpu cr0,f11,f0
mfcr r9
rlwinm r9,r9,2,1
_gz_roll_latch_L2:
andi. r9,r9,0x1
fmr f0,f11
beq- _gz_roll_latch_L4
lis r9,_gz_roll_latch_LC1@ha
lfs f0,_gz_roll_latch_LC1@l(r9)
_gz_roll_latch_L4:
fmuls f12,f0,f12
fmuls f0,f0,f10
fadds f12,f12,f7
fadds f0,f0,f8
fsubs f1,f1,f12
fsubs f2,f2,f0
fmuls f1,f1,f1
fmuls f2,f2,f2
fadds f1,f1,f2
blr
_gz_roll_latch_L11:
fmr f11,f0
li r9,0
b _gz_roll_latch_L2
.align 4
_gz_roll_volume_triangle:
addis r3,r4,0xf000
lis r10,_gz_roll_volume_trace@ha
lis r0,0xafff
addi r10,r10,_gz_roll_volume_trace@l
cmplw r3,r0
lwz r3,12(r10)
addi r3,r3,1
stw r3,12(r10)
.int 0x4c800020

or r3,r8,r4
mr r12,r8
andi. r11,r3,0x3
.int 0x4ca20020

addis r3,r8,0xf000
cmplw r3,r0
.int 0x4c800020

lis r3,0x3
ori r3,r3,0xd40
cmplw r5,r3
.int 0x4d810020

cmplw r6,r3
.int 0x4d810020

cmplw r7,r3
.int 0x4d810020

stwu r1,-184(r1)
lis r3,0x7f80
mflr r0
mtlr r3
stw r31,148(r1)
li r3,4
stw r0,188(r1)
mtctr r3
mr r0,r8
_gz_roll_latch_L15:
mr r3,r0
mr r31,r0
lwz r3,0(r3)
addi r0,r31,4
mflr r31
rlwinm r3,r3,0,1,8
cmpw r3,r31
beq- _gz_roll_latch_L13
bdnz _gz_roll_latch_L15
lis r3,_gz_roll_latch_LC4@ha
lfs f0,4(r8)
lfs f12,_gz_roll_latch_LC4@l(r3)
fcmpu cr0,f0,f12
cror 2,1,2
beq- _gz_roll_latch_L13
lis r3,_gz_roll_latch_LC5@ha
lfs f12,_gz_roll_latch_LC5@l(r3)
fcmpu cr0,f0,f12
blt- _gz_roll_latch_L13
lfs f10,0(r8)
lis r3,_gz_roll_latch_LC6@ha
lfs f9,8(r8)
fmuls f12,f10,f10
lfs f11,_gz_roll_latch_LC6@l(r3)
fmuls f8,f9,f9
fadds f12,f12,f8
fcmpu cr0,f12,f11
blt- _gz_roll_latch_L13
fmuls f11,f0,f0
lis r3,_gz_roll_latch_LC7@ha
lfs f8,_gz_roll_latch_LC7@l(r3)
fadds f11,f11,f12
fcmpu cr0,f11,f8
blt- _gz_roll_latch_L13
lis r3,_gz_roll_latch_LC8@ha
lfs f8,_gz_roll_latch_LC8@l(r3)
fcmpu cr0,f11,f8
bgt- _gz_roll_latch_L13
lis r3,_gz_roll_latch_LC9@ha
lfs f11,12(r9)
lfs f7,_gz_roll_latch_LC9@l(r3)
lis r3,_gz_roll_latch_LC0@ha
lfs f8,_gz_roll_latch_LC0@l(r3)
fmuls f11,f11,f7
fcmpu cr0,f11,f8
cror 2,0,2
beq- _gz_roll_latch_L13
lfs f8,0(r9)
fmuls f6,f11,f11
lfs f7,4(r9)
fmuls f10,f10,f8
lfs f8,8(r9)
lfs f11,12(r8)
fmuls f0,f0,f7
stfd f28,152(r1)
stfd f29,160(r1)
fmuls f9,f9,f8
fadds f0,f10,f0
fmuls f12,f6,f12
fadds f0,f0,f9
fmr f29,f6
fadds f0,f0,f11
fmr f28,f0
fmuls f0,f0,f0
fcmpu cr0,f0,f12
cror 2,1,2
beq- _gz_roll_latch_L52
addi r3,r1,56
addi r8,r1,92
stw r28,136(r1)
mtctr r8
stw r5,92(r1)
lis r0,0xafff
stw r6,96(r1)
mr r6,r3
stw r7,100(r1)
li r7,0
_gz_roll_latch_L16:
li r5,0
slwi r8,r7,2
mr r28,r5
mfctr r5
lwzx r8,r5,r8
mulli r8,r8,12
add r8,r8,r4
_gz_roll_latch_L17:
addis r5,r8,0xf000
lis r31,0x7f80
cmplw r5,r0
bge- _gz_roll_latch_L54
lwz r5,0(r8)
rlwinm r5,r5,0,1,8
cmpw r5,r31
beq- _gz_roll_latch_L54
addi r5,r28,4
lfs f0,0(r8)
cmplwi r5,12
addi r8,r8,4
stfsx f0,r6,r28
mr r28,r5
bne- _gz_roll_latch_L17
addi r7,r7,1
addi r6,r6,12
cmplwi r7,3
bne- _gz_roll_latch_L16
lis r8,0xaaaa
lis r7,_gz_roll_latch_LC0@ha
ori r6,r8,0xaaab
li r8,0
stw r29,140(r1)
addi r4,r1,8
lfs f9,_gz_roll_latch_LC0@l(r7)
mr r29,r8
li r7,0
_gz_roll_latch_L18:
lfs f11,4(r9)
slwi r8,r29,3
lfs f0,4(r3)
add r5,r4,r8
addi r7,r7,1
fsubs f0,f0,f11
fcmpu cr0,f0,f9
bne- _gz_roll_latch_L19
lfs f12,0(r3)
addi r0,r29,1
lfs f0,8(r3)
mr r29,r0
stfsx f12,r4,r8
stfs f0,4(r5)
_gz_roll_latch_L20:
cmplwi r7,3
addi r3,r3,12
bne+ _gz_roll_latch_L18
cmpwi r29,0
beq- _gz_roll_latch_L55
lfs f0,8(r9)
mr r3,r4
lfs f1,0(r9)
fmr f2,f0
stw r27,132(r1)
stfd f30,168(r1)
fmr f30,f0
stfd f31,176(r1)
fmr f31,f1
stw r9,104(r1)
stw r11,108(r1)
stw r12,112(r1)
stw r30,144(r1)
bl _gz_roll_latch_segment_distance2
fmr f0,f1
lis r9,_gz_roll_volume_trace@ha
addi r4,r1,8
addi r10,r9,_gz_roll_volume_trace@l
lwz r12,112(r1)
mr r27,r4
lwz r11,108(r1)
lwz r9,104(r1)
_gz_roll_latch_L30:
addi r11,r11,1
cmplw r11,r29
beq- _gz_roll_latch_L26
addi r8,r27,8
mr r30,r27
mr r4,r8
mr r27,r8
subf r8,r11,r29
mr r31,r8
_gz_roll_latch_L29:
fmr f2,f30
mr r3,r30
fmr f1,f31
stw r9,120(r1)
stw r12,116(r1)
stfs f0,112(r1)
stw r11,108(r1)
stw r4,104(r1)
bl _gz_roll_latch_segment_distance2
lfs f0,112(r1)
addic. r9,r31,-1
lwz r4,104(r1)
lis r10,_gz_roll_volume_trace@ha
fcmpu cr7,f1,f0
mr r31,r9
lwz r11,108(r1)
addi r4,r4,8
lwz r12,116(r1)
addi r10,r10,_gz_roll_volume_trace@l
lwz r9,120(r1)
bnl- cr7,_gz_roll_latch_L28
fmr f0,f1
_gz_roll_latch_L28:
bne+ _gz_roll_latch_L29
b _gz_roll_latch_L30
_gz_roll_latch_L54:
lfd f28,152(r1)
lfd f29,160(r1)
lwz r28,136(r1)
_gz_roll_latch_L13:
lwz r0,188(r1)
lwz r31,148(r1)
addi r1,r1,184
mtlr r0
blr
_gz_roll_latch_L52:
lfd f28,152(r1)
lfd f29,160(r1)
b _gz_roll_latch_L13
_gz_roll_latch_L19:
mulhwu r8,r7,r6
slwi r5,r29,3
add r0,r4,r5
mtctr r0
srwi r8,r8,1
mulli r8,r8,3
subf r8,r8,r7
mulli r8,r8,12
add r0,r4,r8
mr r8,r0
lfs f12,52(r8)
fsubs f12,f12,f11
bnl- _gz_roll_latch_L45
fcmpu cr0,f12,f9
bng- _gz_roll_latch_L20
_gz_roll_latch_L23:
fsubs f10,f0,f12
addi r8,r29,1
mr r29,r8
mr r8,r0
lfs f7,0(r3)
fdivs f0,f0,f10
lfs f11,48(r8)
lfs f8,8(r3)
lfs f12,56(r8)
mfctr r8
fsubs f12,f12,f8
fsubs f11,f11,f7
fmuls f11,f11,f0
fmuls f0,f12,f0
fadds f12,f11,f7
fadds f0,f0,f8
stfsx f12,r4,r5
stfs f0,4(r8)
b _gz_roll_latch_L20
_gz_roll_latch_L45:
bng- _gz_roll_latch_L20
fcmpu cr0,f12,f9
bnl+ _gz_roll_latch_L20
b _gz_roll_latch_L23
_gz_roll_latch_L26:
fcmpu cr0,f29,f0
cror 2,0,2
beq- _gz_roll_latch_L56
li r8,1
li r7,4
stw r8,16(r9)
mtctr r7
stw r8,16(r10)
_gz_roll_latch_L31:
lwz r8,0(r12)
slwi r9,r28,2
addi r12,r12,4
stwx r8,r10,r9
addi r9,r28,1
mr r28,r9
bdnz _gz_roll_latch_L31
stfs f28,64(r10)
stfs f0,68(r10)
_gz_roll_latch_L56:
lfd f28,152(r1)
lfd f29,160(r1)
lfd f30,168(r1)
lfd f31,176(r1)
lwz r27,132(r1)
lwz r28,136(r1)
lwz r29,140(r1)
lwz r30,144(r1)
b _gz_roll_latch_L13
_gz_roll_latch_L55:
lfd f28,152(r1)
lfd f29,160(r1)
lwz r28,136(r1)
lwz r29,140(r1)
b _gz_roll_latch_L13
.align 4
_gz_roll_latch_probe:
lwz r9,26096(r3)
mr r6,r3
stwu r1,-232(r1)
addi r9,r9,-43
cmplwi r9,5
ble- _gz_roll_latch_L179
lwz r10,2512(r3)
addi r9,r3,2600
cmplw r10,r9
beq- _gz_roll_latch_L71
_gz_roll_latch_L72:
li r3,0
_gz_roll_latch_L57:
addi r1,r1,232
blr
_gz_roll_latch_L71:
lwz r10,2516(r3)
addi r9,r3,2612
cmplw r10,r9
bne- _gz_roll_latch_L72
lwz r10,2512(r3)
addi r9,r1,4
lwz r8,2516(r3)
li r7,5
stwu r10,4(r9)
addi r11,r3,26096
addi r10,r3,2520
mtctr r7
stwu r8,4(r9)
_gz_roll_latch_L73:
lwz r3,0(r10)
addi r10,r10,20
lwz r4,-16(r10)
lwz r5,-12(r10)
lwz r7,-8(r10)
lwz r8,-4(r10)
stw r3,4(r9)
stw r4,8(r9)
stw r5,12(r9)
stw r7,16(r9)
stwu r8,20(r9)
bdnz _gz_roll_latch_L73
addi r9,r1,8
addi r10,r1,96
addi r9,r9,100
stw r10,8(r1)
stw r9,12(r1)
addi r8,r1,24
li r9,16
lis r5,0xafff
_gz_roll_latch_L77:
addi r10,r9,-16
cmplwi cr7,r9,88
rlwinm. r10,r10,0,28,26
cmplwi cr5,r9,100
beq- _gz_roll_latch_L74
_gz_roll_latch_L180:
beq- cr7,_gz_roll_latch_L74
beq- cr5,_gz_roll_latch_L74
addi r9,r9,4
addi r8,r8,4
addi r10,r9,-16
cmplwi cr7,r9,88
rlwinm. r10,r10,0,28,26
cmplwi cr5,r9,100
bne+ _gz_roll_latch_L180
_gz_roll_latch_L74:
lwz r10,0(r8)
addis r7,r10,0xf000
andi. r10,r10,0x3
cmplw cr7,r7,r5
bge- cr7,_gz_roll_latch_L72
addi r9,r9,4
bne- _gz_roll_latch_L72
cmplwi r9,104
addi r8,r8,4
bne+ _gz_roll_latch_L77
li r9,1
stb r10,92(r1)
stb r10,90(r1)
addi r4,r6,788
stb r10,89(r1)
stb r10,88(r1)
stb r9,91(r1)
lis r9,_gz_roll_latch_LC19@ha
lfs f0,_gz_roll_latch_LC19@l(r9)
lwz r9,26096(r6)
lfs f11,788(r6)
addi r9,r9,-43
lfs f12,792(r6)
cmplwi r9,5
lfs f10,796(r6)
fadds f0,f12,f0
stfd f29,208(r1)
stfd f30,216(r1)
fmr f29,f10
stfd f31,224(r1)
fmr f30,f11
fmr f31,f0
stw r10,84(r1)
stfs f11,148(r1)
stfs f10,156(r1)
stfs f0,152(r1)
bgt- _gz_roll_latch_L163
lwz r8,1096(r6)
addis r9,r8,0xf000
cmplw r9,r5
bge- _gz_roll_latch_L176
andi. r9,r8,0x3
bne- _gz_roll_latch_L176
lwz r9,172(r8)
addis r7,r9,0xf000
cmplw r7,r5
bge- _gz_roll_latch_L176
andi. r7,r9,0x3
bne- _gz_roll_latch_L176
lwz r7,1092(r6)
cmplw r9,r7
bne- _gz_roll_latch_L176
lwz r7,4(r9)
cmplwi r7,1
ble- _gz_roll_latch_L176
lwz r9,4(r9)
cmplwi r9,128
bgt- _gz_roll_latch_L176
lwz r9,44(r8)
lis r8,0xafff
addis r7,r9,0xf000
cmplw r7,r8
bge- _gz_roll_latch_L176
andi. r7,r9,0x3
bne- _gz_roll_latch_L176
lwz r9,16(r9)
addis r7,r9,0xf000
cmplw r7,r8
bge- _gz_roll_latch_L176
andi. r8,r9,0x3
bne- _gz_roll_latch_L176
addi r9,r9,60
lis r0,0x7f80
addi r12,r1,148
lis r3,_gz_roll_latch_LC0@ha
lis r5,_gz_roll_latch_LC17@ha
_gz_roll_latch_L83:
lwz r7,0(r9)
rlwinm r7,r7,0,1,8
cmpw r7,r0
beq- _gz_roll_latch_L176
lfs f12,0(r9)
addi r9,r9,16
lfsx f0,r4,r8
lfs f10,_gz_roll_latch_LC0@l(r3)
fsubs f0,f12,f0
stfsx f12,r8,r12
addi r8,r8,4
lfs f11,_gz_roll_latch_LC17@l(r5)
cmplwi r8,12
fcmpu cr7,f0,f10
bnl+ cr7,_gz_roll_latch_L80
fneg f0,f0
_gz_roll_latch_L80:
fcmpu cr7,f0,f11
bgt- cr7,_gz_roll_latch_L176
bne+ _gz_roll_latch_L83
mflr r0
stw r30,200(r1)
stw r0,236(r1)
stw r31,204(r1)
lfs f30,148(r1)
lfs f31,152(r1)
lfs f29,156(r1)
b _gz_roll_latch_L78
_gz_roll_latch_L179:
lis r11,_gz_roll_volume_trace@ha
li r10,6
li r9,0
addi r8,r11,_gz_roll_volume_trace@l
li r7,0
mtctr r10
_gz_roll_latch_L59:
addi r10,r9,1
addi r5,r9,3
addi r4,r9,2
slwi r3,r9,2
slwi r10,r10,2
slwi r4,r4,2
slwi r5,r5,2
stwx r7,r8,r3
addi r9,r9,4
stwx r7,r8,r10
stwx r7,r8,r4
stwx r7,r8,r5
bdnz _gz_roll_latch_L59
lis r10,_gz_game_counter@ha
addi r9,r6,788
lwz r10,_gz_game_counter@l(r10)
stw r10,_gz_roll_volume_trace@l(r11)
lwz r10,26096(r6)
stw r10,4(r8)
lwz r10,2104(r6)
cmplw r10,r9
bne- _gz_roll_latch_L72
lwz r9,2192(r6)
cmplwi r9,3
bne- _gz_roll_latch_L72
lwz r7,2196(r6)
addi r9,r6,1868
cmplw r7,r9
bne- _gz_roll_latch_L72
lwz r9,2512(r6)
addi r7,r6,2600
cmplw r9,r7
bne- _gz_roll_latch_L72
lwz r7,2516(r6)
addi r5,r6,2612
cmplw r7,r5
bne- _gz_roll_latch_L72
lwz r4,0(r10)
lis r5,0x7f80
rlwinm r4,r4,0,1,8
cmpw r4,r5
beq- _gz_roll_latch_L72
lwz r4,792(r6)
addi r3,r6,792
stw r31,204(r1)
mr r31,r3
rlwinm r4,r4,0,1,8
cmpw r4,r5
beq- _gz_roll_latch_L177
lwz r4,796(r6)
addi r3,r6,796
stw r30,200(r1)
mr r30,r3
rlwinm r4,r4,0,1,8
cmpw r4,r5
beq- _gz_roll_latch_L162
mflr r0
addi r3,r1,8
stw r0,236(r1)
stw r29,196(r1)
stw r6,168(r1)
stw r10,172(r1)
stw r7,176(r1)
stw r9,180(r1)
bl _gz_roll_sphere_construct
lwz r6,168(r1)
lis r10,_gz_roll_latch_LC0@ha
lwz r9,180(r1)
addi r4,r1,64
lwz r7,176(r1)
addi r5,r1,76
lwz r12,0(r9)
lis r9,_gz_roll_volume_triangle@ha
lwz r3,2604(r6)
addi r9,r9,_gz_roll_volume_triangle@l
lwz r11,2608(r6)
lis r0,0x7f80
stw r3,68(r1)
lis r3,_gz_roll_bg@ha
stw r11,72(r1)
addi r11,r6,1916
stw r12,64(r1)
lfs f10,_gz_roll_latch_LC0@l(r10)
addi r10,r3,_gz_roll_bg@l
lwz r12,0(r7)
mr r29,r10
lwz r3,2616(r6)
fmr f8,f10
lwz r7,4(r6)
lwz r10,172(r1)
stw r9,84(r1)
lis r9,_gz_roll_volume_trace@ha
stw r3,80(r1)
addi r8,r9,_gz_roll_volume_trace@l
stw r4,44(r1)
stw r5,48(r1)
stw r7,52(r1)
stw r12,76(r1)
li r12,0
_gz_roll_latch_L69:
lwz r9,0(r11)
rlwinm r9,r9,0,1,8
cmpw r9,r0
beq- _gz_roll_latch_L108
lwz r9,4(r11)
rlwinm r9,r9,0,1,8
cmpw r9,r0
beq- _gz_roll_latch_L108
lis r9,_gz_roll_latch_LC0@ha
lfs f0,0(r11)
lfs f9,_gz_roll_latch_LC0@l(r9)
fneg f11,f0
lfs f12,4(r11)
fcmpu cr0,f0,f9
blt- _gz_roll_latch_L64
fmr f11,f0
_gz_roll_latch_L64:
lis r9,_gz_roll_latch_LC17@ha
lfs f9,_gz_roll_latch_LC17@l(r9)
fcmpu cr0,f11,f9
bgt- _gz_roll_latch_L108
lis r9,_gz_roll_latch_LC0@ha
lfs f11,_gz_roll_latch_LC0@l(r9)
fcmpu cr0,f12,f11
cror 2,0,2
beq- _gz_roll_latch_L108
lis r9,_gz_roll_latch_LC18@ha
lfs f11,_gz_roll_latch_LC18@l(r9)
fcmpu cr0,f12,f11
bgt- _gz_roll_latch_L108
cmpwi r12,0
beq- _gz_roll_latch_L66
fcmpu cr0,f0,f8
bne- _gz_roll_latch_L66
fcmpu cr0,f10,f12
bne- _gz_roll_latch_L66
_gz_roll_latch_L67:
addi r12,r12,1
addi r11,r11,64
cmplwi r12,3
bne+ _gz_roll_latch_L69
lwz r0,236(r1)
lwz r29,196(r1)
lwz r30,200(r1)
mtlr r0
lwz r31,204(r1)
b _gz_roll_latch_L72
_gz_roll_latch_L176:
lfd f29,208(r1)
lfd f30,216(r1)
lfd f31,224(r1)
b _gz_roll_latch_L72
_gz_roll_latch_L66:
lfs f9,0(r10)
li r9,4
lfs f11,0(r31)
mtctr r9
lfs f10,0(r30)
li r9,0
fadds f11,f0,f11
stfs f9,8(r1)
stw r9,132(r1)
addi r6,r1,116
stfs f9,116(r1)
li r7,5
stfs f11,12(r1)
stfs f10,16(r1)
stfs f12,20(r1)
stfs f12,128(r1)
lwz r9,8(r8)
stfs f11,120(r1)
addi r9,r9,1
stfs f10,124(r1)
stw r9,8(r8)
_gz_roll_latch_L68:
lwz r5,0(r6)
slwi r9,r7,2
addi r6,r6,4
addi r7,r7,1
stwx r5,r8,r9
bdnz _gz_roll_latch_L68
addi r5,r1,116
addi r4,r1,8
mr r3,r29
stw r11,184(r1)
stw r12,180(r1)
stfs f12,176(r1)
stfs f0,172(r1)
stw r10,168(r1)
bl _gz_roll_sphere_cross
lwz r9,132(r1)
lis r0,0x7f80
lwz r10,168(r1)
cmpwi r9,0
lis r9,_gz_roll_volume_trace@ha
lfs f0,172(r1)
addi r8,r9,_gz_roll_volume_trace@l
lfs f12,176(r1)
lwz r12,180(r1)
lwz r11,184(r1)
bne- _gz_roll_latch_L181
fmr f10,f12
fmr f8,f0
b _gz_roll_latch_L67
_gz_roll_latch_L163:
mflr r0
stw r30,200(r1)
stw r0,236(r1)
stw r31,204(r1)
_gz_roll_latch_L78:
lis r8,_gz_roll_bg@ha
lis r9,_gz_roll_latch_LC20@ha
addi r8,r8,_gz_roll_bg@l
addi r9,r9,_gz_roll_latch_LC20@l
mr r31,r8
mr r30,r9
_gz_roll_latch_L86:
subfic r9,r10,1
andi. r9,r10,0x1
subfe r9,r9,r9
stfs f30,116(r1)
neg r9,r9
stfs f30,136(r1)
stfs f31,120(r1)
slwi r9,r9,3
stfs f31,140(r1)
addi r5,r1,116
stfs f29,124(r1)
addi r4,r1,136
stfs f29,144(r1)
stw r10,176(r1)
addi r10,r1,8
add r9,r10,r9
stw r11,172(r1)
lfs f11,0(r30)
stw r6,168(r1)
lfs f12,108(r9)
lfs f0,128(r9)
beq- _gz_roll_latch_L84
fadds f12,f12,f11
mr r3,r10
fsubs f0,f0,f11
stfs f12,108(r9)
stfs f0,128(r9)
bl _gz_roll_line_set
addi r4,r1,8
mr r3,r31
bl _gz_roll_line_cross
lwz r6,168(r1)
cmpwi r3,0
lwz r11,172(r1)
lwz r10,176(r1)
bne- _gz_roll_latch_L85
_gz_roll_latch_L89:
addi r10,r10,1
cmplwi r10,4
bne+ _gz_roll_latch_L86
lwz r0,236(r1)
lfd f29,208(r1)
lfd f30,216(r1)
mtlr r0
lfd f31,224(r1)
lwz r30,200(r1)
lwz r31,204(r1)
b _gz_roll_latch_L72
_gz_roll_latch_L84:
fsubs f12,f12,f11
mr r3,r10
fadds f0,f0,f11
stfs f12,108(r9)
stfs f0,128(r9)
bl _gz_roll_line_set
addi r4,r1,8
mr r3,r31
bl _gz_roll_line_cross
lwz r6,168(r1)
cmpwi r3,0
lwz r11,172(r1)
lwz r10,176(r1)
bne- _gz_roll_latch_L182
_gz_roll_latch_L155:
addi r10,r10,1
b _gz_roll_latch_L86
_gz_roll_latch_L182:
addi r4,r1,28
mr r3,r31
stw r10,176(r1)
stw r11,172(r1)
stw r6,168(r1)
bl _gz_roll_poly_safe
cmpwi r3,0
lwz r6,168(r1)
lwz r11,172(r1)
lwz r10,176(r1)
beq- _gz_roll_latch_L155
_gz_roll_latch_L87:
lwz r9,32(r1)
lis r7,0xafff
addis r8,r9,0xf000
cmplw r8,r7
bge- _gz_roll_latch_L89
andi. r8,r9,0x3
bne- _gz_roll_latch_L89
lwz r8,148(r9)
addis r9,r8,0xf000
cmplw r9,r7
bge- _gz_roll_latch_L89
andi. r9,r8,0x3
bne- _gz_roll_latch_L89
lhz r9,28(r1)
lwz r8,8(r8)
rlwinm r9,r9,0,0xffff
cmplw r9,r8
bge- _gz_roll_latch_L89
lhz r4,30(r1)
mr r3,r31
lhz r5,28(r1)
rlwinm r4,r4,0,0xffff
stw r10,176(r1)
stw r11,172(r1)
stw r6,168(r1)
bl _gz_roll_tri_plane
addis r5,r3,0xf000
lis r7,0xafff
cmplw r5,r7
lwz r6,168(r1)
lwz r11,172(r1)
mr r8,r3
lwz r10,176(r1)
bge- _gz_roll_latch_L89
andi. r7,r3,0x3
bne- _gz_roll_latch_L89
li r9,4
lis r5,0x7f80
mtctr r9
_gz_roll_latch_L90:
lwz r7,0(r8)
addi r8,r8,4
rlwinm r7,r7,0,1,8
cmpw r7,r5
beq- _gz_roll_latch_L89
bdnz _gz_roll_latch_L90
lis r8,_gz_roll_latch_LC0@ha
lfs f11,4(r3)
lfs f12,_gz_roll_latch_LC0@l(r8)
fneg f0,f11
fcmpu cr0,f11,f12
blt- _gz_roll_latch_L91
fmr f0,f11
_gz_roll_latch_L91:
lis r8,_gz_roll_latch_LC21@ha
lfs f12,_gz_roll_latch_LC21@l(r8)
fcmpu cr0,f0,f12
bgt- _gz_roll_latch_L89
lfs f9,0(r3)
lis r8,_gz_roll_latch_LC22@ha
lfs f10,8(r3)
fmuls f0,f9,f9
lfs f12,_gz_roll_latch_LC22@l(r8)
fmuls f8,f10,f10
fadds f0,f0,f8
fcmpu cr0,f0,f12
blt- _gz_roll_latch_L89
lis r8,_gz_roll_latch_LC23@ha
lfs f12,_gz_roll_latch_LC23@l(r8)
fcmpu cr0,f0,f12
bgt- _gz_roll_latch_L89
lis r8,_gz_roll_clip_state@ha
lis r9,_gz_roll_latch_LC19@ha
addi r8,r8,_gz_roll_clip_state@l
lfs f8,_gz_roll_latch_LC19@l(r9)
lfs f7,16(r8)
lfs f0,20(r8)
lfs f12,148(r1)
fadds f0,f0,f8
lfs f5,152(r1)
fmuls f12,f9,f12
lfs f8,24(r8)
lfs f6,156(r1)
fmuls f9,f9,f7
lwz r8,0(r11)
lfs f7,12(r3)
fmuls f0,f0,f11
addi r8,r8,-43
cmplwi r8,5
fmuls f11,f11,f5
fmuls f8,f10,f8
fmuls f10,f10,f6
fadds f0,f0,f9
fadds f12,f12,f11
fadds f0,f0,f8
fadds f12,f12,f10
fadds f0,f0,f7
fadds f12,f12,f7
ble- _gz_roll_latch_L183
lis r9,_gz_roll_latch_LC4@ha
lfs f11,_gz_roll_latch_LC4@l(r9)
fcmpu cr0,f0,f11
bng- _gz_roll_latch_L89
lis r9,_gz_roll_latch_LC25@ha
lfs f11,_gz_roll_latch_LC25@l(r9)
fcmpu cr0,f0,f11
bnl- _gz_roll_latch_L89
lis r9,_gz_roll_latch_LC26@ha
lfs f0,_gz_roll_latch_LC26@l(r9)
fcmpu cr0,f12,f0
bnl- _gz_roll_latch_L89
lis r9,_gz_roll_latch_LC24@ha
lfs f0,_gz_roll_latch_LC24@l(r9)
fcmpu cr0,f12,f0
cror 2,0,3
beq- _gz_roll_latch_L89
_gz_roll_latch_L111:
lwz r0,236(r1)
li r3,1
lfd f29,208(r1)
lfd f30,216(r1)
mtlr r0
lfd f31,224(r1)
lwz r30,200(r1)
lwz r31,204(r1)
b _gz_roll_latch_L57
_gz_roll_latch_L85:
addi r4,r1,28
mr r3,r31
stw r10,176(r1)
stw r11,172(r1)
stw r6,168(r1)
bl _gz_roll_poly_safe
cmpwi r3,0
lwz r6,168(r1)
lwz r11,172(r1)
lwz r10,176(r1)
bne+ _gz_roll_latch_L87
b _gz_roll_latch_L89
_gz_roll_latch_L108:
li r3,0
_gz_roll_latch_L178:
lwz r0,236(r1)
lwz r29,196(r1)
lwz r30,200(r1)
mtlr r0
lwz r31,204(r1)
b _gz_roll_latch_L57
_gz_roll_latch_L162:
lwz r30,200(r1)
_gz_roll_latch_L177:
lwz r31,204(r1)
b _gz_roll_latch_L72
_gz_roll_latch_L181:
li r3,1
b _gz_roll_latch_L178
_gz_roll_latch_L183:
lis r9,_gz_roll_latch_LC24@ha
lfs f11,_gz_roll_latch_LC24@l(r9)
fcmpu cr0,f0,f11
cror 2,0,3
beq- _gz_roll_latch_L89
lis r9,_gz_roll_latch_LC25@ha
lfs f10,_gz_roll_latch_LC25@l(r9)
fcmpu cr0,f0,f10
bnl- _gz_roll_latch_L89
lis r9,_gz_roll_latch_LC26@ha
lfs f0,_gz_roll_latch_LC26@l(r9)
fcmpu cr0,f12,f0
bnl- _gz_roll_latch_L89
fcmpu cr0,f12,f11
cror 2,1,2
beq- _gz_roll_latch_L111
b _gz_roll_latch_L89
.align 4
_gz_roll_feedback_expire:
lis r9,_gz_roll_feedback@ha
lwz r8,_gz_roll_feedback@l(r9)
addi r10,r9,_gz_roll_feedback@l
cmpwi r8,0
bne- _gz_roll_latch_L185
lwz r8,8(r10)
cmpwi r8,0
.int 0x4d820020

_gz_roll_latch_L185:
lis r8,_gz_game_counter@ha
lis r7,_gz_roll_result_tick@ha
lwz r6,_gz_game_counter@l(r8)
lwz r8,_gz_roll_result_tick@l(r7)
subf r8,r8,r6
cmplwi r8,89
.int 0x4c810020

lwz r6,8(r10)
lis r8,_gz_roll_state@ha
addi r5,r8,_gz_roll_state@l
cmplwi r6,2
beq- _gz_roll_latch_L197
_gz_roll_latch_L187:
lwz r6,8(r10)
li r8,0
stw r8,12(r10)
cmplwi r6,2
stw r8,8(r10)
stw r8,_gz_roll_feedback@l(r9)
stw r8,4(r10)
stw r8,_gz_roll_result_tick@l(r7)
beq- _gz_roll_latch_L188
li r6,1
_gz_roll_latch_L188:
stw r6,108(r5)
blr
_gz_roll_latch_L197:
lwz r8,_gz_roll_state@l(r8)
cmpwi r8,0
beq- _gz_roll_latch_L187
lwz r8,100(r5)
cmpwi r8,0
bne- _gz_roll_latch_L187
lwz r6,56(r5)
cmplwi r6,42
ble- _gz_roll_latch_L187
lwz r6,56(r5)
cmplwi r6,48
bgt+ _gz_roll_latch_L187
stw r8,12(r10)
stw r8,_gz_roll_feedback@l(r9)
stw r8,4(r10)
blr
.align 4
_gz_roll_cancel:
lis r6,_gz_roll_state@ha
lis r7,_gz_roll_clip_state@ha
li r9,0
addi r10,r6,_gz_roll_state@l
addi r8,r7,_gz_roll_clip_state@l
stw r9,100(r10)
stw r9,56(r10)
stw r9,36(r10)
stw r9,20(r10)
stw r9,12(r10)
stw r9,4(r10)
stw r9,_gz_roll_state@l(r6)
stw r9,12(r8)
stw r9,8(r8)
stw r9,4(r8)
stw r9,_gz_roll_clip_state@l(r7)
blr
.align 4
_gz_roll_begin:
stwu r1,-16(r1)
lis r12,_gz_roll_state@ha
addi r7,r12,_gz_roll_state@l
mflr r0
stw r31,12(r1)
lwz r31,32(r7)
lwz r9,_gz_roll_state@l(r12)
stw r0,20(r1)
cmpwi r9,0
beq- _gz_roll_latch_L204
lwz r9,4(r7)
cmplw r9,r3
beq- _gz_roll_latch_L218
_gz_roll_latch_L204:
li r9,0
lis r0,_gz_roll_feedback@ha
mtlr r9
mr r9,r0
addi r11,r9,_gz_roll_feedback@l
_gz_roll_latch_L200:
li r8,7
li r10,0
li r9,0
mtctr r8
_gz_roll_latch_L201:
addi r8,r10,1
addi r6,r10,3
addi r5,r10,2
slwi r4,r10,2
slwi r8,r8,2
slwi r5,r5,2
slwi r6,r6,2
stwx r9,r7,r4
addi r10,r10,4
stwx r9,r7,r8
stwx r9,r7,r5
stwx r9,r7,r6
bdnz _gz_roll_latch_L201
li r10,1
lis r5,_gz_game_counter@ha
stw r10,_gz_roll_state@l(r12)
mflr r10
stw r3,4(r7)
cmpwi r10,0
lis r8,_gz_roll_clip_state@ha
lis r6,_gz_roll_result_tick@ha
lwz r4,_gz_game_counter@l(r5)
addi r10,r8,_gz_roll_clip_state@l
stw r4,8(r7)
stw r31,32(r7)
lwz r4,26096(r3)
stw r4,56(r7)
stw r9,12(r10)
stw r9,8(r10)
stw r9,4(r10)
stw r9,_gz_roll_clip_state@l(r8)
mr r8,r0
stw r9,12(r11)
stw r9,8(r11)
stw r9,_gz_roll_feedback@l(r8)
stw r9,4(r11)
stw r9,_gz_roll_result_tick@l(r6)
beq- _gz_roll_latch_L202
li r9,2
stw r9,8(r11)
lwz r9,_gz_game_counter@l(r5)
stw r9,_gz_roll_result_tick@l(r6)
_gz_roll_latch_L202:
lwz r8,788(r3)
li r9,1
lwz r0,20(r1)
stw r8,16(r10)
lwz r31,12(r1)
mtlr r0
lwz r8,792(r3)
stw r8,20(r10)
lwz r8,796(r3)
stw r8,24(r10)
stw r9,12(r10)
addi r1,r1,16
blr
_gz_roll_latch_L218:
lwz r9,100(r7)
lis r0,_gz_roll_feedback@ha
cmpwi r9,0
bne- _gz_roll_latch_L205
mtlr r9
mr r9,r0
addi r11,r9,_gz_roll_feedback@l
lwz r9,8(r11)
cmplwi r9,2
bne+ _gz_roll_latch_L200
lwz r9,26096(r3)
cmplwi r9,48
bne+ _gz_roll_latch_L200
lwz r9,56(r7)
cmplwi r9,42
ble- _gz_roll_latch_L200
lwz r9,56(r7)
subfic r9,r9,47
subfe r9,r9,r9
addi r9,r9,1
mtlr r9
b _gz_roll_latch_L200
_gz_roll_latch_L205:
li r9,0
mtlr r9
mr r9,r0
addi r11,r9,_gz_roll_feedback@l
b _gz_roll_latch_L200
.align 4
_gz_roll_gate_mark:
lwz r9,26096(r3)
cmplwi r9,48
.int 0x4c820020

lis r9,_gz_roll_state@ha
lwz r10,_gz_roll_state@l(r9)
addi r9,r9,_gz_roll_state@l
cmpwi r10,0
beq- _gz_roll_latch_L222
lwz r10,4(r9)
cmplw r3,r10
beq- _gz_roll_latch_L243
_gz_roll_latch_L222:
stwu r1,-16(r1)
mflr r0
stw r3,8(r1)
stw r0,20(r1)
bl _gz_roll_begin
lis r9,_gz_roll_state@ha
lwz r3,8(r1)
addi r9,r9,_gz_roll_state@l
lwz r10,12(r9)
cmpwi r10,0
bne- _gz_roll_latch_L219
li r7,1
lis r8,_gz_game_counter@ha
stw r7,12(r9)
lis r10,_gz_roll_clip_state@ha
addi r10,r10,_gz_roll_clip_state@l
lwz r6,_gz_game_counter@l(r8)
stw r6,16(r9)
lwz r6,12(r10)
cmpwi r6,0
beq- _gz_roll_latch_L244
_gz_roll_latch_L225:
lwz r10,20(r9)
cmplwi r10,2
beq- _gz_roll_latch_L245
_gz_roll_latch_L226:
li r10,0
stw r10,20(r9)
_gz_roll_latch_L219:
lwz r0,20(r1)
addi r1,r1,16
mtlr r0
blr
_gz_roll_latch_L244:
lwz r6,788(r3)
stw r6,16(r10)
lwz r6,792(r3)
stw r6,20(r10)
lwz r6,796(r3)
stw r6,24(r10)
stw r7,12(r10)
b _gz_roll_latch_L225
_gz_roll_latch_L243:
lwz r10,56(r9)
cmplwi r10,48
bne+ _gz_roll_latch_L222
lwz r10,12(r9)
cmpwi r10,0
.int 0x4c820020

li r7,1
lis r8,_gz_game_counter@ha
stw r7,12(r9)
lis r10,_gz_roll_clip_state@ha
addi r10,r10,_gz_roll_clip_state@l
lwz r6,_gz_game_counter@l(r8)
stw r6,16(r9)
lwz r6,12(r10)
cmpwi r6,0
bne- _gz_roll_latch_L238
lwz r6,788(r3)
stw r6,16(r10)
lwz r6,792(r3)
stw r6,20(r10)
lwz r6,796(r3)
stw r6,24(r10)
stw r7,12(r10)
_gz_roll_latch_L238:
lwz r10,20(r9)
cmplwi r10,2
beq- _gz_roll_latch_L246
_gz_roll_latch_L240:
li r10,0
stw r10,20(r9)
blr
_gz_roll_latch_L246:
lwz r7,_gz_game_counter@l(r8)
lwz r10,24(r9)
subf r10,r10,r7
cmplwi r10,4
bgt- _gz_roll_latch_L240
lwz r5,24(r9)
lis r7,_gz_roll_feedback@ha
lwz r10,_gz_game_counter@l(r8)
addi r6,r7,_gz_roll_feedback@l
li r3,0
lis r4,_gz_roll_result_tick@ha
subf r10,r10,r5
li r5,1
stw r10,4(r6)
stw r3,12(r6)
lwz r8,_gz_game_counter@l(r8)
stw r8,_gz_roll_result_tick@l(r4)
stw r5,_gz_roll_feedback@l(r7)
stw r10,48(r9)
blr
_gz_roll_latch_L245:
lwz r7,_gz_game_counter@l(r8)
lwz r10,24(r9)
subf r10,r10,r7
cmplwi r10,4
bgt- _gz_roll_latch_L226
lwz r5,24(r9)
lis r7,_gz_roll_feedback@ha
lwz r10,_gz_game_counter@l(r8)
addi r6,r7,_gz_roll_feedback@l
li r3,0
lis r4,_gz_roll_result_tick@ha
subf r10,r10,r5
li r5,1
stw r10,4(r6)
stw r3,12(r6)
lwz r8,_gz_game_counter@l(r8)
stw r8,_gz_roll_result_tick@l(r4)
stw r5,_gz_roll_feedback@l(r7)
stw r10,48(r9)
b _gz_roll_latch_L219
.align 4
_gz_roll_press:
lwz r8,26096(r3)
lis r7,_gz_game_counter@ha
lis r9,_gz_roll_state@ha
lwz r7,_gz_game_counter@l(r7)
cmplwi r8,48
addi r10,r9,_gz_roll_state@l
beq- _gz_roll_latch_L281
_gz_roll_latch_L265:
lwz r9,_gz_roll_state@l(r9)
cmpwi cr7,r9,0
.int 0x4d9e0020

lwz r9,4(r10)
cmplw cr7,r3,r9
.int 0x4c9e0020

lwz r9,100(r10)
cmpwi cr7,r9,0
.int 0x4c9e0020

lwz r9,12(r10)
cmpwi cr7,r9,0
bne- cr7,_gz_roll_latch_L271
lwz r9,22684(r3)
lwz r8,22680(r3)
.int 0x4c820020

lwz r6,8(r10)
cmplw r6,r7
.int 0x4d820020

addis r9,r9,0xbe58
lis r6,0x18
cmplw r9,r6
.int 0x4d810020

lis r9,0x7f7f
addi r8,r8,-1
ori r9,r9,0xfffe
cmplw r8,r9
.int 0x4d810020

li r9,2
stw r9,20(r10)
stw r7,24(r10)
blr
_gz_roll_latch_L281:
lwz r6,_gz_roll_state@l(r9)
cmpwi cr7,r6,0
beq- cr7,_gz_roll_latch_L249
lwz r6,4(r10)
cmplw cr7,r3,r6
beq- cr7,_gz_roll_latch_L282
_gz_roll_latch_L249:
stwu r1,-24(r1)
mfcr r9
mflr r0
stw r9,20(r1)
stw r7,16(r1)
stw r8,12(r1)
stw r3,8(r1)
stw r0,28(r1)
bl _gz_roll_begin
lis r9,_gz_roll_state@ha
lwz r8,20(r1)
addi r10,r9,_gz_roll_state@l
lwz r9,_gz_roll_state@l(r9)
mtcrf 128,r8
lwz r7,16(r1)
cmpwi cr7,r9,0
lwz r8,12(r1)
lwz r3,8(r1)
beq- cr7,_gz_roll_latch_L247
lwz r9,4(r10)
cmplw cr7,r3,r9
bne- cr7,_gz_roll_latch_L247
lwz r9,100(r10)
cmpwi cr7,r9,0
bne- cr7,_gz_roll_latch_L247
lwz r9,12(r10)
cmpwi cr7,r9,0
bne- cr7,_gz_roll_latch_L251
lwz r9,22684(r3)
lwz r8,22680(r3)
bne- _gz_roll_latch_L247
lwz r6,8(r10)
cmplw r6,r7
beq- _gz_roll_latch_L247
addis r9,r9,0xbe58
lis r6,0x18
cmplw r9,r6
bgt- _gz_roll_latch_L247
lis r9,0x7f7f
addi r8,r8,-1
ori r9,r9,0xfffe
cmplw r8,r9
bgt- _gz_roll_latch_L247
li r9,2
stw r9,20(r10)
stw r7,24(r10)
_gz_roll_latch_L247:
lwz r0,28(r1)
addi r1,r1,24
mtlr r0
blr
_gz_roll_latch_L282:
lwz r6,56(r10)
cmplwi cr7,r6,48
bne+ cr7,_gz_roll_latch_L249
b _gz_roll_latch_L265
_gz_roll_latch_L271:
lwz r9,16(r10)
subf r9,r9,r7
cmplwi cr7,r9,8
.int 0x4d9d0020

beq- _gz_roll_latch_L278
addi r9,r8,-4
cmplwi r9,1
ble- _gz_roll_latch_L278
addi r8,r8,-30
cmplwi r8,1
.int 0x4da10020

_gz_roll_latch_L278:
li r9,1
stw r9,20(r10)
stw r7,24(r10)
blr
_gz_roll_latch_L251:
lwz r9,16(r10)
subf r9,r9,r7
cmplwi cr7,r9,8
bgt- cr7,_gz_roll_latch_L247
beq- _gz_roll_latch_L252
addi r9,r8,-4
cmplwi r9,1
ble- _gz_roll_latch_L252
addi r8,r8,-30
cmplwi r8,1
bgt+ _gz_roll_latch_L247
_gz_roll_latch_L252:
li r9,1
stw r9,20(r10)
stw r7,24(r10)
b _gz_roll_latch_L247
.align 4
_gz_roll_gate_finish:
lis r9,_gz_roll_state@ha
lwz r10,_gz_roll_state@l(r9)
addi r9,r9,_gz_roll_state@l
cmpwi r10,0
.int 0x4d820020

lwz r10,4(r9)
cmplw r10,r3
.int 0x4c820020

lwz r8,12(r9)
cmpwi r8,0
.int 0x4d820020

lwz r8,100(r9)
cmpwi r8,0
.int 0x4c820020

lwz r10,26096(r10)
lis r6,_gz_game_counter@ha
lwz r7,_gz_game_counter@l(r6)
addi r5,r10,-30
cmplwi r5,1
.int 0x4d810020

lwz r5,16(r9)
subf r5,r5,r7
cmplwi r5,8
.int 0x4d810020

xori r10,r10,0x1f
lis r3,_gz_roll_clip_state@ha
cntlzw r10,r10
li r5,1
srwi r10,r10,5
addi r3,r3,_gz_roll_clip_state@l
stw r5,36(r9)
lis r11,_gz_roll_feedback@ha
stw r10,40(r9)
addi r4,r11,_gz_roll_feedback@l
stw r7,96(r9)
lis r12,_gz_roll_result_tick@ha
stw r5,100(r9)
stw r5,20(r9)
stw r7,24(r9)
stw r8,8(r3)
lwz r10,16(r9)
subf r10,r10,r7
stw r10,4(r4)
stw r8,8(r4)
stw r5,12(r4)
lwz r8,_gz_game_counter@l(r6)
stw r8,_gz_roll_result_tick@l(r12)
stw r5,_gz_roll_feedback@l(r11)
stw r10,48(r9)
blr
.align 4
_gz_roll_clip_sample:
lis r10,_gz_roll_state@ha
lwz r8,_gz_roll_state@l(r10)
cmpwi r8,0
.int 0x4d820020

addi r10,r10,_gz_roll_state@l
mr r9,r3
lwz r8,4(r10)
cmplw r8,r3
.int 0x4c820020

lis r7,_gz_roll_clip_state@ha
addi r8,r7,_gz_roll_clip_state@l
lwz r6,12(r8)
cmpwi r6,0
.int 0x4d820020

lis r4,_gz_roll_feedback@ha
addi r4,r4,_gz_roll_feedback@l
lwz r6,8(r4)
cmplwi r6,2
.int 0x4d820020

lwz r6,108(r10)
cmplwi r6,2
.int 0x4d820020

lis r11,_gz_game_counter@ha
lwz r3,_gz_game_counter@l(r11)
lwz r5,26096(r9)
lwz r6,4(r8)
cmpwi r6,0
beq- _gz_roll_latch_L293
lwz r6,_gz_roll_clip_state@l(r7)
cmplw r6,r3
.int 0x4d820020

_gz_roll_latch_L293:
addi r6,r5,-43
li r0,1
cmplwi cr5,r6,5
stw r3,_gz_roll_clip_state@l(r7)
stw r0,4(r8)
ble- cr5,_gz_roll_latch_L294
addi r5,r5,-4
li r7,0
cmplwi r5,1
ble- _gz_roll_latch_L370
_gz_roll_latch_L295:
addi r12,r9,788
cmpwi r7,0
mr r5,r12
li r7,0
lis r0,0x7f80
_gz_roll_latch_L300:
addi r6,r7,4
beq- _gz_roll_latch_L296
lwz r3,0(r5)
slwi r6,r6,2
rlwinm r3,r3,0,1,8
cmpw r3,r0
beq- _gz_roll_latch_L371
lwzx r6,r8,r6
rlwinm r6,r6,0,1,8
xoris r6,r6,0x7f80
addic r3,r6,-1
subfe r6,r3,r6
cmpwi r6,0
_gz_roll_latch_L296:
addi r7,r7,1
addi r5,r5,4
cmplwi cr7,r7,3
bne+ cr7,_gz_roll_latch_L300
_gz_roll_latch_L299:
stwu r1,-24(r1)
addi r7,r9,792
bgt- cr5,_gz_roll_latch_L301
beq- _gz_roll_latch_L310
lwz r6,8(r8)
addi r6,r6,1
cmplwi r6,1
stw r6,8(r8)
ble+ _gz_roll_latch_L303
mflr r0
mr r3,r9
stw r12,16(r1)
stw r7,12(r1)
stw r9,8(r1)
stw r0,28(r1)
bl _gz_roll_latch_probe
cmpwi r3,0
lis r10,_gz_roll_clip_state@ha
addi r8,r10,_gz_roll_clip_state@l
lis r10,_gz_roll_state@ha
addi r10,r10,_gz_roll_state@l
lis r6,_gz_roll_feedback@ha
lwz r9,8(r1)
lis r11,_gz_game_counter@ha
lwz r7,12(r1)
addi r4,r6,_gz_roll_feedback@l
lwz r12,16(r1)
lwz r10,108(r10)
beq- _gz_roll_latch_L369
_gz_roll_latch_L325:
li r10,2
b _gz_roll_latch_L327
_gz_roll_latch_L301:
bne+ _gz_roll_latch_L372
_gz_roll_latch_L310:
li r10,0
stw r10,8(r8)
_gz_roll_latch_L303:
lwz r10,0(r12)
stw r10,28(r8)
lwz r10,0(r7)
stw r10,32(r8)
lwz r9,796(r9)
stw r9,36(r8)
addi r1,r1,24
blr
_gz_roll_latch_L370:
lwz r6,100(r10)
cmpwi r6,0
beq- _gz_roll_latch_L295
_gz_roll_latch_L294:
lwz r7,27144(r9)
rlwinm r7,r7,0,1,8
xoris r7,r7,0x7f80
addic r6,r7,-1
subfe r7,r6,r7
b _gz_roll_latch_L295
_gz_roll_latch_L371:
cmpwi cr7,r7,2
cmplwi cr6,r7,1
li r7,0
addi r5,r5,8
cmpwi r7,0
li r7,2
beq- cr7,_gz_roll_latch_L298
bne+ cr6,_gz_roll_latch_L296
li r7,0
cmpwi r7,0
b _gz_roll_latch_L299
_gz_roll_latch_L372:
lis r6,_gz_roll_latch_LC41@ha
lfs f12,27144(r9)
lfs f0,_gz_roll_latch_LC41@l(r6)
fcmpu cr0,f12,f0
cror 2,0,2
beq- _gz_roll_latch_L310
lis r6,_gz_roll_clip_state+20@ha
lfs f0,792(r9)
lfs f11,_gz_roll_clip_state+20@l(r6)
lis r6,_gz_roll_latch_LC0@ha
lfs f12,_gz_roll_latch_LC0@l(r6)
fsubs f0,f0,f11
fcmpu cr0,f0,f12
bnl+ _gz_roll_latch_L311
fneg f0,f0
_gz_roll_latch_L311:
lis r5,_gz_roll_latch_LC42@ha
lfs f12,_gz_roll_latch_LC42@l(r5)
fcmpu cr0,f0,f12
bgt- _gz_roll_latch_L310
lwz r5,8(r8)
cmpwi r5,0
beq- _gz_roll_latch_L315
lfs f0,788(r9)
lfs f11,28(r8)
lfs f12,_gz_roll_latch_LC0@l(r6)
fsubs f0,f0,f11
fcmpu cr0,f0,f12
bnl+ _gz_roll_latch_L316
fneg f0,f0
_gz_roll_latch_L316:
lis r5,_gz_roll_latch_LC4@ha
lfs f12,_gz_roll_latch_LC4@l(r5)
fcmpu cr0,f0,f12
bgt- _gz_roll_latch_L318
lis r3,_gz_roll_clip_state+28@ha
lfs f0,796(r9)
addi r3,r3,_gz_roll_clip_state+28@l
lfs f12,_gz_roll_latch_LC0@l(r6)
lfs f11,8(r3)
fsubs f0,f0,f11
fcmpu cr0,f0,f12
bnl+ _gz_roll_latch_L319
fneg f0,f0
_gz_roll_latch_L319:
lfs f12,_gz_roll_latch_LC4@l(r5)
fcmpu cr0,f0,f12
bgt- _gz_roll_latch_L318
lfs f0,792(r9)
lfs f11,4(r3)
lfs f12,_gz_roll_latch_LC0@l(r6)
fsubs f0,f0,f11
fcmpu cr0,f0,f12
bnl+ _gz_roll_latch_L321
fneg f0,f0
_gz_roll_latch_L321:
lis r6,_gz_roll_latch_LC1@ha
lfs f12,_gz_roll_latch_LC1@l(r6)
fcmpu cr0,f0,f12
bng- _gz_roll_latch_L315
_gz_roll_latch_L318:
li r6,0
stw r6,8(r8)
_gz_roll_latch_L315:
lwz r6,8(r8)
addi r6,r6,1
stw r6,8(r8)
lwz r6,8(r8)
cmplwi r6,5
ble+ _gz_roll_latch_L303
mflr r0
mr r3,r9
stw r12,16(r1)
stw r7,12(r1)
stw r9,8(r1)
stw r0,28(r1)
bl _gz_roll_latch_probe
cmpwi r3,0
lis r10,_gz_roll_state@ha
addi r10,r10,_gz_roll_state@l
lis r8,_gz_roll_clip_state@ha
lis r6,_gz_roll_feedback@ha
lwz r9,8(r1)
lwz r7,12(r1)
lis r11,_gz_game_counter@ha
lwz r12,16(r1)
addi r8,r8,_gz_roll_clip_state@l
lwz r10,108(r10)
addi r4,r6,_gz_roll_feedback@l
bne- _gz_roll_latch_L325
_gz_roll_latch_L369:
cmpwi r10,0
bne- _gz_roll_latch_L326
li r10,1
_gz_roll_latch_L327:
lwz r6,8(r4)
cmplw r10,r6
beq- _gz_roll_latch_L328
lwz r5,_gz_game_counter@l(r11)
lis r6,_gz_roll_result_tick@ha
stw r5,_gz_roll_result_tick@l(r6)
_gz_roll_latch_L328:
stw r10,8(r4)
_gz_roll_latch_L326:
lwz r0,28(r1)
li r10,0
stw r10,8(r8)
mtlr r0
b _gz_roll_latch_L303
_gz_roll_latch_L298:
li r10,0
stw r10,8(r8)
lwz r10,0(r12)
stw r10,28(r8)
lwz r10,792(r9)
stw r10,32(r8)
lwz r9,796(r9)
stw r9,36(r8)
blr
.align 4
_gz_roll_observe:
stwu r1,-32(r1)
mflr r0
stw r31,28(r1)
mr r31,r3
stw r0,36(r1)
bl _gz_roll_feedback_expire
lis r7,_gz_roll_state@ha
lwz r8,26096(r31)
addi r9,r7,_gz_roll_state@l
lwz r10,4(r9)
addi r6,r8,-43
cmplw r31,r10
bne- _gz_roll_latch_L438
cmplwi r6,5
ble- _gz_roll_latch_L377
_gz_roll_latch_L378:
stw r8,56(r9)
lwz r10,_gz_roll_state@l(r7)
cmpwi cr7,r10,0
beq- cr7,_gz_roll_latch_L373
_gz_roll_latch_L444:
lis r10,_gz_roll_feedback@ha
addi r6,r10,_gz_roll_feedback@l
lwz r5,8(r6)
cmplwi cr7,r5,2
beq- cr7,_gz_roll_latch_L439
_gz_roll_latch_L396:
lwz r10,12(r9)
cmpwi r10,0
beq- _gz_roll_latch_L399
lis r10,_gz_game_counter@ha
lwz r8,_gz_game_counter@l(r10)
lwz r10,16(r9)
subf r10,r10,r8
cmplwi r10,90
bgt- _gz_roll_latch_L440
_gz_roll_latch_L399:
lwz r10,12(r9)
cmpwi r10,0
beq- _gz_roll_latch_L400
lwz r10,56(r9)
cmplwi r10,48
beq- _gz_roll_latch_L400
lwz r10,100(r9)
cmpwi r10,0
bne- _gz_roll_latch_L400
lwz r10,20(r9)
cmplwi r10,1
beq- _gz_roll_latch_L441
_gz_roll_latch_L400:
lwz r0,36(r1)
mr r3,r31
lwz r31,28(r1)
addi r1,r1,32
mtlr r0
b _gz_roll_clip_sample
_gz_roll_latch_L440:
lis r6,_gz_roll_clip_state@ha
li r10,0
addi r8,r6,_gz_roll_clip_state@l
stw r10,100(r9)
stw r10,56(r9)
stw r10,36(r9)
stw r10,20(r9)
stw r10,12(r9)
stw r10,4(r9)
stw r10,_gz_roll_state@l(r7)
stw r10,12(r8)
stw r10,8(r8)
stw r10,4(r8)
stw r10,_gz_roll_clip_state@l(r6)
_gz_roll_latch_L373:
lwz r0,36(r1)
lwz r31,28(r1)
addi r1,r1,32
mtlr r0
blr
_gz_roll_latch_L438:
cmplwi r6,5
lis r5,_gz_roll_clip_state@ha
li r10,0
addi r6,r5,_gz_roll_clip_state@l
stw r10,100(r9)
stw r10,56(r9)
stw r10,36(r9)
stw r10,20(r9)
stw r10,12(r9)
stw r10,4(r9)
stw r10,_gz_roll_state@l(r7)
stw r10,12(r6)
stw r10,8(r6)
stw r10,4(r6)
stw r10,_gz_roll_clip_state@l(r5)
bgt- _gz_roll_latch_L373
mfcr r9
mr r3,r31
stw r8,12(r1)
stw r9,8(r1)
bl _gz_roll_begin
lwz r9,8(r1)
lwz r8,12(r1)
mtcrf 128,r9
lis r9,_gz_roll_state@ha
mr r7,r9
addi r9,r9,_gz_roll_state@l
_gz_roll_latch_L377:
cmplwi cr7,r8,48
lwz r6,22684(r31)
lwz r10,56(r9)
beq- cr7,_gz_roll_latch_L442
cmplwi cr7,r8,43
beq- cr7,_gz_roll_latch_L382
cmplwi cr7,r8,45
beq- cr7,_gz_roll_latch_L443
lwz r6,_gz_roll_state@l(r7)
cmpwi cr7,r6,0
beq- cr7,_gz_roll_latch_L402
lwz r6,100(r9)
cmpwi cr7,r6,0
bne- cr7,_gz_roll_latch_L402
addi r10,r10,-43
cmplwi cr7,r10,5
bgt- cr7,_gz_roll_latch_L402
b _gz_roll_latch_L378
_gz_roll_latch_L382:
cmplwi cr7,r10,43
beq- cr7,_gz_roll_latch_L384
_gz_roll_latch_L437:
lwz r10,_gz_roll_state@l(r7)
cmpwi cr7,r10,0
beq- cr7,_gz_roll_latch_L402
lwz r10,100(r9)
_gz_roll_latch_L402:
mfcr r9
mr r3,r31
stw r8,8(r1)
stw r9,12(r1)
bl _gz_roll_begin
lis r9,_gz_roll_state@ha
lwz r8,8(r1)
mr r7,r9
addi r9,r9,_gz_roll_state@l
lwz r10,12(r1)
stw r8,56(r9)
mtcrf 128,r10
lwz r10,_gz_roll_state@l(r7)
cmpwi cr7,r10,0
bne+ cr7,_gz_roll_latch_L444
b _gz_roll_latch_L373
_gz_roll_latch_L439:
lwz r5,100(r9)
cmpwi cr7,r5,0
bne- cr7,_gz_roll_latch_L396
ble- _gz_roll_latch_L396
addi r8,r8,-4
cmplwi r8,1
ble- _gz_roll_latch_L445
_gz_roll_latch_L398:
li r8,0
stw r8,8(r6)
lwz r10,_gz_roll_feedback@l(r10)
cmpwi r10,0
bne- _gz_roll_latch_L396
lis r8,_gz_roll_result_tick@ha
stw r10,_gz_roll_result_tick@l(r8)
b _gz_roll_latch_L396
_gz_roll_latch_L442:
lwz r5,_gz_roll_state@l(r7)
cmpwi cr7,r5,0
beq- cr7,_gz_roll_latch_L390
lwz r5,100(r9)
cmpwi cr7,r5,0
bne- cr7,_gz_roll_latch_L390
addi r5,r10,-43
cmplwi cr7,r5,5
bgt- cr7,_gz_roll_latch_L390
cmplwi cr7,r10,48
bne+ cr7,_gz_roll_latch_L390
lwz r10,80(r9)
cmpwi cr7,r10,0
beq- cr7,_gz_roll_latch_L381
lwz r10,80(r9)
addis r5,r6,0x1
cmplw cr7,r5,r10
bge- cr7,_gz_roll_latch_L381
_gz_roll_latch_L390:
mfcr r9
mr r3,r31
stw r6,12(r1)
stw r9,16(r1)
stw r8,8(r1)
bl _gz_roll_begin
lwz r10,16(r1)
lis r9,_gz_roll_state@ha
lwz r8,8(r1)
mr r7,r9
lwz r6,12(r1)
mtcrf 128,r10
addi r9,r9,_gz_roll_state@l
_gz_roll_latch_L381:
stw r6,80(r9)
stw r8,56(r9)
lwz r10,_gz_roll_state@l(r7)
cmpwi cr7,r10,0
bne+ cr7,_gz_roll_latch_L444
b _gz_roll_latch_L373
_gz_roll_latch_L443:
cmplwi cr7,r10,45
bne+ cr7,_gz_roll_latch_L437
_gz_roll_latch_L384:
lwz r10,_gz_roll_state@l(r7)
cmpwi cr7,r10,0
beq- cr7,_gz_roll_latch_L402
lwz r10,100(r9)
cmpwi cr7,r10,0
bne- cr7,_gz_roll_latch_L402
b _gz_roll_latch_L378
_gz_roll_latch_L441:
lis r10,_gz_game_counter@ha
lwz r10,_gz_game_counter@l(r10)
lwz r9,24(r9)
subf r9,r9,r10
cmplwi r9,1
bgt+ _gz_roll_latch_L400
mr r3,r31
bl _gz_roll_gate_finish
b _gz_roll_latch_L400
_gz_roll_latch_L445:
lwz r8,12(r9)
cmpwi r8,0
beq- _gz_roll_latch_L398
lis r8,_gz_game_counter@ha
lwz r5,_gz_game_counter@l(r8)
lwz r8,16(r9)
subf r8,r8,r5
cmplwi r8,8
ble- _gz_roll_latch_L396
b _gz_roll_latch_L398
.align 4
_gz_roll_volume_trace:
.int 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.int 0,0,0,0,0,0,0,0
.align 4
_gz_roll_clip_state:
.int 0,0,0,0,0,0,0,0,0,0
.align 4
_gz_roll_result_tick:
.int 0
.align 4
_gz_roll_feedback:
.int 0,0,0,0
.origin = codecave
.align 4
_gz_roll_latch_LC0:
.int	0
.align 4
_gz_roll_latch_LC1:
.int	1065353216
.align 4
_gz_roll_latch_LC4:
.int	1056964608
.align 4
_gz_roll_latch_LC5:
.int	-1085485875
.align 4
_gz_roll_latch_LC6:
.int	1050253722
.align 4
_gz_roll_latch_LC7:
.int	1065185444
.align 4
_gz_roll_latch_LC8:
.int	1065437102
.align 4
_gz_roll_latch_LC9:
.int	1057803469
.align 4
_gz_roll_latch_LC17:
.int	1133903872
.align 4
_gz_roll_latch_LC18:
.int	1120403456
.align 4
_gz_roll_latch_LC19:
.int	1106247680
.align 4
_gz_roll_latch_LC20:
.int	1115684864
.align 4
_gz_roll_latch_LC21:
.int	1036831949
.align 4
_gz_roll_latch_LC22:
.int	1063675494
.align 4
_gz_roll_latch_LC23:
.int	1066192077
.align 4
_gz_roll_latch_LC24:
.int	-1046478848
.align 4
_gz_roll_latch_LC25:
.int	1125515264
.align 4
_gz_roll_latch_LC26:
.int	-1090519040
.align 4
_gz_roll_latch_LC41:
.int	1045220557
.align 4
_gz_roll_latch_LC42:
.int	1110704128
