.section .text,"ax",@progbits
.balign 4
.global _gz_identity_record_safe
.type _gz_identity_record_safe,@function
_gz_identity_record_safe:
stwu 1,-0xA0(1)
mflr 0
stw 0,0xA4(1)
stmw 6,0x38(1)
bl _gz_identity_record_validate_c
lmw 6,0x38(1)
lwz 0,0xA4(1)
mtlr 0
addi 1,1,0xA0
blr
.size _gz_identity_record_safe,.-_gz_identity_record_safe
