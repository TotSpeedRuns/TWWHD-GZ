#ifndef GZ_SOURCE_PIN_FORMAT_H
#define GZ_SOURCE_PIN_FORMAT_H
/* Saved-source checks only. Every installed/CURRENT comparison stays fresh. */
enum { SFM_FULL=0, SFM_REUSE=1, SFM_REFUSE=2, SFM_MINT=3, SFM_CHECK=4 };
enum { SFM_PF=1, SFM_BC=2, SFM_LG=3, SFM_B294=4, SFM_SAND=5, SFM_CGA=6 };
typedef struct { unsigned int pin,mask; } GzSourceFormatMemo;
extern GzSourceFormatMemo _gz_sfm_memo;
unsigned int _gz_som_source_format_check(unsigned int,unsigned int,unsigned int);

/* Genuine inactive SOM is the original full-validation phase, including
 * internal SO decode/installation before the retained operation is issued.
 * An active/lost pin or inconsistent/fatal SOM must never become FULL. */
static inline unsigned int gz_sfm_som_phase(unsigned int active,unsigned int pin,
        unsigned int token,unsigned int fatal)
{
    if (!active && !pin && !token && !fatal) return SFM_FULL;
    return active==1 && pin && !fatal ? SFM_CHECK : SFM_REFUSE;
}
/* Retired serial fields may remain while inactive. A live SO lease requires
 * its actual nonzero serial and published adapter, never a lost-pin fallback. */
static inline unsigned int gz_sfm_full_lease_phase(unsigned int active,
        unsigned int serial,unsigned int adapter_active)
{
    return (!active && !adapter_active) ||
           (active==1 && serial && adapter_active==1);
}
/* Detached scalar policy; this does not authenticate a lease or mint a bit. */
static inline unsigned int gz_sfm_plan(unsigned int old_pin,unsigned int mask,
        unsigned int pin,unsigned int kind)
{
    if (!pin || kind<SFM_PF || kind>SFM_CGA || (mask&~63u) ||
        (!old_pin && mask) || old_pin>pin) return SFM_REFUSE;
    if (old_pin!=pin) return SFM_MINT;
    return mask&(1u<<(kind-1)) ? SFM_REUSE : SFM_MINT;
}
/* The immutable directory is still checked, and variable CURRENT reads keep
 * the exact PREPARE candidate's bounded source/bank extent policy. */
static inline unsigned int gz_sfm_bank_bounds(unsigned int source,unsigned int used,
        unsigned int bank,unsigned int bytes,unsigned int minimum,unsigned int maximum)
{
    unsigned int offset;
    if ((source|used|bank|bytes)&3u || source<0x10000000u ||
        source>=0xbffe0000u || !used || used>0xbffe0000u-source ||
        bytes<minimum || bytes>maximum || bank<source) return 0;
    offset=bank-source;
    return offset<=used && bytes<=used-offset;
}
#endif
