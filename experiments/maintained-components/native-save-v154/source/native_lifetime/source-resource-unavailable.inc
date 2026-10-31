/* Production-only laboratory exclusion. No observer/HR/object-binding
 * function is replaced. Unavailable is an explicit failure, never a valid
 * image or a restore capability. Surviving diagnostics stay private. */
#include "source_directory.h"
#include "resource_references.h"
ScDirectory *_gz_sc_storage;
u32 _gz_scs_report[16]={0x53435331u,64,1,2,SC_UNAVAILABLE,sizeof(ScDirectory)};
/* Same 64-byte SI diagnostic layout; no rows or heaps are allocated. */
u32 _gz_si_control[16]={0x53494e31u,64,1,0,SC_UNAVAILABLE,0,0,0,0,0,0,0,0,0,31,0};
u32 _gz_sc_storage_ready(void){return 0;}
void _gz_sc_storage_tick(void){}
u32 _gz_sc_private(u32 p,u32 n){
 /* A corrupt disabled receipt fails closed, as the enabled implementation
  * does for an uncertain allocation identity. The common HR predicate still
  * excludes all HL/HI/mutator metadata and live managed heap extents. */
 if(_gz_sc_storage||_gz_scs_report[0]!=0x53435331u||_gz_scs_report[1]!=64||
  _gz_scs_report[2]!=1||_gz_scs_report[3]!=2||_gz_scs_report[4]!=SC_UNAVAILABLE||
  _gz_scs_report[5]!=sizeof(ScDirectory)||_gz_scs_report[6]||_gz_scs_report[7]||_gz_scs_report[8]||
  _gz_si_control[0]!=0x53494e31u||_gz_si_control[1]!=64||_gz_si_control[2]!=1||
  _gz_si_control[3]||_gz_si_control[4]!=SC_UNAVAILABLE||_gz_si_control[14]!=31)return 0;
 return hr_apart(p,n,(u32)&_gz_sc_storage,sizeof(_gz_sc_storage))&&
  hr_apart(p,n,(u32)_gz_scs_report,sizeof(_gz_scs_report))&&
  hr_apart(p,n,(u32)_gz_si_control,sizeof(_gz_si_control));
}
u32 _gz_sc_capture(u32 object,u32 owner){(void)object;(void)owner;return SC_UNAVAILABLE;}
u32 _gz_sc_validate(const void*image,u32 bytes){(void)image;(void)bytes;return SC_UNAVAILABLE;}
u32 _gz_sp_capture(u32 object,u32 owner,void*image,u32 capacity,u32*written){
 (void)object;(void)owner;(void)image;(void)capacity;(void)written;return SC_UNAVAILABLE;
}
u32 _gz_sp_validate(const void*image,u32 bytes){(void)image;(void)bytes;return SC_UNAVAILABLE;}
u32 _gz_sp_references(const void*image,u32 bytes,SpReferenceReport*out){
 (void)image;(void)bytes;(void)out;return SC_UNAVAILABLE;
}
