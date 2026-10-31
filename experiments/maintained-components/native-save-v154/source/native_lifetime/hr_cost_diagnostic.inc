/* Private cost observations only. The existing BRR report tail17..23 has
 * no admission consumer. All callers below hold the original HL metadata lock;
 * no lock, SDK call, callback, source read or authority query is added here.
 * Rows are per actual WCT request/frame {raw TB ticks32, completed calls32},
 * then sticky flags. Existing WCT report3/4 identify the measured invocation.
 * Read after its completed command. No assumed TB frequency/seconds. */
extern volatile u32 _gz_brr_report[24];
extern volatile u32 _gz_wct_report[128],_gz_wct_request,_gz_wct_ack,_gz_pbac_frame;
typedef unsigned long long HrpTicks;
typedef struct {u32 request,frame;} HrpInvocation;
static u32 hrp_window(HrpInvocation*out){u32 request=_gz_wct_request,ack=_gz_wct_ack,frame=_gz_pbac_frame;
 if(!request||ack==0xffffffffu||request!=ack+1||!frame||
  _gz_wct_report[0]!=0x57435431u||_gz_wct_report[1]!=512||_gz_wct_report[2]!=1||
  _gz_wct_report[3]!=request||_gz_wct_report[4]!=frame||
  request!=_gz_wct_request||ack!=_gz_wct_ack||frame!=_gz_pbac_frame)return 0;
 out->request=request;out->frame=frame;return 1;
}
static u32 hrp_same(const HrpInvocation*key){HrpInvocation now;
 return hrp_window(&now)&&now.request==key->request&&now.frame==key->frame;
}
static HrpTicks hrp_clock(void){u32 hi,lo,again;
 do{__asm__ volatile("mftbu %0\n\tmftb %1\n\tmftbu %2"
  :"=r"(hi),"=r"(lo),"=r"(again)::"memory");}while(hi!=again);
 return ((HrpTicks)hi<<32)|lo;
}
static void hrp_note(u32 category,const HrpInvocation*key,HrpTicks started){HrpTicks elapsed;volatile u32*row;u32 flags,value;
 elapsed=hrp_clock()-started;
 /* Diagnostic loss/corruption must never change the real operation. */
 if(category>2||!hrp_same(key)||_gz_hl_control.busy!=1||_gz_brr_report[0]!=0x42525233u||
  _gz_brr_report[1]!=96||_gz_brr_report[2]!=3)return;
 row=_gz_brr_report+17+category*2;flags=_gz_brr_report[23];value=(u32)elapsed;
 if((elapsed>>32)||value>0xffffffffu-row[0]){row[0]=0xffffffffu;flags|=1u<<category;}
 else row[0]+=value;
 if(row[1]==0xffffffffu)flags|=1u<<(category+3);else row[1]++;
 _gz_brr_report[23]=flags;
}
