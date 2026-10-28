/* Portable tests of the actual order classifier; no game or OS API model.
 * The join notification is void and has no classification result. Its stub
 * merely counts notifications; native visit/IRQ/frame integration is external.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mss_visit_join.h"
extern unsigned _gz_mss_order_input(unsigned *,const unsigned *,unsigned,unsigned);
static unsigned state[50],frame,notifications,cases;
void _gz_msj_begin(unsigned *s,unsigned f){(void)s;(void)f;notifications++;}
static unsigned bits(float f){unsigned n;assert(sizeof(n)==sizeof(f));memcpy(&n,&f,sizeof(n));return n;}
static void poll_at(unsigned held,unsigned trigger,unsigned release,float y,unsigned tick){
 unsigned input[14]={0};input[0]=held;input[2]=bits(y);input[13]=trigger;
 _gz_mss_order_input(state,input,tick,release);
 /* The real outer caller updates previous-held word11 after this call. */
 state[11]=held;
 /* Contract of the external legacy display consumer: unknown0 preserves37. */
 if(state[36])state[37]=state[36];
}
static void poll(unsigned held,unsigned trigger,unsigned release,float y){poll_at(held,trigger,release,y,++frame);}
static void reset(unsigned display){memset(state,0,sizeof(state));frame=0;state[37]=display;}
static void begin(void){poll(8,8,0,1);poll(0,0,8,1);assert(state[42]==1&&state[36]==0);}
static void dwell(unsigned n,unsigned display){unsigned i;for(i=0;i<n;i++){poll(0,0,0,1);assert(state[42]==1&&state[36]==0&&state[37]==display);}}
static void complete(unsigned kind){
 if(kind==2){poll(0,0,0,-1);assert(state[42]==3);poll(8,8,0,-1);poll(0,0,8,-1);}
 else if(kind==1){poll(8,8,0,1);assert(state[42]==2);poll(0,0,8,1);poll(0,0,0,-1);}
 else if(kind==3){poll(0x4000,0x4000,0,-1);poll(0,0,0x4000,-1);}
 else{assert(kind==4);poll(0x4000,0x4000,0,1);assert(state[42]==2);poll(0,0,0x4000,1);poll(0,0,0,-1);kind=3;}
 assert(state[42]==4&&state[36]==kind&&state[37]==kind);
}
int main(void){
 const unsigned delays[]={0,1,2,3,10,120};unsigned i,k;
 for(i=0;i<sizeof(delays)/sizeof(delays[0]);i++)for(k=1;k<=4;k++){
  reset(2);begin();assert(state[37]==2);dwell(delays[i],2);complete(k);
  {unsigned last=state[37];begin();assert(state[36]==0&&state[37]==last);dwell(3,last);}
  cases++;
 }
 reset(2);poll(0x4000,0x4000,0,-1);assert(state[42]==0&&state[37]==2);cases++;
 reset(3);begin();poll(0x4008,0x4008,0,1);assert(state[42]==4&&state[36]==0&&state[37]==3);cases++;
 reset(3);begin();poll(0x4008,0x4008,0,-1);assert(state[42]==4&&state[36]==0&&state[37]==3);cases++;
 reset(2);begin();poll(8,8,0,-1);assert(state[42]==4&&state[36]==0&&state[37]==2);cases++;
 reset(1);begin();poll(0,0,0,-1);poll(0x4000,0x4000,0,-1);assert(state[42]==4&&state[36]==0&&state[37]==1);cases++;
 reset(2);begin();poll(0,0,0,.98f);assert(state[42]==1);poll(8,8,0,.98f);assert(state[42]==2);poll(0,0,8,.98f);poll(0,0,0,-1);assert(state[37]==1);cases++;
 reset(3);poll(8,8,0,1);poll(8,0,0,1);poll(8,8,0,1);assert(state[42]==1&&state[37]==3);poll(8,8,8,1);assert(state[42]==2);poll(0,0,8,-1);assert(state[37]==1);cases++;
 reset(2);state[42]=99;poll(0,0,0,1);assert(state[42]==0&&state[36]==0&&state[37]==2);cases++;
 reset(3);poll(8,8,0,-1);poll(0,0,8,-1);assert(state[42]==1&&state[37]==3);poll(8,8,0,-1);poll(0,0,8,1);assert(state[37]==1);cases++;
 assert(notifications);printf("MSS classifier cases: %u passed\n",cases);return 0;
}
