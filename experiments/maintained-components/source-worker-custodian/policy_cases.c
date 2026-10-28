/* Real exported scalar helpers only; no API/provider implementation. */
#include "source_worker_policy.h"
#include <stdint.h>
#include <stdio.h>
int main(void){uint32_t c,p,i,j;unsigned cases=0;GzSwcContext a={9,77,2,4,8},b;uint32_t *fields[5]={&b.request,&b.frame,&b.slot,&b.thread,&b.parent};const uint32_t expected[5]={9,77,2,4,8};
 for(c=0;c<6;c++)for(p=0;p<6;p++){unsigned want=(c==0&&p==1)||(c==1&&p==2)||(c==2&&p==3);if(!!gz_swc_work_next(c,p)!=want)return 1;++cases;}
 if(gz_swc_work_next(UINT32_MAX,0)||gz_swc_work_next(UINT32_MAX-1,UINT32_MAX))return 2;
 for(i=0;i<5;i++)for(j=0;j<5;j++){b=a;*fields[i]=j;if(!!gz_swc_context_same(&a,&b)!=(j==expected[i]))return 3;++cases;}
 b=a;if(!gz_swc_context_same(&a,&b))return 4;
 for(i=1;i<=3;i++){for(j=0;j<=4;j++){if(!!gz_swc_stage(i,0,j)!=(i==j))return 5;++cases;}if(gz_swc_stage(i,1,i))return 6;}
 for(i=0;i<625;i++){unsigned n=i,count=0,ok=1,k;for(k=0;k<4;k++){unsigned event=n%5;n/=5;if(!gz_swc_work_next(count,event)){ok=0;break;}count=event;}if(ok)return 7;++cases;}
 puts("scalar context/stage/order cases passed");return cases==701?0:8;}
