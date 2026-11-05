#include "nps_consumer.h"
typedef NcbNpsU u32;
typedef unsigned short u16;
typedef unsigned char u8;
#define R32(p,o) (*(const volatile u32*)((p)+(o)))
#define R16(p,o) (*(const volatile u16*)((p)+(o)))
#define R8(p,o) (*(const volatile u8*)((p)+(o)))
extern u32 _gz_ncb_nbs_span(u32 pointer,u32 bytes);

u32 _gz_ncb_schema_c31_029d0e08(u32 p){
 u32 h,i,a,b;
 if(!_gz_ncb_nbs_span(p,256u))return 1310u;
 if(R32(p,0)!=0x4e505331u||R32(p,4)!=256u||R32(p,8)!=1u||R32(p,12)!=1u||
    !R32(p,16)||R32(p,16)>0xfffffffdu||!R32(p,20)||
    !R32(p,24)||R32(p,24)>0xfffffffdu||R32(p,16)==R32(p,24)||
    !R32(p,28)||!R32(p,32)||R32(p,36)>4095u||R32(p,40)>4095u||
    R32(p,44)!=317u||(R32(p,48)!=0u&&R32(p,48)!=3u)||R32(p,52)!=64u)return 1310u;
 {u32 saved=R32(p,56);h=0x811c9dc5u;
  for(i=0;i<256u;i++){
   if((i>=12u&&i<16u)||(i>=56u&&i<60u))h*=0x01000193u;
   else h=(h^R8(p,i))*0x01000193u;
  }
  if(saved!=h)return 1310u;
 }
 if(R32(p,60)!=31u||R32(p,64)!=4u||R32(p,68)!=2u||R32(p,72)!=1u||R32(p,76)!=1u||
    R32(p,80)||R32(p,84)||!R32(p,88)||R32(p,88)>32767u||
    !R32(p,92)||R32(p,92)>4096u||R32(p,96)>4u||
    (R32(p,100)!=1u&&R32(p,100)!=2u))return 1310u;
 if((R32(p,104)&0x7fffffffu)>=0x7f800000u||R32(p,108)>255u||R32(p,112)>255u||
    R32(p,116)||R32(p,120)||R32(p,124)||
    (R32(p,128)&0x7fffffffu)>=0x7f800000u||
    (R32(p,132)&0x7fffffffu)>=0x7f800000u)return 1310u;
 a=R16(p,136);b=R16(p,138);if(a>b)return 1310u;
 a=R16(p,138);b=R32(p,88);if(a>=b)return 1310u;
 a=R16(p,140);b=R16(p,138);if(a>b)return 1310u;
 if(R8(p,142)>4u||R8(p,143)>3u)return 1310u;
 for(i=144u;i<160u;i+=4u)if((R32(p,i)&0x7fffffffu)>=0x7f800000u)return 1310u;
 a=R32(p,160);b=R32(p,96);if(a!=b)return 1310u;
 a=R32(p,164);if((a&0x7fffffffu)>=0x7f800000u&&a!=0x7f800000u)return 1310u;
 if(R16(p,170)!=2u||R16(p,172)>1u||R16(p,174)!=2u)return 1310u;
 a=R16(p,176);b=R32(p,92);if(a>=b)return 1310u;
 if(R16(p,178)||R8(p,188)>1u||R8(p,189)||R8(p,190)||R8(p,191))return 1310u;
 if(R32(p,180)-1u<=0xfffffffdu||R32(p,184)-1u<=0xfffffffdu)return 1310u;
 for(i=0;i<4u;i++){
  if(R32(p,96)>i){
   if(R32(p,192u+12u*i)!=0x7f800000u){
    if((R32(p,192u+12u*i)&0x7fffffffu)>=0x7f800000u||
       (R32(p,196u+12u*i)&0x7fffffffu)>=0x7f800000u||
       (int)R32(p,192u+12u*i)<0)return 1313u;
    a=R32(p,196u+12u*i);b=R32(p,192u+12u*i);
    if(a<=b||R32(p,200u+12u*i)>4095u)return 1313u;
    continue;
   }
  }else if(R32(p,192u+12u*i))return 1310u;
  if(R32(p,196u+12u*i)||R32(p,200u+12u*i))return 1310u;
 }
 for(i=240u;i<256u;i++)if(R8(p,i))return 1310u;
 return 0u;
}
