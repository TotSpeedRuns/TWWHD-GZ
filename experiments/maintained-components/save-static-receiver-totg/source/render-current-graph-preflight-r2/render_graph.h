#ifndef GZ_RENDER_CURRENT_GRAPH_PREFLIGHT_H
#define GZ_RENDER_CURRENT_GRAPH_PREFLIGHT_H
#include <stdint.h>
enum {RCG_OK=0,RCG_ARGUMENT=1,RCG_SCOPE=2,RCG_BUSY=3,RCG_CHANGED=4,
 RCG_UNSUPPORTED=5,RCG_GRAPH=6,RCG_LIMIT=7,RCG_ALIAS=8};
enum {RCG_MAX_NODES=4096,RCG_HASH=8192,RCG_FIELDS=32768,RCG_LOG=24576,
 RCG_ALLOC_PROBES=262144,RCG_HEADER_WORDS=64,RCG_SCRATCH_WORDS=RCG_HEADER_WORDS+RCG_HASH+RCG_FIELDS+RCG_LOG*2,
 RCG_SCRATCH_BYTES=RCG_SCRATCH_WORDS*4,RCG_READ_BUDGET=131072,RCG_HASH_PROBES=64};
/* Scratch is an actual writer4 OIS staging subrange for req/frame, with no
 * live capture data in the reserved slice. This component never reserves it.
 * Receipt words: magic,result,stage,badEA,bucket,nodes,graphWords,passes,req,frame,
 * hg,raw,rawBytes. It is advisory and cannot authorize a reset on its own.
 * No native reset is called by this component. */
uint32_t _gz_rcg_preflight(uint32_t req,uint32_t frame,uint32_t scratch,uint32_t bytes);
#endif
