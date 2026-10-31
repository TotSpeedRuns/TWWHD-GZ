#ifndef GZ_GENERIC_FPC_GLOBAL_CUT_H
#define GZ_GENERIC_FPC_GLOBAL_CUT_H
#include "owned_heap_save.h"
typedef unsigned int FgcU;
enum {FGC_MAGIC=0x46474331u,FGC_VERSION=1,FGC_OK=0,FGC_ARGUMENT=1,
 FGC_SCOPE=2,FGC_SPACE=3,FGC_CHANGED=4,FGC_FORMAT=5,FGC_PROTECTED=6,
 FGC_MISSING_ALL=255,FGC_ROOTS_ONLY=3,FGC_COUNT=20,FGC_RECORD_BYTES=32,
 FGC_TABLE=128,FGC_DATA=768,FGC_PAYLOAD_BYTES=468,FGC_BYTES=1248,
 FGC_MODEL=0x776a8ec3u,FGC_STATIC_HASH=0x53c3d5b5u,
 FGC_CONTROL=1,FGC_POINTER=2,FGC_LIST=3,FGC_TREE=4,FGC_LIST_ARRAY=5,
 FGC_LAYER=6,FGC_DEVICE_POINTER=7,FGC_NO_WRITE_GRANT=1,
 FGC_EXTERNAL_DEVICE=2,FGC_HAS_REFERENCES=4};
typedef struct {
 FgcU magic,bytes,version,valid,missing,count,table,data;
 FgcU data_bytes,checksum,model,flags,static_hash,reserved[3];
 GhcSaveProducer producer;
} FgcHeader;
typedef struct {FgcU id,address,bytes,kind,data,items,item_bytes,flags;} FgcRecord;
typedef char FgcHeader128[sizeof(FgcHeader)==128?1:-1];
typedef char FgcRecord32[sizeof(FgcRecord)==32?1:-1];
/* Immutable layout keys; no descriptor is a native read/write permission. */
typedef struct {FgcU id,address,bytes,kind,offset,items,item_bytes,flags;} FgcLayout;
extern const FgcLayout _gz_fgc_layout[FGC_COUNT];
/* PRIVATE phase1 codec, not AS7/AS6 installation or a restore protocol.
 * All addresses in records are immutable layout keys, never followed by the
 * validator. Captured pointer VALUES remain opaque. This code writes only
 * caller-owned genuine OIS staging; it never writes any native global.
 * All20 fixed roots are retained, including nonempty/pending queues and the
 * external renderer pointer. Dynamic layers/nodes and render/device/world
 * references are not closed by this root cut. missing255 always remains.
 * Future AS7 integration must explicitly reserve/join this1248B region and
 * include it in whole-image accounting/seals; current AS6 is untouched. */
FgcU _gz_fgc_capture(FgcU request,FgcU frame,void *output,FgcU capacity);
/* Pure PRIVATE format/checksum validation. Valid means canonical captured
 * roots only, never native lifetime/coherence/writer/read permission. */
FgcU _gz_fgc_validate(const void *image,FgcU bytes);
/* Genuine same held cut comparison, for future before/after raw-domain join.
 * It compares fixed native roots only; success is still an observation. */
FgcU _gz_fgc_verify_held(FgcU request,FgcU frame,const void *image,FgcU bytes);
FgcU _gz_fgc_checksum(const void *image,FgcU bytes);
FgcU _gz_fgc_bytes(void);
#endif
