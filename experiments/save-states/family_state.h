#ifndef GZ_POT_FAMILY_STATE_H
#define GZ_POT_FAMILY_STATE_H
typedef unsigned int u32;
enum {PF_BYTES=2048,PF_HEADER=128,PF_SCALAR_CAP=1024,PF_REFS=1152,PF_AUDIT=1200,PF_CB=1248,
      PF_REF_COUNT=10,PF_AUDIT_COUNT=9,PF_CB_COUNT=7,PF_MAGIC=0x50465332u};
enum {PF_OK=0,PF_RANGE=220,PF_ACTOR=221,PF_FORMAT=222,PF_CHECKSUM=223,
      PF_FLOAT=224,PF_CONTEXT=225,PF_BINDING=226,PF_DEPENDENCY=227,
      PF_RECIPE=228,PF_DIRTY=229,PF_ROLLED_BACK=230};
enum {PF_RELATED_PID=1,PF_EMITTER=2,PF_EXTERNAL_CALLBACK=4,PF_BRK=8,
      PF_LINK_CARRY=16,PF_COLLISION_GRAPH=32,PF_CALLBACK_CLASS=64,
      PF_EXTERNAL_DRAW_MODEL=128,PF_PARENT=256};
extern volatile u32 _gz_pt_epoch;
extern const unsigned char _gz_pf_private_begin[],_gz_pf_private_end[];
u32 _gz_pmr_model(u32 actor,u32 pid,u32 epoch);
/* Capture all16 variants and8 modes, including dependencies. Output is a
 * typed packet, NOT proof of complete whole-world replay coverage. Callback
 * and collider external addresses are audit-only and never dereferenced. */
u32 _gz_pf_capture(u32 actor,u32 output,u32 pid,u32 epoch);
u32 _gz_pf_packet_check(u32 packet);
/* Scalar replay only. Caller owns a stable non-yielding world/actor/resource,
 * collision and particle Management boundary plus genuine queued identity
 * and allocation-generation/lifetime authority. Local dependency0 does not
 * prove global reverse-reference closure. Additional participant
 * coverage is required before normal save-state admission/publication.
 * External relationships/effects/BRK/carry remain explicit dependencies.
 * No native query/model/cylinder refresh is performed by this scalar leaf. */
u32 _gz_pf_preflight(u32 actor,u32 packet,u32 pid,u32 epoch);
u32 _gz_pf_apply_scalars(u32 actor,u32 packet,u32 pid,u32 epoch);
u32 _gz_pf_field_count(void);
u32 _gz_pf_field_word(u32 index);
u32 _gz_pf_packed_bytes(void);
#endif
