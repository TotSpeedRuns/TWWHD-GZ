/* Input presentation only: use the original normalized f32 sample bits,
 * rather than the integer-thousandths trainer/ESS measurements. No libc,
 * heap allocation, gameplay write or floating-point control-state change. */
typedef unsigned int U;
typedef unsigned long long Q;
static char ivf_buffer[4][24];
char*_gz_ivf_text(U slot,U bits){
 U exp=(bits>>23)&255,m=bits&0x7fffff,sign=bits>>31,shift,n,whole,part,power,digit,i;
 Q scaled;char*p,*out;
 if(slot>3)return 0;
 out=p=ivf_buffer[slot];*p++=slot<2?'L':'R';*p++=(slot&1)?'Y':'X';*p++=':';*p++=' ';
 /* The native normalized range is[-1,1]. Keep modest over-range samples
  * visible up to2; invalid/nonfinite values get an explicit unavailable. */
 if((bits&0x7fffffffu)>0x40000000u){
  const char*s="Unavailable";while((*p++=*s++));return out;
 }
 if(sign)*p++='-';
 if(exp)m|=0x800000u;
 shift=exp?150-exp:149;
 scaled=(Q)m*1000000000u;
 n=shift>=64?0:(U)((scaled+((Q)1<<(shift-1)))>>shift);
 whole=n/1000000000u;part=n-whole*1000000000u;
 *p++=(char)('0'+whole);*p++='.';power=100000000u;
 for(i=0;i<9;i++){
  digit=part/power;*p++=(char)('0'+digit);part-=digit*power;power/=10;
 }
 while(p[-1]=='0'&&p[-2]!='.')--p;
 *p=0;return out;
}
