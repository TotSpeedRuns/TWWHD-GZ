/* Actual native Plus-menu residency, independent of MSS input order and
 * gameplay buffer length. Called once at each completed game/UI frame.
 * All stores target trainer metadata. No menu or gameplay state is written. */
typedef unsigned int U;
extern volatile U _gz_tools_flags,_gz_state[],_gz_game_counter;
extern volatile U _gz_ps_ui_state_root,_gz_pbac_epoch;
extern unsigned char _gz_ps_play[],_gz_ps_menu[],_gz_ps_to_play[],_gz_ps_to_menu[];
extern volatile unsigned char _gz_loading_store;

/* mode: 0 interrupted/unknown, 1 gameplay, 2 settled native Plus menu,
 * 3 native ToMenu opening, 4 native ToPlay closing.
 * An enabling-in-menu sample cannot establish a complete visit. Observe
 * gameplay first. A missed frame, scene change or GZ menu cancels the live
 * measurement instead of publishing an incomplete duration. */
U _gz_mpd_report[16]={0x4d504431u,64,1};
void _gz_mpd_observe(U frame,U mode,U context){
 U *s=_gz_mpd_report,delta;
 if(s[3]&&frame==s[4])return;
 delta=frame-s[4];
 if(s[3]&&(delta!=1||context!=s[5])){s[6]=s[7]=s[8]=0;s[14]++;}
 s[3]=1;s[4]=frame;s[5]=context;s[13]=mode;
 if(mode==0){s[6]=s[7]=s[8]=0;return;}
 if(mode==1||mode==4){
  if(s[7]){s[9]=s[8];s[10]=1;s[11]++;}
  s[7]=s[8]=0;s[6]=mode==1;return;
 }
 if(mode==3){
  /* Opening is not a settled pause frame. Keep the gameplay baseline but
   * never bridge an already measured visit through another opening. */
  if(s[7])s[6]=s[7]=s[8]=0;
  return;
 }
 if(mode!=2){s[6]=s[7]=s[8]=0;return;}
 if(!s[6])return;
 s[7]=1;if(s[8]!=0xffffffffu)s[8]++;
}

void _gz_mpd_tick(void){U root,callback,mode=0;
 if((_gz_tools_flags&16u)&&!_gz_state[0]&&!_gz_loading_store){
  root=_gz_ps_ui_state_root;
  if(!(root&3u)&&root>=0x10000000u&&root<=0xbffffdd0u){
   callback=*(volatile U*)(root+12);
   if(callback==(U)_gz_ps_play)mode=1;
   else if(callback==(U)_gz_ps_menu)mode=2;
   else if(callback==(U)_gz_ps_to_menu)mode=3;
   else if(callback==(U)_gz_ps_to_play)mode=4;
  }
 }
 _gz_mpd_observe(_gz_game_counter,mode,_gz_pbac_epoch);
}

static char line[64];
static char *append(char*p,const char*s){while(*s)*p++=*s++;return p;}
const char *_gz_mpd_text(void){U *s=_gz_mpd_report,n;char *p=line,digits[10];U i=0;
 if(!s[7]&&!s[10])return "Pause frames: -- (2f Insta, 3f target)";
 n=s[7]?s[8]:s[9];p=append(p,s[7]?"|| Pause: ":"> Last pause: ");
 do{digits[i++]=(char)('0'+n%10);n/=10;}while(n);
 while(i)*p++=digits[--i];p=append(p,"f");
 n=s[7]?s[8]:s[9];
 if(n==2)p=append(p," (Insta)");
 else if(n==3)p=append(p," (3f target)");
 *p=0;return line;
}
