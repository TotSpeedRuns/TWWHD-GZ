/* UI storage uses only the original SDK MEM2 allocator. Native task heaps
 * are never borrowed. A private, signature-checked boot patch leaves 32 MiB
 * more in that SDK heap; it does not change restore admission or GPU fences. */
typedef unsigned int U32;
extern U32 MEMAllocFromExpHeapEx(U32,U32,int),_gz_roh_user_guard(void);
U32 _gz_uia_report[24]={0x55494131u,96,1};
static U32 busy;
U32 _gz_uia_allocate(U32 original,U32 bytes,int alignment){U32 p=0,z=0;
 if(!__sync_bool_compare_and_swap(&busy,0,1))return 0;
 _gz_uia_report[3]++;_gz_uia_report[8]=bytes;_gz_uia_report[9]=(U32)alignment;
 _gz_uia_report[10]=original;_gz_uia_report[11]=0;
 if(!bytes||bytes>0x1000000u||alignment!=256){z=1;goto done;}
 if(_gz_roh_user_guard()){z=2;goto done;}
 if(!original){z=3;goto done;}
 p=MEMAllocFromExpHeapEx(original,bytes,alignment);
 if(p)_gz_uia_report[4]++;else z=4;
done:
 _gz_uia_report[7]=z;_gz_uia_report[12]=p;if(z)_gz_uia_report[6]++;
 __sync_synchronize();busy=0;return p;
}
