extern void _gz_hif_step(void);
extern void _gz_wdp_step(void),_gz_wct_step(void);
/* Private native observation: prepare one expired-bomb recipe under the real
 * owner, exercise service-scoped access and abort cleanly. No world mutation. */
extern u32 _gz_slots_install(u32),_gz_roh_begin(u32,u32),_gz_roh_service_begin(u32,u32);
extern u32 _gz_roh_service_end(u32,u32,u32),_gz_roh_abort_clean(u32);
volatile u32 _gz_hbp_request,_gz_hbp_ack;
u32 _gz_hbp_report[64]={0x48425054u,256};
void _gz_hbp_step(void){u32 req=_gz_hbp_request,t=0,open=0,z,i,p,slot=_gz_pbac_selected,frame=_gz_pbac_frame;
 if(!req||req==_gz_hbp_ack)return;
 for(i=2;i<64;i++)_gz_hbp_report[i]=0;
 _gz_hbp_report[2]=req;_gz_hbp_report[3]=frame;_gz_hbp_report[4]=slot;
 if(req!=_gz_hbp_ack+1||_gz_pbac_active||_gz_pbac_issued.valid||_gz_pbac_fatal||_gz_osa_pin_id()||_gz_roh_user_guard()){
  _gz_hbp_report[5]=5;goto done;
 }
 z=_gz_slots_install(slot);_gz_hbp_report[5]=z;if(z)goto done;
 z=_gz_roh_begin(slot,60);_gz_hbp_report[6]=z;if(z)goto done;
 t=_gz_roh_owner[5];_gz_hbp_report[7]=t;_gz_hbp_report[8]=_gz_osa_pin_id();
 z=_gz_roh_service_begin(t,1);_gz_hbp_report[9]=z;if(z)goto abort;
 open=1;_gz_hbp_report[10]=_gz_hbp_prepare(t,frame);p=_gz_hbp_read(t,frame);_gz_hbp_report[11]=p;
 if(p){const HbpRecipe*s=(const HbpRecipe*)p;
  _gz_hbp_report[12]=s->source;_gz_hbp_report[13]=s->used;_gz_hbp_report[14]=s->pin;_gz_hbp_report[15]=s->identity;
  _gz_hbp_report[16]=s->generation;_gz_hbp_report[17]=s->serial;_gz_hbp_report[18]=s->saved.pid;
  _gz_hbp_report[19]=btr_saved(&s->saved,0xa7c,2);_gz_hbp_report[20]=s->recipe.pid;_gz_hbp_report[21]=s->row_hash;
 }
 _gz_hbp_report[22]=_gz_osa_native_store(slot);_gz_hbp_report[23]=_gz_hbp_forget(t);
 _gz_hbp_report[24]=_gz_hbp_read(t+1,frame);_gz_hbp_report[25]=_gz_hbp_read(t,frame+1);
 _gz_hbp_report[26]=_gz_roh_service_end(t,1,0);open=0;
 _gz_hbp_report[27]=_gz_hbp_read(t,frame);_gz_hbp_report[28]=_gz_osa_pin_id();
 z=_gz_roh_service_begin(t,1);_gz_hbp_report[29]=z;if(z)goto abort;
 open=1;_gz_hbp_report[30]=_gz_hbp_read(t,frame);_gz_hbp_report[31]=_gz_roh_owner[13];
 _gz_hbp_report[32]=_gz_hbp_prepare(t,frame);_gz_hbp_report[33]=_gz_roh_service_end(t,1,0);open=0;
abort:
 if(open)_gz_hbp_report[34]=_gz_roh_service_end(t,1,0);
 _gz_hbp_report[35]=_gz_roh_abort_clean(t);_gz_hbp_report[36]=_gz_hbp_read(t,frame);
 _gz_hbp_report[37]=_gz_hbp_forget(t);_gz_hbp_report[38]=_gz_osa_pin_id();_gz_hbp_report[39]=_gz_roh_user_guard();
done:
 _gz_hbp_report[40]=_gz_roh_owner[3];_gz_hbp_report[41]=_gz_roh_owner[4];_gz_hbp_report[42]=_gz_roh_owner[7];
 _gz_hbp_report[43]=_gz_pbac_frame;__asm__ volatile("sync":::"memory");_gz_hbp_ack=req;
}
void _gz_boh_step(void);
extern void _gz_hir_step(void);
extern void _gz_srh_step(void);
extern void _gz_dto_step(void);
extern void _gz_gqk_step(void);
extern void _gz_doh_step(void);
void _gz_bfm_tick(void);
void _gz_bex_probe_step(void);
void _gz_brx_probe_step(void);
void _gz_wsp_step(void);
void _gz_wrt_step(void);
extern void _gz_bci_step(void);
void _gz_hbp_dispatch(void){_gz_kec_tick();_gz_boh_step();_gz_bfm_tick();_gz_bex_probe_step();_gz_brx_probe_step();_gz_hif_step();wcm_continue_tick();_gz_wct_step();_gz_wcm_tick();_gz_wdp_step();_gz_wsp_step();_gz_wrt_step();_gz_bci_step();}
