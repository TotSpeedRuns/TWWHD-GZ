/* Private fixed worker-purpose metadata; never a context setter. */
static struct {uint32_t entered,request,frame,slot;} wct_source __attribute__((section(".bss.gz_swc_native")));
