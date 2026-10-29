#ifndef GZ_CURRENT_ONLY_PARK_SCALAR_H
#define GZ_CURRENT_ONLY_PARK_SCALAR_H
#include <stdint.h>
/* Scalar planning only: these grant no owner, API result or mutation scope. */
static inline uint32_t ipk_pause_introduced(uint32_t before)
{ return 3u & ~before; }
static inline uint32_t ipk_suffix_shape(uint32_t source,uint32_t current,uint32_t extra)
{ return source && source<=512 && extra && extra<=8 && current<=512 &&
         current>=source && current-source==extra; }
static inline uint32_t ipk_contact_extent(uint32_t used,uint32_t components,uint32_t offset,uint32_t bytes)
{ return (components==29 || components==30) && used<=0x200000u &&
         offset>=16u+8u*components && !(offset&3u) && !(bytes&3u) &&
         bytes>=128 && offset<=used && bytes<=used-offset; }
/* Journal retirement, not a supplied native/API outcome. Only untouched
 * PREPARED or complete finite RESTORED masks can reach actual token release. */
static inline uint32_t ipk_retirable(uint32_t phase,uint32_t error,uint32_t extra,
 uint32_t paused,uint32_t cut,uint32_t added,uint32_t unpaused)
{ uint32_t all;
 if(error || !extra || extra>8)return 0;
 all=(1u<<extra)-1u;
 if(phase==1)return !(paused|cut|added|unpaused);
 return phase==4 && paused==all && cut==all && added==all && unpaused==all;
}
#endif
