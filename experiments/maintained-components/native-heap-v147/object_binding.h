#ifndef GZ_OBJECT_LIFETIME_BINDING_H
#define GZ_OBJECT_LIFETIME_BINDING_H
typedef unsigned int ObU32;
/* A metadata identity, not a pointer lease or a restore permission. Consumers
 * must retain their actual game-thread/world hold and constructor ownership.
 * No native byte is read by either API, including when the saved allocation
 * has expired, its heap has reset, or the address has been reused. */
typedef struct {
 ObU32 magic,bytes,version,query,query_bytes,pointer,capacity,generation;
 ObU32 heap,heap_generation,epoch,kind,block,missing,session,reserved;
} ObBinding;
enum {OB_OK=0,OB_ARGUMENT=1,OB_OBSERVER=2,OB_ABSENT=3,OB_CHANGED=4,OB_NEWER=5};
ObU32 _gz_ob_capture(ObU32 pointer,ObU32 bytes,ObBinding*output);
ObU32 _gz_ob_validate(const ObBinding*binding);
/* Same immutable binding plus strict allocation/heap birth-before-source-frame
 * proof under the observer lock. Same-frame births are ambiguous and refused.
 * The source frame must be nonzero and no later than the current monotonic
 * native counter (within its half-range). No shared observation output is
 * exposed, no native actor/model bytes are read, and no restore authority is
 * granted. The caller must independently authenticate the source frame,
 * scene/session, native world hold and actor/process identity. */
ObU32 _gz_ob_validate_before(const ObBinding*binding,ObU32 source_frame);

#endif
