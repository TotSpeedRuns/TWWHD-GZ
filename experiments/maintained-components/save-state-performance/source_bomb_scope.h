#ifndef GZ_SOURCE_BOMB_SCOPE_H
#define GZ_SOURCE_BOMB_SCOPE_H
/* Purpose alone grants no lease. The real SO/OSA pin remains authoritative. */
typedef struct {
    unsigned int entered, kind, request, frame, thread, poison, seen_plan;
} GzSourceBombCall;
extern GzSourceBombCall _gz_sbp_call;
enum { SBP_NONE=0, SBP_CHECK=1, SBP_REFUSE=2 };
/* Exact existing owner.h bound, checked by source staging. */
#define SBP_ROH_MAX_SERVICES 8192u
unsigned int _gz_bfm_source_digest_scope(void);
unsigned int _gz_source_reuse_scope(void);
unsigned int _gz_som_verify_bomb_final(unsigned int, unsigned int);

/* Scalar decisions only, never an authenticated owner or native result. */
static inline unsigned int gz_sbp_record_plan(unsigned int entered,
        unsigned int kind, unsigned int poison, unsigned int seen)
{
    if (!entered) return SBP_NONE;
    return entered==1 && (kind==1 || kind==2) && !poison && seen<=1
        ? SBP_CHECK : SBP_REFUSE;
}
static inline unsigned int gz_sbp_owner_phase(unsigned int phase,
        unsigned int hold, unsigned int mutated, unsigned int error,
        unsigned int open, unsigned int service)
{
    if (hold!=1 || error || open!=1) return 0;
    if (phase==1) return !mutated && service<=1;
    return phase==2 && mutated==1 &&
        (service==0 || (service>=2 && service<=9));
}
static inline unsigned int gz_sbp_deadline(unsigned int start,
        unsigned int limit, unsigned int frame, unsigned int last)
{
    return limit && limit<=120 && frame-start<=limit && last==frame;
}
static inline unsigned int gz_sbp_plan_phase(unsigned int valid,
        unsigned int seen)
{
    if (!valid) return seen ? SBP_REFUSE : SBP_NONE;
    return valid==1 ? SBP_CHECK : SBP_REFUSE;
}
static inline unsigned int gz_sbp_combine(unsigned int wct,
        unsigned int bomb)
{
    if (wct>1 || bomb>SBP_REFUSE || bomb==SBP_REFUSE ||
        (wct && bomb==SBP_CHECK)) return SBP_REFUSE;
    return wct ? SBP_CHECK : bomb;
}
#endif
