/* Actual full owner/CLEAN/OBSERVE source view. No actor creation or replay. */
#include "live_layer_binding.h"
extern volatile u32 _gz_roh_owner[60],_gz_roh_live_frame,_gz_slots_selected;
extern volatile u32 _gz_state,_gz_tools_flags,_gz_pending,_gz_route_choice;
extern volatile u8 _gz_loading_store,_gz_somp_event;
extern u32 _gz_slots_install(u32),_gz_slots_store(u32),_gz_roh_begin(u32,u32),_gz_roh_user_guard(void);
extern u32 _gz_roh_service_begin(u32,u32),_gz_roh_service_end(u32,u32,u32),_gz_roh_abort_clean(u32),_gz_osa_pin_id(void);
extern u32 _gz_bsr_begin(u32,u32),_gz_bsr_end(u32),_gz_bsr_read_world(u32,u32);
extern u32 _gz_srd_kind(u32,u32),_gz_srd_serial(u32,u32),_gz_srd_read_world(u32,u32),_gz_srd_read_bank(u32,u32),_gz_srd_read_layers(u32,u32);
volatile u32 _gz_somp_request,_gz_somp_ack;
volatile u32 _gz_somp_report[64]={0x48535231u,256};
/* Retain the established audit scratch ABI. */
BbBank _gz_somp_snapshot,_gz_somp_current_birth;
u32 _gz_somp_rows[BB_CAP][8],_gz_somp_layer_receipt[8],_gz_somp_current_receipt[16];
LgBank _gz_somp_layers,_gz_somp_current_layers;
LcView _gz_somp_current_view;
LmPlan _gz_somp_current_plan;
static void audit_copy(void*d,const void*s,u32 n){u32 i;for(i=0;i<n/4;i++)((volatile u32*)d)[i]=((const volatile u32*)s)[i];}
void _gz_somp_step(void){u32 req=_gz_somp_request,slot=_gz_slots_selected,frame=_gz_roh_live_frame,z,t=0,i,open=0;
 if(!req||req==_gz_somp_ack)return;
 for(i=2;i<64;i++)_gz_somp_report[i]=0;
 for(i=0;i<16;i++)_gz_somp_current_receipt[i]=0;
 _gz_somp_report[2]=req;_gz_somp_report[3]=frame;_gz_somp_report[4]=_gz_route_choice;
 if(req!=_gz_somp_ack+1||slot>9||_gz_state!=1||_gz_tools_flags||_gz_pending||_gz_loading_store||_gz_somp_event){_gz_somp_report[5]=0xffffffffu;goto done;}
 z=_gz_slots_install(slot);_gz_somp_report[5]=z;if(z)goto done;
 z=_gz_roh_begin(slot,60);_gz_somp_report[6]=z;if(z)goto done;
 t=_gz_roh_owner[5];_gz_somp_report[7]=t;_gz_somp_report[8]=_gz_osa_pin_id();
 z=_gz_roh_service_begin(t,1);_gz_somp_report[9]=z;if(z)goto abort;
 open=1;
 _gz_somp_report[10]=_gz_lgc_build(t,frame);_gz_somp_report[11]=_gz_lgc_read(t,frame);
 _gz_somp_report[12]=_gz_srd_kind(t,frame);_gz_somp_report[13]=_gz_srd_serial(t,frame);
 _gz_somp_report[14]=_gz_bsr_read_world(t,frame);_gz_somp_report[15]=_gz_bsr_begin(slot,frame);_gz_somp_report[16]=_gz_bsr_end(t);
 _gz_somp_report[17]=_gz_lgc_read(t+1,frame);_gz_somp_report[18]=_gz_lgc_read(t,frame+1);
 _gz_somp_report[19]=_gz_slots_store(slot);_gz_somp_report[20]=_gz_roh_user_guard();
 _gz_somp_report[21]=_gz_srd_read_world(t,frame);_gz_somp_report[22]=_gz_srd_read_bank(t,frame);_gz_somp_report[23]=_gz_srd_read_layers(t,frame);
 audit_copy(&_gz_somp_current_view,&_gz_lgc_view,sizeof(LcView));
 audit_copy(&_gz_somp_current_birth,&_gz_lgc_birth,BB_BYTES);audit_copy(&_gz_somp_current_layers,&_gz_lgc_layers,LG_BYTES);audit_copy(&_gz_somp_current_plan,&_gz_lgm_work,LM_BYTES);
 _gz_somp_current_receipt[0]=_gz_lgc_view.required_count;_gz_somp_current_receipt[1]=_gz_lgc_view.required_unbound;
 _gz_somp_current_receipt[2]=_gz_lsp_work.count;_gz_somp_current_receipt[3]=_gz_lsp_work.valid;
 _gz_somp_report[24]=_gz_roh_service_end(t,1,0);open=0;
 _gz_somp_report[25]=_gz_lgc_read(t,frame);_gz_somp_report[26]=_gz_srd_read_world(t,frame);
 _gz_somp_report[27]=_gz_osa_pin_id();_gz_somp_report[28]=_gz_roh_user_guard();
 z=_gz_roh_service_begin(t,1);_gz_somp_report[29]=z;if(z)goto abort;
 open=1;_gz_somp_report[30]=_gz_srd_serial(t,frame);_gz_somp_report[31]=_gz_lgc_read(t,frame);
 _gz_somp_report[32]=_gz_lgc_build(t,frame);_gz_somp_report[33]=_gz_lgc_read(t,frame);
 _gz_somp_report[34]=_gz_roh_service_end(t,1,0);open=0;_gz_somp_report[35]=_gz_lgc_read(t,frame);
abort:
 if(open)_gz_somp_report[36]=_gz_roh_service_end(t,1,0);
 _gz_somp_report[37]=_gz_roh_abort_clean(t);_gz_somp_report[38]=_gz_lgc_read(t,frame);_gz_somp_report[39]=_gz_srd_kind(t,frame);
done:
 _gz_somp_report[40]=_gz_roh_owner[3];_gz_somp_report[41]=_gz_roh_owner[4];_gz_somp_report[42]=_gz_osa_pin_id();
 _gz_somp_report[43]=_gz_roh_user_guard();_gz_somp_report[44]=_gz_roh_live_frame;_gz_somp_report[45]=_gz_roh_owner[6];_gz_somp_report[46]=_gz_roh_owner[7];
 __asm__ volatile("sync":::"memory");_gz_somp_ack=req;
}
