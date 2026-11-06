/* Lossless recovered ordinary-reader core. See ordinary_reader.h and roles.json. */
/* Numeric comments are Source offsets; all external addresses are symbolic. */
.section .text.gz_ordinary_reader_core,"ax",@progbits
.balign 4
.extern _gz_ordinary_reader_view_preflight
.type _gz_ordinary_reader_view_preflight,@function
.extern _gz_ordinary_reader_workers_quiet
.type _gz_ordinary_reader_workers_quiet,@function
.extern _gz_ordinary_reader_current_thread
.type _gz_ordinary_reader_current_thread,@function
.extern _gz_ordinary_reader_gate_try_exclusive
.type _gz_ordinary_reader_gate_try_exclusive,@function
.extern _gz_ordinary_reader_gate_leave_exclusive
.type _gz_ordinary_reader_gate_leave_exclusive,@function
.extern _gz_ordinary_reader_sticky_halt
.type _gz_ordinary_reader_sticky_halt,@function
.extern OSDisableInterrupts
.type OSDisableInterrupts,@function
.extern OSRestoreInterrupts
.type OSRestoreInterrupts,@function
.extern _gz_ordinary_reader_report
.type _gz_ordinary_reader_report,@object
.extern _gz_ordinary_reader_jwa_owner
.type _gz_ordinary_reader_jwa_owner,@object
.extern _gz_ordinary_reader_nbd_report
.type _gz_ordinary_reader_nbd_report,@object

.global _gz_ordinary_reader_current
.type _gz_ordinary_reader_current,@function
_gz_ordinary_reader_current:
.Lorr_000:
    lis      r9,_gz_ordinary_reader_holder@ha                 /* +0x000 */
    lwz      r8,_gz_ordinary_reader_holder@l(r9)              /* +0x004 */
    cmpwi    r8,0                                             /* +0x008 */
    bc       12,2,.Lorr_038                                   /* +0x00c */
    addi     r9,r9,_gz_ordinary_reader_holder@l               /* +0x010 */
    lwz      r7,4(r9)                                         /* +0x014 */
    cmpwi    r7,0                                             /* +0x018 */
    bc       4,2,.Lorr_038                                    /* +0x01c */
    lwz      r7,8(r9)                                         /* +0x020 */
    cmpw     r7,r3                                            /* +0x024 */
    bc       4,2,.Lorr_038                                    /* +0x028 */
    lwz      r10,12(r9)                                       /* +0x02c */
    cmpw     r10,r4                                           /* +0x030 */
    bc       12,2,.Lorr_040                                   /* +0x034 */
.Lorr_038:
    li       r3,2930                                          /* +0x038 */
    blr                                                       /* +0x03c */
.Lorr_040:
    stwu     r1,-16(r1)                                       /* +0x040 */
    mflr     r0                                               /* +0x044 */
    stw      r0,20(r1)                                        /* +0x048 */
    bl       _gz_ordinary_reader_view_preflight               /* +0x04c */
    cmpwi    r3,0                                             /* +0x050 */
    bc       12,2,.Lorr_134                                   /* +0x054 */
    stw      r30,8(r1)                                        /* +0x058 */
    lis      r30,_gz_ordinary_reader_holder@ha                /* +0x05c */
    addi     r9,r30,_gz_ordinary_reader_holder@l              /* +0x060 */
    stw      r31,12(r1)                                       /* +0x064 */
    lwz      r10,16(r9)                                       /* +0x068 */
    mr       r31,r10                                          /* +0x06c */
    bl       _gz_ordinary_reader_current_thread               /* +0x070 */
    cmpw     r31,r3                                           /* +0x074 */
    bc       4,2,.Lorr_12c                                    /* +0x078 */
    lis      r10,_gz_ordinary_reader_nbd_report+16@ha         /* +0x07c */
    addi     r9,r30,_gz_ordinary_reader_holder@l              /* +0x080 */
    lwz      r8,20(r9)                                        /* +0x084 */
    lwz      r10,_gz_ordinary_reader_nbd_report+16@l(r10)     /* +0x088 */
    cmpw     r8,r10                                           /* +0x08c */
    bc       4,2,.Lorr_12c                                    /* +0x090 */
    lwz      r10,40(r9)                                       /* +0x094 */
    cmplwi   r10,1                                            /* +0x098 */
    bc       4,2,.Lorr_12c                                    /* +0x09c */
    lis      r10,_gz_ordinary_reader_jwa_owner@ha             /* +0x0a0 */
    lwz      r7,28(r9)                                        /* +0x0a4 */
    addi     r10,r10,_gz_ordinary_reader_jwa_owner@l          /* +0x0a8 */
    addi     r8,r10,8                                         /* +0x0ac */
    cmpw     r7,r8                                            /* +0x0b0 */
    bc       4,2,.Lorr_12c                                    /* +0x0b4 */
    lwz      r7,32(r9)                                        /* +0x0b8 */
    lwz      r8,16(r9)                                        /* +0x0bc */
    cmplw    r7,r8                                            /* +0x0c0 */
    bc       4,2,.Lorr_12c                                    /* +0x0c4 */
    lwz      r7,36(r9)                                        /* +0x0c8 */
    cmpwi    r7,0                                             /* +0x0cc */
    bc       12,2,.Lorr_12c                                   /* +0x0d0 */
    lwz      r7,80(r10)                                       /* +0x0d4 */
    cmplwi   r7,1                                             /* +0x0d8 */
    bc       4,2,.Lorr_12c                                    /* +0x0dc */
    lwz      r7,84(r10)                                       /* +0x0e0 */
    cmpwi    r7,0                                             /* +0x0e4 */
    bc       4,2,.Lorr_12c                                    /* +0x0e8 */
    lwz      r7,88(r10)                                       /* +0x0ec */
    cmpwi    r7,0                                             /* +0x0f0 */
    bc       4,2,.Lorr_12c                                    /* +0x0f4 */
    lwz      r7,100(r10)                                      /* +0x0f8 */
    cmplw    r7,r8                                            /* +0x0fc */
    bc       4,2,.Lorr_12c                                    /* +0x100 */
    lwz      r10,96(r10)                                      /* +0x104 */
    lwz      r9,36(r9)                                        /* +0x108 */
    cmplw    r10,r9                                           /* +0x10c */
    bc       4,2,.Lorr_12c                                    /* +0x110 */
    bl       _gz_ordinary_reader_workers_quiet                /* +0x114 */
    cmpwi    r3,0                                             /* +0x118 */
    bc       4,2,.Lorr_148                                    /* +0x11c */
    lwz      r30,8(r1)                                        /* +0x120 */
    lwz      r31,12(r1)                                       /* +0x124 */
    b        .Lorr_138                                        /* +0x128 */
.Lorr_12c:
    lwz      r30,8(r1)                                        /* +0x12c */
    lwz      r31,12(r1)                                       /* +0x130 */
.Lorr_134:
    li       r3,2930                                          /* +0x134 */
.Lorr_138:
    lwz      r0,20(r1)                                        /* +0x138 */
    addi     r1,r1,16                                         /* +0x13c */
    mtlr     r0                                               /* +0x140 */
    blr                                                       /* +0x144 */
.Lorr_148:
    lwz      r30,8(r1)                                        /* +0x148 */
    li       r3,5                                             /* +0x14c */
    lwz      r31,12(r1)                                       /* +0x150 */
    b        .Lorr_138                                        /* +0x154 */
.size _gz_ordinary_reader_current,.-_gz_ordinary_reader_current

.global _gz_ordinary_reader_release
.type _gz_ordinary_reader_release,@function
_gz_ordinary_reader_release:
.Lorr_158:
    lis      r9,_gz_ordinary_reader_holder@ha                 /* +0x158 */
    lwz      r8,_gz_ordinary_reader_holder@l(r9)              /* +0x15c */
    cmpwi    r8,0                                             /* +0x160 */
    bc       12,2,.Lorr_178                                   /* +0x164 */
    addi     r10,r9,_gz_ordinary_reader_holder@l              /* +0x168 */
    lwz      r8,8(r10)                                        /* +0x16c */
    cmpw     r8,r3                                            /* +0x170 */
    bc       12,2,.Lorr_180                                   /* +0x174 */
.Lorr_178:
    li       r3,2931                                          /* +0x178 */
    b        _gz_ordinary_reader_sticky_halt                  /* +0x17c */
.Lorr_180:
    stwu     r1,-16(r1)                                       /* +0x180 */
    mflr     r0                                               /* +0x184 */
    stw      r0,20(r1)                                        /* +0x188 */
    stw      r31,12(r1)                                       /* +0x18c */
    bl       OSDisableInterrupts                              /* +0x190 */
    mr       r31,r3                                           /* +0x194 */
    bl       OSRestoreInterrupts                              /* +0x198 */
    cmpwi    r31,0                                            /* +0x19c */
    bc       12,2,.Lorr_1d4                                   /* +0x1a0 */
    cmpwi    r3,0                                             /* +0x1a4 */
    bc       4,2,.Lorr_1d4                                    /* +0x1a8 */
    stw      r30,8(r1)                                        /* +0x1ac */
    lis      r30,_gz_ordinary_reader_holder@ha                /* +0x1b0 */
    addi     r10,r30,_gz_ordinary_reader_holder@l             /* +0x1b4 */
    lwz      r9,16(r10)                                       /* +0x1b8 */
    mr       r31,r9                                           /* +0x1bc */
    bl       _gz_ordinary_reader_current_thread               /* +0x1c0 */
    cmpw     r31,r3                                           /* +0x1c4 */
    addi     r10,r30,_gz_ordinary_reader_holder@l             /* +0x1c8 */
    bc       12,2,.Lorr_1ec                                   /* +0x1cc */
    lwz      r30,8(r1)                                        /* +0x1d0 */
.Lorr_1d4:
    li       r3,2931                                          /* +0x1d4 */
.Lorr_1d8:
    lwz      r0,20(r1)                                        /* +0x1d8 */
    lwz      r31,12(r1)                                       /* +0x1dc */
    addi     r1,r1,16                                         /* +0x1e0 */
    mtlr     r0                                               /* +0x1e4 */
    b        _gz_ordinary_reader_sticky_halt                  /* +0x1e8 */
.Lorr_1ec:
    lis      r3,_gz_ordinary_reader_jwa_owner@ha              /* +0x1ec */
    addi     r4,r10,28                                        /* +0x1f0 */
    addi     r3,r3,_gz_ordinary_reader_jwa_owner@l            /* +0x1f4 */
    addi     r3,r3,8                                          /* +0x1f8 */
    bl       _gz_ordinary_reader_gate_leave_exclusive         /* +0x1fc */
    addi     r10,r30,_gz_ordinary_reader_holder@l             /* +0x200 */
    lwz      r7,40(r10)                                       /* +0x204 */
    lis      r8,_gz_ordinary_reader_report@ha                 /* +0x208 */
    addi     r8,r8,_gz_ordinary_reader_report@l               /* +0x20c */
    or.      r7,r3,r7                                         /* +0x210 */
    stw      r3,32(r8)                                        /* +0x214 */
    bc       4,2,.Lorr_228                                    /* +0x218 */
    lwz      r10,28(r10)                                      /* +0x21c */
    cmpwi    r10,0                                            /* +0x220 */
    bc       12,2,.Lorr_234                                   /* +0x224 */
.Lorr_228:
    lwz      r30,8(r1)                                        /* +0x228 */
    li       r3,2931                                          /* +0x22c */
    b        .Lorr_1d8                                        /* +0x230 */
.Lorr_234:
    lwz      r0,20(r1)                                        /* +0x234 */
    li       r7,4                                             /* +0x238 */
    stw      r10,_gz_ordinary_reader_holder@l(r30)            /* +0x23c */
    li       r3,0                                             /* +0x240 */
    stw      r7,16(r8)                                        /* +0x244 */
    mtlr     r0                                               /* +0x248 */
    lwz      r30,8(r1)                                        /* +0x24c */
    lwz      r31,12(r1)                                       /* +0x250 */
    addi     r1,r1,16                                         /* +0x254 */
    blr                                                       /* +0x258 */
.size _gz_ordinary_reader_release,.-_gz_ordinary_reader_release

.global _gz_ordinary_reader_acquire
.type _gz_ordinary_reader_acquire,@function
_gz_ordinary_reader_acquire:
.Lorr_25c:
    lis      r6,_gz_ordinary_reader_holder@ha                 /* +0x25c */
    addi     r10,r6,_gz_ordinary_reader_holder@l              /* +0x260 */
    lwz      r7,_gz_ordinary_reader_holder@l(r6)              /* +0x264 */
    lwz      r9,4(r10)                                        /* +0x268 */
    lwz      r8,40(r10)                                       /* +0x26c */
    or       r9,r9,r7                                         /* +0x270 */
    or.      r9,r9,r8                                         /* +0x274 */
    bc       4,2,.Lorr_394                                    /* +0x278 */
    lwz      r8,28(r10)                                       /* +0x27c */
    li       r9,2931                                          /* +0x280 */
    cmpwi    r8,0                                             /* +0x284 */
    bc       12,2,.Lorr_294                                   /* +0x288 */
    mr       r3,r9                                            /* +0x28c */
    blr                                                       /* +0x290 */
.Lorr_294:
    stwu     r1,-24(r1)                                       /* +0x294 */
    mflr     r0                                               /* +0x298 */
    stw      r31,20(r1)                                       /* +0x29c */
    mr       r31,r3                                           /* +0x2a0 */
    stw      r0,28(r1)                                        /* +0x2a4 */
    stw      r4,8(r1)                                         /* +0x2a8 */
    bl       _gz_ordinary_reader_view_preflight               /* +0x2ac */
    cmpwi    r3,0                                             /* +0x2b0 */
    bc       12,2,.Lorr_3f0                                   /* +0x2b4 */
    lis      r8,_gz_ordinary_reader_holder@ha                 /* +0x2b8 */
    lwz      r7,8(r1)                                         /* +0x2bc */
    addi     r10,r8,_gz_ordinary_reader_holder@l              /* +0x2c0 */
    li       r3,2931                                          /* +0x2c4 */
    lwz      r9,24(r10)                                       /* +0x2c8 */
    cmpwi    r9,-1                                            /* +0x2cc */
    bc       12,2,.Lorr_380                                   /* +0x2d0 */
    stw      r30,16(r1)                                       /* +0x2d4 */
    stw      r7,8(r1)                                         /* +0x2d8 */
    stw      r7,12(r10)                                       /* +0x2dc */
    stw      r31,8(r10)                                       /* +0x2e0 */
    bl       _gz_ordinary_reader_current_thread               /* +0x2e4 */
    lis      r7,_gz_ordinary_reader_holder@ha                 /* +0x2e8 */
    mr       r0,r3                                            /* +0x2ec */
    addi     r10,r7,_gz_ordinary_reader_holder@l              /* +0x2f0 */
    lis      r9,_gz_ordinary_reader_nbd_report+16@ha          /* +0x2f4 */
    lwz      r8,24(r10)                                       /* +0x2f8 */
    lis      r3,_gz_ordinary_reader_jwa_owner@ha              /* +0x2fc */
    lwz      r11,_gz_ordinary_reader_nbd_report+16@l(r9)      /* +0x300 */
    addi     r3,r3,_gz_ordinary_reader_jwa_owner@l            /* +0x304 */
    lis      r9,_gz_ordinary_reader_report@ha                 /* +0x308 */
    li       r5,1                                             /* +0x30c */
    addi     r9,r9,_gz_ordinary_reader_report@l               /* +0x310 */
    addi     r8,r8,1                                          /* +0x314 */
    addi     r3,r3,8                                          /* +0x318 */
    addi     r4,r10,28                                        /* +0x31c */
    stw      r31,12(r9)                                       /* +0x320 */
    mr       r30,r7                                           /* +0x324 */
    stw      r5,16(r9)                                        /* +0x328 */
    stw      r5,_gz_ordinary_reader_holder@l(r7)              /* +0x32c */
    stw      r8,24(r9)                                        /* +0x330 */
    stw      r0,16(r10)                                       /* +0x334 */
    stw      r11,20(r10)                                      /* +0x338 */
    stw      r8,24(r10)                                       /* +0x33c */
    bl       _gz_ordinary_reader_gate_try_exclusive           /* +0x340 */
    cmpwi    r3,0                                             /* +0x344 */
    lis      r9,_gz_ordinary_reader_report@ha                 /* +0x348 */
    addi     r9,r9,_gz_ordinary_reader_report@l               /* +0x34c */
    lwz      r7,8(r1)                                         /* +0x350 */
    stw      r3,28(r9)                                        /* +0x354 */
    bc       12,2,.Lorr_3a0                                   /* +0x358 */
    addi     r10,r30,_gz_ordinary_reader_holder@l             /* +0x35c */
    lwz      r9,40(r10)                                       /* +0x360 */
    cmpwi    r9,0                                             /* +0x364 */
    bc       4,2,.Lorr_378                                    /* +0x368 */
    lwz      r9,28(r10)                                       /* +0x36c */
    cmpwi    r9,0                                             /* +0x370 */
    bc       12,2,.Lorr_40c                                   /* +0x374 */
.Lorr_378:
    li       r3,2931                                          /* +0x378 */
.Lorr_37c:
    lwz      r30,16(r1)                                       /* +0x37c */
.Lorr_380:
    lwz      r0,28(r1)                                        /* +0x380 */
    lwz      r31,20(r1)                                       /* +0x384 */
    addi     r1,r1,24                                         /* +0x388 */
    mtlr     r0                                               /* +0x38c */
    b        _gz_ordinary_reader_sticky_halt                  /* +0x390 */
.Lorr_394:
    li       r9,2931                                          /* +0x394 */
    mr       r3,r9                                            /* +0x398 */
    blr                                                       /* +0x39c */
.Lorr_3a0:
    li       r10,2                                            /* +0x3a0 */
    stw      r7,8(r1)                                         /* +0x3a4 */
    stw      r10,16(r9)                                       /* +0x3a8 */
    bl       _gz_ordinary_reader_workers_quiet                /* +0x3ac */
    cmpwi    r3,0                                             /* +0x3b0 */
    lis      r9,_gz_ordinary_reader_report@ha                 /* +0x3b4 */
    addi     r9,r9,_gz_ordinary_reader_report@l               /* +0x3b8 */
    stw      r3,36(r9)                                        /* +0x3bc */
    bc       4,2,.Lorr_428                                    /* +0x3c0 */
    lwz      r7,8(r1)                                         /* +0x3c4 */
    mr       r3,r31                                           /* +0x3c8 */
    mr       r4,r7                                            /* +0x3cc */
    bl       .Lorr_000                                        /* +0x3d0 */
    mr.      r9,r3                                            /* +0x3d4 */
    bc       13,2,.Lorr_420                                   /* +0x3d8 */
    mr       r3,r31                                           /* +0x3dc */
    bl       .Lorr_158                                        /* +0x3e0 */
    lwz      r30,16(r1)                                       /* +0x3e4 */
    mr.      r9,r3                                            /* +0x3e8 */
    bc       4,2,.Lorr_3f4                                    /* +0x3ec */
.Lorr_3f0:
    li       r9,2930                                          /* +0x3f0 */
.Lorr_3f4:
    lwz      r0,28(r1)                                        /* +0x3f4 */
    mr       r3,r9                                            /* +0x3f8 */
    lwz      r31,20(r1)                                       /* +0x3fc */
    addi     r1,r1,24                                         /* +0x400 */
    mtlr     r0                                               /* +0x404 */
    blr                                                       /* +0x408 */
.Lorr_40c:
    cmplwi   r3,1                                             /* +0x40c */
    stw      r9,_gz_ordinary_reader_holder@l(r30)             /* +0x410 */
    bc       12,2,.Lorr_448                                   /* +0x414 */
    li       r3,2930                                          /* +0x418 */
    b        .Lorr_37c                                        /* +0x41c */
.Lorr_420:
    lwz      r30,16(r1)                                       /* +0x420 */
    b        .Lorr_3f4                                        /* +0x424 */
.Lorr_428:
    stw      r3,8(r1)                                         /* +0x428 */
    mr       r3,r31                                           /* +0x42c */
    bl       .Lorr_158                                        /* +0x430 */
    mr.      r9,r3                                            /* +0x434 */
    bc       5,2,.Lorr_420                                    /* +0x438 */
    lwz      r10,8(r1)                                        /* +0x43c */
    cmplwi   r10,1                                            /* +0x440 */
    bc       4,2,.Lorr_454                                    /* +0x444 */
.Lorr_448:
    lwz      r30,16(r1)                                       /* +0x448 */
    li       r9,5                                             /* +0x44c */
    b        .Lorr_3f4                                        /* +0x450 */
.Lorr_454:
    lwz      r30,16(r1)                                       /* +0x454 */
    b        .Lorr_3f0                                        /* +0x458 */
.size _gz_ordinary_reader_acquire,.-_gz_ordinary_reader_acquire

.section .bss.gz_ordinary_reader_holder,"aw",@nobits
.balign 4
.global _gz_ordinary_reader_holder
.type _gz_ordinary_reader_holder,@object
_gz_ordinary_reader_holder:
    .zero 44
.size _gz_ordinary_reader_holder,44

.section .note.GNU-stack,"",@progbits
