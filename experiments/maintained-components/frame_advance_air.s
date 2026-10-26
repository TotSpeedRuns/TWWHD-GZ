/* Whole native air countdown-state handler entry: 0x026E743C.
 * The separate draw virtual at 0x026E66E4 stays live. No native state store.
 * A granted frame has forced=0 through the existing Frame Advance boundary.
 */
.pushsection .text.gz_fad_air,"ax",@progbits
.balign 4
.global _gz_fad_air_update
.type _gz_fad_air_update,@function
_gz_fad_air_update:
lis 12,_gz_fad_forced@ha
lwz 12,_gz_fad_forced@l(12)
cmpwi 12,0
bne .Lgz_fad_air_paused
/* Exact displaced retail first instruction; original prologue continues. */
mflr 0
b _gz_fad_air_resume
.Lgz_fad_air_paused:
blr
.size _gz_fad_air_update,.-_gz_fad_air_update
.popsection
