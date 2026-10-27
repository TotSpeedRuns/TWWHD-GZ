#ifndef GZ_MSS_VISIT_JOIN_H
#define GZ_MSS_VISIT_JOIN_H

typedef unsigned int MssJoinWord;

/* Diagnostic/control metadata only. The legacy MSS snapshot remains 50 words.
 * Retain both named frame readings; the accepted image aliases them to the
 * same native counter at 0x101FF558. They are not independent clocks.
 * A last-duration display value is never accepted as a fresh visit receipt.
 */
typedef struct MssVisitJoin {
    MssJoinWord magic, bytes, version;
    MssJoinWord attempt, epoch, actor, begin_native, begin_common, armed;
    MssJoinWord visit, visit_attempt, visit_epoch, visit_common, visit_native;
    MssJoinWord bound, completed_before;
    MssJoinWord completed_visit, completed_attempt, completed_frames;
    MssJoinWord completed_common, completed_native, consumed_attempt;
    MssJoinWord published, cancelled, reason;
    MssJoinWord observed_common, observed_epoch, observed_mode;
    MssJoinWord observed, gameplay_baseline, busy, lost;
} MssVisitJoin;

typedef char MssVisitJoinSize[(sizeof(MssVisitJoin) == 128) ? 1 : -1];
extern volatile MssVisitJoin _gz_msj_report;

void _gz_msj_begin(MssJoinWord *state, MssJoinWord native_frame);
void _gz_msj_observe(MssJoinWord frame, MssJoinWord mode, MssJoinWord epoch);
void _gz_msj_visit_begin(MssJoinWord frame, MssJoinWord epoch);
void _gz_msj_visit_complete(MssJoinWord frame, MssJoinWord epoch,
                            MssJoinWord settled_frames);

#endif
