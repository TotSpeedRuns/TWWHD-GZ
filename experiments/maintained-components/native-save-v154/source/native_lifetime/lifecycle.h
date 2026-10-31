#ifndef GZ_ARENA_LIFECYCLE_H
#define GZ_ARENA_LIFECYCLE_H
typedef unsigned int AlU32;
/* Algorithm-only receipt. No actual engine/JPA/resource/OS ownership binding
 * exists here. This adapter may not be called by normal restore admission. */
typedef struct {AlU32 root,session,token,obligations;} AlDomain;
typedef struct {AlU32 heap,start,span,parent,kind,direction,flags,name;} AlRecipe;
typedef struct {AlRecipe heap;AlU32 unit_size,alignment,extra_gap,reserved;} AlUnitRecipe;
enum {AL_OK=0,AL_ARGUMENT=1,AL_SCOPE=2,AL_LAYOUT=3,AL_EXISTS=4,AL_NATIVE=5};
/* Exact original game factory 027ECD24 constructs a forward, unlocked
 * SolidHeap and clears numeric bit4 after SDK initialization at027ECD70/78.
 * No evidence admits low-policy Exp/Unit, locked Solid1, or reverse Solid0. */
AlU32 _gz_al_policy_supported(AlU32 kind,AlU32 direction,AlU32 flags);
AlU32 _gz_al_construct(const AlDomain*,const AlRecipe*);
/* UnitHeap uses the original fixed-pool constructor and initializer. */
AlU32 _gz_al_unit_construct(const AlDomain*,const AlUnitRecipe*);
AlU32 _gz_al_destroy(const AlDomain*,const AlRecipe*);
#endif
