#ifndef GZ_HELD_HEAP_ROUNDTRIP_H
#define GZ_HELD_HEAP_ROUNDTRIP_H

typedef unsigned int GhwU;
enum {
    GHW_REFUSED = 2894,
    GHW_ROUNDTRIP = 2895,
    GHW_RECOVERED = 2896,
    GHW_CPU_COMMITTED = 2897,
    GHW_RETAINED = 2886,
    GHW_PREPARE = 1,
    GHW_APPLY = 2,
    GHW_INVERSE = 3,
    GHW_VERIFY = 4
};

/* Private CPU-retention experiment: verify heap/globals and actual
 * generic lifetime tables before commit; independently undo failed
 * application. Missing255 and external/device obligations remain.
 * Same-scene gameplay must be tested before any wider claim.
 * Historical function name is retained for the internal ABI. */
GhwU _gz_ghw_roundtrip_held(GhwU request, GhwU frame);
GhwU _gz_ghw_scope(GhwU request, GhwU frame, GhwU phase);
GhwU _gz_ghw_private_apart(GhwU pointer, GhwU bytes);
GhwU _gz_ghs_apply_scope(GhwU request, GhwU frame);
GhwU _gz_ghc_held_domain(GhwU request, GhwU frame, GhwU pointer, GhwU bytes);
extern volatile GhwU _gz_ghw_report[32];

#endif
