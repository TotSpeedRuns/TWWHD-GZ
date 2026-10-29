/* Native frame service. No host callback or replacement native API.
 * Normal snapshot admission remains unchanged. The existing typed participant
 * consumes its own guarded nonce and retains failure/undo ownership. */
#include "wct_heap_lease.h"
#include "lifecycle_writer.h"
typedef unsigned int U32;
extern volatile U32 _gz_pbac_frame;
extern void _gz_bg_probe_step(void),_gz_cs_probe_step(void),_gz_cgp_tick(void),_gz_cgw_tick(void);
extern void _gz_hbp_dispatch(void),_gz_sc_storage_tick(void),_gz_jp_tick(void);
U32 _gz_nbd_report[16]={0x4e424431u,64,1};
static volatile U32 busy,seen,last;
U32 _gz_nbd_context_owned(U32 f){return busy==1&&seen==1&&last==f&&f==_gz_pbac_frame?1u:0u;}
void _gz_nbd_tick(void){U32 f=_gz_pbac_frame;
 _gz_nbd_report[3]++;
 if(!__sync_bool_compare_and_swap(&busy,0,1)){_gz_nbd_report[7]++;return;}
 if(seen&&last==f){_gz_nbd_report[8]++;busy=0;return;}
 seen=1;last=f;_gz_nbd_report[4]++;_gz_nbd_report[5]=f;
 __sync_synchronize();_gz_sc_storage_tick();_gz_jp_tick();_gz_cgp_tick();_gz_cgw_tick();_gz_bg_probe_step();_gz_cs_probe_step();if(!_gz_lrw_dispatch()){_gz_wct_native_enter();_gz_hbp_dispatch();_gz_wct_native_exit();}
 _gz_nbd_report[6]++;__sync_synchronize();busy=0;
}
