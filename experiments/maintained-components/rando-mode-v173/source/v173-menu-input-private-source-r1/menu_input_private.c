typedef unsigned int U;
extern unsigned char _gz_menu_input_trace[24];
__asm__(".type _gz_menu_input_trace,@object");
_Static_assert(sizeof(U)==4 && sizeof(void*)==4,"PPC32 span ABI");
/* Metadata rejection only. Does not dereference either input or owner. */
U _gz_mit_private_apart(U p,U n) {
    U base=(U)_gz_menu_input_trace;
    return n && n<=0xffffffffu-p && 24u<=0xffffffffu-base &&
        (p+n<=base || base+24u<=p);
}
