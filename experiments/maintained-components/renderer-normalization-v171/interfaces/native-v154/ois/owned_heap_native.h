#ifndef GZ_OWNED_HEAP_NATIVE_DIAGNOSTIC_H
#define GZ_OWNED_HEAP_NATIVE_DIAGNOSTIC_H
#include "owned_heap_image_version.h"
#include "arena_snapshot.h"
#include "owned_heap_save.h"
typedef unsigned int GhcU;
enum { GHC_OK=0,GHC_CAPTURED=2880,GHC_CONTEXT=2881,GHC_OBSERVER=2882,
 GHC_SPACE=2883,GHC_CHANGED=2884,GHC_OVERLAP=2885,GHC_RETAINED=2886,
 GHC_FORMAT=2887,GHC_MAGIC=0x41534831u,GHC_VERSION=4,GHC_MASK=2,
 GHC_HEAPS=4096,GHC_ROW=128,GHC_FIXED=3552,
 GHC_LEGACY_PROCESS_VERSION=6,GHC_LEGACY_COMMON_VERSION=7,GHC_LEGACY_PACKED_VERSION=8,GHC_PROCESS_VERSION=GZ_CURRENT_HEAP_IMAGE_VERSION,GHC_PROCESS_CAP=8192,GHC_PROCESS_HEADER=352,GHC_PROCESS_ROW=256,
 GHC_EXTERNAL_WRITERS=1,GHC_RUNTIME_INTERVALS=2,GHC_GPU=4,GHC_AUDIO=8,
 GHC_GLOBAL_ROOTS=16,GHC_RESOURCE_REFERENCES=32,GHC_REPLAY=64,
 GHC_COHERENCE=128,GHC_MISSING_ALL=255 };
/* AS v4 (WCT Source) and v5 (native Save producer) are opaque diagnostics, never AS v3 replay.
 * The old AsDomain obligations/session stay zero. No WORLD/READ certificate.
 * One raw union includes every byte of the chosen native allocator domain,
 * including its native control/header bytes and free/stale payload. */
typedef struct {
 GhcU magic,version,bytes,valid;AsDomain domain;
 GhcU diagnostic,missing,request,frame,hg,lease,parent,thread;
 GhcU cut,table,count,data,data_bytes,checksum,plan_checksum,root_mask,reserved[2];
} GhcHeader;
typedef struct {
 GhcU source,used,pin,identity,generation,revision,source_hash,slot;
 GhcU request,frame,thread,parent,roh_token,hg,lease,reserved;
} GhcSource;
/* First96 bytes are exact HlHeap64 + HmScope32. Remaining fields describe
 * their sole physical location in the raw union and AS control-word policy.
 * They do not authorize replay of mutexes, callbacks or SDK runtime state. */
typedef struct {GhcU heap[16],scope[8],raw_offset,raw_bytes,mutable_offset,
 mutable_count,mask,slot,header_bytes,reserved;} GhcHeapRow;
typedef char GhcHeader128[sizeof(GhcHeader)==128?1:-1];
typedef char GhcSource64[sizeof(GhcSource)==64?1:-1];
typedef char GhcRow128[sizeof(GhcHeapRow)==128?1:-1];
/* Pure interval/layout arithmetic used by the native capture and ROOT host
 * cases. These functions supply no heap ownership, readiness or native API. */
static inline GhcU ghc_policy_apart(GhcU a,GhcU n,GhcU b,GhcU m){return
 n<=0xffffffffu-a&&m<=0xffffffffu-b&&(a+n<=b||b+m<=a);}
static inline GhcU ghc_policy_contains(GhcU a,GhcU n,GhcU b,GhcU m){return
 n<=0xffffffffu-a&&m<=0xffffffffu-b&&b>=a&&m<=n&&b-a<=n-m;}
static inline GhcU ghc_policy_capacity(GhcU raw){GhcU fixed=GHC_FIXED+GHC_HEAPS*GHC_ROW;
 return raw&&!(raw&3)&&raw<=0x40000000u-fixed?fixed+raw:0;}
static inline GhcU ghc_policy_data(GhcU count){return count&&count<=GHC_HEAPS?
 (3536u+count*GHC_ROW+31u)&~31u:0;}
static inline GhcU ghc_policy_used(GhcU count,GhcU raw,GhcU capacity){GhcU at=ghc_policy_data(count);
 return at&&raw&&!(raw&3)&&capacity<=0x40000000u&&at<=capacity&&raw<=capacity-at?at+raw:0;}
/* AS7 appends a counted process directory before the complete raw union.
 * The count is a pre-reserve size proposal and is rechecked under real locks. */
static inline GhcU ghc_process_offset(GhcU heaps){return ghc_policy_data(heaps);}
static inline GhcU ghc_process_data(GhcU heaps,GhcU processes){GhcU at=ghc_process_offset(heaps);
 return at&&processes<=GHC_PROCESS_CAP?at+GHC_PROCESS_HEADER+processes*GHC_PROCESS_ROW:0;}
/* Temporary batch work is a private staging tail, never AS7 wire bytes.
 * Header32 +radix buckets1024 +4096 heap-cache20 +two PO-index arrays. */
static inline GhcU ghc_batch_bytes(GhcU processes){return processes<=GHC_PROCESS_CAP?
 (82976u+processes*8u+31u)&~31u:0;}
static inline GhcU ghc_process_capacity(GhcU raw,GhcU processes){GhcU fixed,scratch;
 if(processes>GHC_PROCESS_CAP)return 0;
 scratch=ghc_batch_bytes(processes);
 fixed=GHC_FIXED+GHC_HEAPS*GHC_ROW+GHC_PROCESS_HEADER+processes*GHC_PROCESS_ROW;
 return scratch&&raw&&!(raw&3)&&fixed<=0x40000000u-scratch&&raw<=0x40000000u-fixed-scratch?fixed+raw+scratch:0;}
GhcU _gz_wct_ghc_scope(GhcU request,GhcU frame,GhcU phase);
GhcU _gz_wct_ghc_hg(GhcU request,GhcU frame);
GhcU _gz_wct_ghc_source(GhcU request,GhcU frame,GhcSource*);
GhcU _gz_wct_ghc_private_apart(GhcU,GhcU);
GhcU _gz_gac_heap_reserve(GhcU,GhcU);
GhcU _gz_gac_heap_read(GhcU,GhcU);
GhcU _gz_gac_heap_finish(GhcU,GhcU,GhcU);
GhcU _gz_gac_heap_scope(GhcU,GhcU,GhcU);
GhcU _gz_gac_heap_private_apart(GhcU,GhcU);
GhcU _gz_ois_heap_reserve(GhcU,GhcU,GhcU);
GhcU _gz_ois_heap_read(GhcU,GhcU);
GhcU _gz_ois_heap_finish(GhcU,GhcU,GhcU);
GhcU _gz_ois_heap_stage_match(GhcU,GhcU,GhcU,GhcU);
GhcU _gz_ois_heap_apart(GhcU,GhcU,GhcU,GhcU);
GhcU _gz_ois_heap_private_apart(GhcU,GhcU);
GhcU _gz_ghc_propose(GhcU,GhcU);
GhcU _gz_ghc_refresh(GhcU,GhcU);
GhcU _gz_ghc_capture(GhcU,GhcU,GhcU,GhcU,GhcU*);
GhcU _gz_ghc_validate(const void*,GhcU,GhcU);
GhcU _gz_ghc_release(GhcU,GhcU);
GhcU _gz_ghc_private_apart(GhcU,GhcU);
GhcU _gz_ghc_all_private_apart(GhcU,GhcU);
extern volatile GhcU _gz_ghc_report[32];
#endif
