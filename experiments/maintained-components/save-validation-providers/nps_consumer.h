#ifndef GZ_NCB_NPS_CONSUMER_H
#define GZ_NCB_NPS_CONSUMER_H
typedef unsigned int NcbNpsU;
/* Native 029d0e08: one borrowed numeric pointer; 0,1310,1313 results.
 * A successful numeric span test supplies no physical access or stable lease.
 * Every subsequent word/halfword/byte load is volatile and sequential. */
NcbNpsU _gz_ncb_schema_c31_029d0e08(NcbNpsU record);
#endif
