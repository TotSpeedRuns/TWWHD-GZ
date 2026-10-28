u32 _gz_pfb_archive_saved_validate(u32 p,u32 n){u32 pf,pbytes,world,old,oldbytes,e;
 if(!gz_arc_shape(p,n)||R32(p,172)!=sizeof(VrWorld))return 42;
 pf=p+R32(p,184);pbytes=R32(p,188);world=p+R32(p,168);old=p+R32(p,160);oldbytes=R32(p,164);
 e=_gz_pfb_validate(pf,pbytes,(const VrWorld*)world,old,oldbytes);if(e)return 42;
 /* The attachment's saved generation must match the same archive's owner
  * birth record. This reads source metadata only, never a historical actor. */
 {u32 i,j,row,npc,birth=p+R32(p,192),birthbytes=R32(p,196),count=R32(world,16);
  if(birthbytes!=96+count*192)return 42;
  for(i=0;i<R32(pf,16);i++){row=pf+64+i*2048;if(R32(row,8)!=3)continue;
   npc=R32(row+1444,24);
   for(j=0;j<count;j++)if(R32(world+80+j*48,4)==npc)break;
   if(j==count||R32(birth+96+j*192,4)!=1||R32(birth+96+j*192,12)!=R32(row+1444,28))return 42;
  }
 }

 return 0;
}

u32 _gz_pfb_archive_validate(u32 p,u32 n,u32 installed){u32 pf,pbytes;u32 saved;
 saved=_gz_som_source_format_check(p,n,SFM_PF);
 if(saved!=SFM_FULL&&saved!=SFM_REUSE)return 42;
 if(saved==SFM_FULL&&_gz_pfb_archive_saved_validate(p,n))return 42;
 if(!gz_arc_shape(p,n)||R32(p,172)!=sizeof(VrWorld))return 42;
 pf=p+R32(p,184);pbytes=R32(p,188);
 if(!gz_sfm_bank_bounds(p,n,pf,pbytes,PB_HEADER,PB_BYTES))return 42;
 if(installed&&!same((u32)_gz_pfb_work,pf,pbytes))return 42;
 return 0;
}
