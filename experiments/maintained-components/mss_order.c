/* Pause type follows controller event order only. No player, speed, gain,
 * native pause duration or facing field is read by this classifier. */
typedef unsigned int U;
typedef long long S64;
typedef unsigned long long U64;

static int axis(U bits,int*out){
 U exp=(bits>>23)&255,m=bits&0x7fffff,shift,value;
 if((bits&0x7fffffffu)>0x40000000u)return 0;
 if(exp)m|=0x800000;
 shift=exp?134-exp:133;
 value=exp>=127?65536:shift>=32?0:m>>shift;
 *out=bits>>31?-(int)value:(int)value;return 1;
}
static U moved(U x,U y,U oldx,U oldy){
 int a,b,c,d;S64 dx,dy;
 if(!axis(x,&a)||!axis(y,&b)||!axis(oldx,&c)||!axis(oldy,&d))return 0;
 dx=a-c;dy=b-d;
 /* A vector deadband rejects normal stick jitter without consulting speed. */
 return (U64)(dx*dx+dy*dy)>=42954916ull;
}
static void result(U*s,U kind){s[36]=s[37]=kind;s[42]=4;}
static void first(U*s,const U*input,U frame){
 s[42]=1;s[43]=input[1];s[44]=input[2];s[45]=frame;
 s[36]=s[37]=0;s[47]=0;
}
U _gz_mss_order_input(U*s,const U*input,U frame){
 U held=input[0],trigger=input[13],edges=(held&~s[11])|(trigger&~s[48]);
 U phase=s[42],motion;
 s[48]=trigger;s[49]=frame;
 if(phase>4||(phase&&(U)(frame-s[45])>120)){
  s[42]=phase=0;s[36]=s[37]=0;
 }
 if(edges&8){
  if(phase==3){result(s,2);return edges;}
  if(phase==1){
   s[47]=2;
   if(moved(input[1],input[2],s[43],s[44]))result(s,4);
   else s[42]=2;
   return edges;
  }
  first(s,input,frame);return edges;
 }
 if((edges&0x4000)&&phase==1){
  s[47]=1;
  if(moved(input[1],input[2],s[43],s[44]))result(s,4);
  else s[42]=2;
  return edges;
 }
 if(phase!=1&&phase!=2)return edges;
 motion=moved(input[1],input[2],s[43],s[44]);
 if(motion){
  if(phase==1)s[42]=3;
  else {result(s,s[47]==1?3:1);s[46]=s[47];}
 }
 return edges;
}
