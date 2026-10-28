u32 _gz_bcb_archive_saved_validate(u32 p,u32 n){u32 b,bytes,world;
 if(!gz_arc_shape(p,n)||R32(p,172)!=sizeof(VrWorld))return 42;
 b=p+R32(p,192);bytes=R32(p,196);world=p+R32(p,168);
 if(_gz_bcb_validate((const BbBank*)b,bytes,(const VrWorld*)world))return 42;

 return 0;
}

u32 _gz_bcb_archive_validate(u32 p,u32 n,u32 installed){u32 b,bytes;u32 saved;
 saved=_gz_som_source_format_check(p,n,SFM_BC);
 if(saved!=SFM_FULL&&saved!=SFM_REUSE)return 42;
 if(saved==SFM_FULL&&_gz_bcb_archive_saved_validate(p,n))return 42;
 if(!gz_arc_shape(p,n)||R32(p,172)!=sizeof(VrWorld))return 42;
 b=p+R32(p,192);bytes=R32(p,196);
 if(!gz_sfm_bank_bounds(p,n,b,bytes,BB_HEADER,BB_BYTES))return 42;
 if(installed&&!same((u32)&_gz_bcb_work,b,bytes))return 42;
 return 0;
}
