/* Private exact first GamePad sample, consumed before ordinary item dispatch.
 * No controller argument is native owner authority. One capture per process. */
typedef unsigned int U;
extern volatile U _gz_gac_request;
static U gai_latch;
U _gz_gai_sample(U held){
 if(gai_latch&&gai_latch!=0x80f0u)return held==0x80f0u;
 if(gai_latch&&(held&gai_latch)!=gai_latch)gai_latch=0;
 if(held!=0x80f0u)return 0;
 if(!gai_latch){gai_latch=held;__asm__ volatile("sync":::"memory");_gz_gac_request=1;}
 return 1;
}
U _gz_gai_view(U address,U count,U maximum,U channel){
 U end,request=(U)&_gz_gac_request,latch=(U)&gai_latch;
 if(channel||!count||count>32||!maximum||maximum>32||count>maximum||
  (address&3u)||address<0x10000000u||address>0xbffe0000u-172u)return 0;
 end=address+172u;
 if((address<request+4u&&request<end)||(address<latch+4u&&latch<end))return 0;
 return _gz_gai_sample(*(const volatile U*)address);
}
