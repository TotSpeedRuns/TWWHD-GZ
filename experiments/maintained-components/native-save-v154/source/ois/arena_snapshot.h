#ifndef GZ_ARENA_SNAPSHOT_PROTOTYPE_H
#define GZ_ARENA_SNAPSHOT_PROTOTYPE_H
typedef unsigned int AsU32;

/* Private algorithm prototype. There is deliberately no native game binding.
 * A future engine owner must establish these obligations using real leases;
 * setting this structure's flags does not establish any of them. */
enum { AS_ALLOCATORS=1, AS_ACTORS=2, AS_PARTICLES=4, AS_RESOURCES=8,
       AS_EXTERNAL_REFERENCES=16, AS_REQUIRED=31, AS_MAX_BLOCKS=256 };
enum { AS_OK=0, AS_ARGUMENT=1, AS_SCOPE=2, AS_LAYOUT=3, AS_LIMIT=4,
       AS_SPACE=5, AS_CORRUPT=6, AS_CHANGED=7, AS_BUSY=8,
       AS_APPLY_FAILED=9, AS_ROLLBACK_FAILED=10 };
typedef struct {
 AsU32 heap,start,span,heap_generation,session,token,obligations,reserved[3];
} AsDomain;

/* Staging buffer only: a caller publishes a new slot after success. Never
 * overwrite the user's previous committed slot while capturing this image.
 * Images contain allocator nodes and allocated payloads, not free payloads,
 * mutexes, thread state, GPU handles or an emulator-wide RAM/CPU state. */
AsU32 _gz_as_capture(const AsDomain*,void*staging,AsU32 capacity,AsU32*written);
AsU32 _gz_as_validate(const AsDomain*,const void*image,AsU32 bytes);
AsU32 _gz_as_restore(const AsDomain*,const void*image,AsU32 bytes,
                    void*undo,AsU32 undo_capacity,AsU32*undo_bytes);
#endif
