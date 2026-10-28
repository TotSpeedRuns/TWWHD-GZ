/* Pure scalar predicates only; no native API, owner, pin or lease fixture. */
#include <stdio.h>
#include "../source-worker-custodian/source_worker_custodian.h"
#include "source_pin_format.h"
#include "source_custodian_format_policy.h"
int main(void)
{
    static const unsigned int phases[]={0,1,2,3,4,0xffffffffu};
    static const unsigned int borrowers[]={0,1,2,3,4,0xffffffffu};
    static const unsigned int counts[]={0,1,2,3,4,0xfffffffeu,0xffffffffu};
    static const unsigned int pins[]={0,1,2,3,0xffffffffu};
    unsigned int a,b,c,m,k,p,want,cases=0;
    if(SWC_READY!=1||SWC_ADMITTED!=2||SWC_ADMISSION!=1||SWC_INSTALL!=2||SWC_WORK_BFLOWER!=3)return 1;
    for(a=0;a<6;a++)for(b=0;b<6;b++)for(c=0;c<7;c++){
        want=(phases[a]==1&&borrowers[b]==1&&counts[c]==0)||
             (phases[a]==2&&borrowers[b]==2&&counts[c]<=3);
        if(gz_scf_stage(phases[a],borrowers[b],counts[c])!=want)return 2;
        cases++;
    }
    for(a=0;a<5;a++)for(b=0;b<5;b++)for(m=0;m<64;m++)for(k=0;k<8;k++){
        p=pins[b];
        if(!p||k<1||k>6||(!pins[a]&&m)||pins[a]>p)want=SFM_REFUSE;
        else if(pins[a]!=p)want=SFM_MINT;
        else want=(m&(1u<<(k-1)))?SFM_REUSE:SFM_MINT;
        if(gz_sfm_plan(pins[a],m,p,k)!=want)return 3;
        cases++;
    }
    if(gz_sfm_plan(1,64,1,1)!=SFM_REFUSE||gz_sfm_plan(1,0xffffffffu,1,1)!=SFM_REFUSE)return 4;
    cases+=2;
    printf("scalar cases passed: %u\n",cases);
    return 0;
}
