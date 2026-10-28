#include "source_worker_custodian.h"
#include "source_worker_policy.h"
typedef struct {
 GzSwcContext key;
 unsigned int phase,borrowed,error,work_count,pin,source,used,capacity;
 unsigned int identity,generation,raw_hash,row_seal,revision,seal;
} SwcState;
typedef char SwcStateSize[sizeof(SwcState)==76?1:-1];
/* Named private BSS keeps the source-owner objects/section anchors distinct. */
static SwcState swc __attribute__((section(".bss.gz_swc")));
static unsigned int swc_pending(void),swc_work_replaced(unsigned int);
static void swc_release_refused(void);
