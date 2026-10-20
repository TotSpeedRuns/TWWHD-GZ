#ifndef GZ_NATIVE_SAVE_ROOT_OBSERVATION_H
#define GZ_NATIVE_SAVE_ROOT_OBSERVATION_H
typedef unsigned int SroU32;
enum { SRO_OK=0,SRO_NO_CHANGE=1,SRO_BUSY=5,SRO_DENIED=28,
 SRO_INVALID=70,SRO_CHANGED=71,SRO_EXHAUSTED=74,SRO_CORRUPT=75 };
enum { SRO_EMPTY=0,SRO_CONSTRUCTED=1,SRO_BOOT_READY=2,SRO_RETIRED=3 };
enum { SRO_CTOR_PUBLISHED=1,SRO_REUSE_RETURN=2,SRO_BOOT_COMPLETED=3,
 SRO_ROOT_DTOR_BEGIN=4,SRO_DIRECT_FREE_BEGIN=5 };
enum { SRO_MAGIC=0x53524f31u,SRO_VERSION=1,SRO_BYTES=64,
 SRO_CAPACITY=0x00f02d80u,SRO_SAVE_OFFSET=0x20u,SRO_SAVE_BYTES=0x12a0u };
/* A value record only. Root/heap/save are observed integers and MUST NOT be
 * followed merely because this kernel accepted an event or query. */
typedef struct {
 SroU32 magic,version,bytes,phase,generation,root,heap,capacity;
 SroU32 save,save_bytes,last_event,retire_reason,reserved[4];
} SroValue;
typedef char SroSize[(sizeof(SroValue)==SRO_BYTES)?1:-1];
/* Serialized, nonyielding fixed caller. No callback, native pointer read or
 * allocator is performed. Event origin is UNAUTHENTICATED in this component:
 * only separately implemented/reviewed fixed original ctor/dtor/boot wrappers
 * could make these observations evidence of a native event. Caller scalars
 * cannot issue mapping, allocation, source, world, ownership or pin authority.
 * expected_generation is the exact retained prior/current value. CTOR accepts
 * only EMPTY/RETIRED and issues next generation. REUSE changes nothing.
 * BOOT accepts the same constructed root. Matching retirement always revokes
 * eligibility without incrementing generation, so exhaustion cannot keep an
 * eligible root alive; the exact tombstone survives duplicate retirement. */
SroU32 _gz_sro_observe(SroU32 event,SroU32 root,SroU32 heap,SroU32 expected_generation);
/* out must be an actually mapped, caller-owned 64-byte writable object. Numeric
 * span/alignment/private-overlap guards are SHAPE ONLY, never mapping proofs.
 * required_phase is CONSTRUCTED (accepts either active phase) or BOOT_READY.
 * Failure leaves out unchanged. Success copies observations only, no native
 * authority or readable lease. A retired/stale receipt never queries0. */
SroU32 _gz_sro_query(SroU32 root,SroU32 generation,SroU32 required_phase,SroValue* out);
/* Literal denial even after BOOT_READY and a successful value query. */
SroU32 _gz_sro_native_admit(void);
#endif
