/* Additive exact artifact-role recognition only. No filesystem/admission model.
 * p/n borrow a completed genuine Native SHA256 string at the pinned caller.
 */
#include "verified_roles.h"
unsigned _gz_setup_role_known(const char *p,unsigned n,unsigned role){
 unsigned i,j;const char *wanted;
 if(!p||n!=64||role>1)return 0;
 for(i=0;i<20;i++){
  wanted=role?gz_roles[i].title:gz_roles[i].rpx;
  for(j=0;j<64;j++)if(p[j]!=wanted[j])break;
  if(j==64)return 1;
 }
 return 0;
}
