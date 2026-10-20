#ifndef GZ_VOID_CORRESPONDENCE_H
#define GZ_VOID_CORRESPONDENCE_H
typedef unsigned int u32;
enum { VR_CAPACITY=512, VR_WORLD_MAGIC=0x56525731u, VR_PLAN_MAGIC=0x56525031u };
enum VrResult {
    VR_READY=0, VR_BAD_INPUT=1, VR_NOT_READY=2, VR_DIFFERENT_SCENE=3,
    VR_DUPLICATE_ID=4, VR_BAD_PLAYER=5, VR_UNMATCHED=6,
    VR_AMBIGUOUS=7, VR_PARENT_CHANGED=8, VR_INPUT_CHANGED=9,
    VR_BAD_OUTPUT=10
};
enum VrKind { VR_SURVIVING_ACTOR=1, VR_PLACEMENT_CANDIDATE=2, VR_OWNED_PLAYER=3 };
/* This is immutable identity evidence, not a replay of native actor pointers.
 * home[5] covers actor_place at +0x2ec..+0x2ff exactly. The adjacent old/current
 * position is deliberately not part of constructor identity. */
typedef struct {
    u32 actor, pid, profile, parameters, set_id, parent_pid, home[5], init_state;
} VrActor;
typedef struct {
    u32 magic, bytes, version, complete, count, frame, epoch;
    u32 stage0, stage1, room, layer, link_actor, link_pid;
    u32 queue_head, node_count, quiet, session, sequence, checksum, reserved;
    VrActor actors[VR_CAPACITY];
} VrWorld;
typedef struct {
    u32 source_index, live_index, source_pid, live_pid, live_actor, kind;
} VrPair;
typedef struct {
    u32 magic, bytes, version, valid, result, bad_row, count, source_sequence;
    u32 live_sequence, source_hash, live_hash, source_epoch, live_epoch;
    u32 resource_validation_required, external_zero_parent_count, reserved;
    VrPair pairs[VR_CAPACITY];
} VrPlan;
u32 _gz_vr_world_hash(const VrWorld *world);
u32 _gz_vr_correspond(const VrWorld *source, const VrWorld *live, VrPlan *output);
/* Each component uses this plan to resolve its fresh identity, then validates
 * resource/model/ownership/collision bindings before replaying typed state. */
#endif
