#ifndef GZ_GAME_SOLID_HEAP_OWNER_H
#define GZ_GAME_SOLID_HEAP_OWNER_H
typedef unsigned int u32;
enum { GH_OK=0, GH_CLOSURE=69, GH_MAX_DEPTH=32, GH_MAX_NODES=4096 };
typedef struct {
 u32 valid,heap,parent,vtable,name,start,bytes,direction,head,tail,model,model_bytes;
} GhIdentity;
typedef char GhIdentitySize[sizeof(GhIdentity)==48?1:-1];
/* Private result, invalidated on every call and published valid-last. No
 * caller-chosen write address. Game/source memory is never written.
 * This identifies a current registration; it contains NO generation serial. */
extern volatile GhIdentity _gz_gh_identity;
/* Caller must already own the stable native lifetime/Management boundary.
 * Does not interpret model payload. On rejection, registered heap metadata
 * may alias that requested region and must be read to reject the relation.
 * Membership precedes ALL target-header reads. For the game actor's forward
 * solid heap, the complete model extent must be in an allocated front/back
 * region, outside its header/free interval and every descendant's declared
 * managed span plus evidenced class/base header prefix. Unknown subclass
 * storage and external metadata are not certified by this relation. */
u32 _gz_gh_solid(u32 heap,u32 model,u32 model_bytes);
/* A complete scan of both actual native root registries is required. A root
 * or child-link reference equal to heap returns denial BEFORE reading heap
 * or its sibling node. Absence alone is not a destruction/generation receipt;
 * pair with the owner's exact PID/birth/delete completion evidence. */
u32 _gz_gh_absent(u32 heap);
#endif
