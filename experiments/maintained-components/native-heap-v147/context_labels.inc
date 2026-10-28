/* Scalar labels from the existing genuine private context. Separate C ABI
 * spelling prevents cloning the old authentication function into new code.
 * No new context, cookie, callback or permission can be supplied by callers.
 */
extern uint32_t jbm_real_context_owned(uint32_t cookie) __asm__("_gz_jol_context_owned");
#define JBM_CONTEXT __attribute__((noinline, noclone, section(".text.gz_jbm_ctx")))
JBM_CONTEXT uint32_t _gz_jol_context_frame(uint32_t cookie) {
    return jbm_real_context_owned(cookie) ? jol_context.frame : 0;
}
JBM_CONTEXT uint32_t _gz_jol_context_parent(uint32_t cookie) {
    return jbm_real_context_owned(cookie) ? jol_context.parent : 0;
}
#undef JBM_CONTEXT
