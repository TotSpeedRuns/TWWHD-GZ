API u32 _gz_b294m_archive_saved_validate(u32 p,u32 n){u32 b,world;
 if(!gz_arc_shape(p,n)||*(const u32*)(p+172)!=sizeof(VrWorld)||*(const u32*)(p+212)!=B294_BYTES)return 42;
 world=p+*(const u32*)(p+168);b=p+*(const u32*)(p+208);
 if(_gz_b294m_source_matches(b,B294_BYTES,(const VrWorld*)world))return 42;

 return 0;
}

API u32 _gz_b294m_archive_validate(u32 p,u32 n,u32 installed){u32 b;u32 saved;
 saved=_gz_som_source_format_check(p,n,SFM_B294);
 if(saved!=SFM_FULL&&saved!=SFM_REUSE)return 42;
 if(saved==SFM_FULL&&_gz_b294m_archive_saved_validate(p,n))return 42;
 if(!gz_arc_shape(p,n)||*(const u32*)(p+172)!=sizeof(VrWorld)||*(const u32*)(p+212)!=B294_BYTES)return 42;
 b=p+*(const u32*)(p+208);
 if(installed&&!equal(_gz_b294_source,(const u8*)b,B294_BYTES))return 42;
 return 0;
}
