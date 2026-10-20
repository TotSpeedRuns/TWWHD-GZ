#include "morth_setup.h"
typedef unsigned char U8;
static MhsU32 rd(const U8*p){return (MhsU32)p[0]<<24|(MhsU32)p[1]<<16|(MhsU32)p[2]<<8|p[3];}
static const MhsU32 fields[12][2]={{0x2ec,20},{0x300,20},{0x314,20},{0x328,6},{0x330,24},{0x370,12},{0x37c,12},{0x3a0,2},{0x3a4,4},{0x3a8,2},{0x544,12},{0x558,72}};
static MhsU32 finite(MhsU32 x){return (x&0x7f800000u)!=0x7f800000u;}
MhsU32 _gz_mhs_validate(const void*record,MhsU32 bytes){const U8*p=(const U8*)record;MhsU32 i,j,o,n,at=192,h=2166136261u;
 if(!p||bytes!=400)return 1;
 if(rd(p)!=0x4d485331u||rd(p+4)!=1||rd(p+8)!=bytes||rd(p+12)!=205||rd(p+16)!=2||
  rd(p+20)!=0x47616e6fu||rd(p+24)!=0x6e4b0000u||rd(p+28)||rd(p+32)!=12||rd(p+36)!=206||rd(p+44))return 2;
 for(i=0;i<12;i++){
  o=rd(p+48+12*i);n=rd(p+52+12*i);
  if(o!=fields[i][0]||n!=fields[i][1]||rd(p+56+12*i)!=at||n>bytes-at)return 3;
  /* Only numeric vectors use floating point; byte enums and signed timers
   * preserve the captured values exactly. Engine references are absent. */
  if(i<=2){for(j=0;j<12;j+=4)if(!finite(rd(p+at+j)))return 4;}
  if(i==4||i==5||i==6){for(j=0;j<n;j+=4)if(!finite(rd(p+at+j)))return 4;}
  if(i==11){for(j=0;j<12;j+=4)if(!finite(rd(p+at+j)))return 4;for(j=40;j<72;j+=4)if(!finite(rd(p+at+j)))return 4;}
  if(i==7&&(p[at]!=1||p[at+1]!=0))return 5;
  if(i==9&&p[at]!=1)return 5;
  if(i==10&&(p[at+3]!=0||p[at+4]!=2))return 5;
  at+=n;
 }
 for(i=at;i<bytes;i++)if(p[i])return 6;
 for(i=0;i<bytes;i++)h=(h^((i>=40&&i<44)?0:p[i]))*16777619u;
 return h==rd(p+40)?0:7;
}
