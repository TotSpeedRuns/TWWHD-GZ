u32 _gz_lgb_archive_saved_validate(u32 p,u32 n){u32 world,bank,g;
 if(!gz_arc_shape(p,n)||R32(p,172)!=sizeof(VrWorld)||R32(p,204)!=LG_BYTES)return 42;
 world=p+R32(p,168);bank=p+R32(p,192);g=p+R32(p,200);
 if(_gz_lgb_validate((const LgBank*)g,LG_BYTES,(const VrWorld*)world,(const BbBank*)bank))return 42;
return 0;
}

u32 _gz_lgb_archive_validate(u32 p,u32 n,u32 installed){u32 g;u32 saved;
 saved=_gz_som_source_format_check(p,n,SFM_LG);
 if(saved!=SFM_FULL&&saved!=SFM_REUSE)return 42;
 if(saved==SFM_FULL&&_gz_lgb_archive_saved_validate(p,n))return 42;
 if(!gz_arc_shape(p,n)||R32(p,172)!=sizeof(VrWorld)||R32(p,204)!=LG_BYTES)return 42;
 g=p+R32(p,200);
 if(installed&&!same(&_gz_lgb_work,(const void*)g,LG_BYTES))return 42;
 return 0;
}
