#ifndef GZ_SOURCE_CUSTODIAN_FORMAT_POLICY_H
#define GZ_SOURCE_CUSTODIAN_FORMAT_POLICY_H
/* Scalar stages only: real custody/lease/context validation is mandatory. */
static inline unsigned int gz_scf_stage(unsigned int phase,
        unsigned int borrowed,unsigned int work_count)
{
    return (phase==SWC_READY && borrowed==SWC_ADMISSION && !work_count) ||
        (phase==SWC_ADMITTED && borrowed==SWC_INSTALL &&
         work_count<=SWC_WORK_BFLOWER);
}
#endif
