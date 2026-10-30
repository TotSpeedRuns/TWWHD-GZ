#ifndef GZ_BRG3_REPLAY_H
#define GZ_BRG3_REPLAY_H
unsigned int _gz_brr_idle(void);
unsigned int _gz_brr_private_apart(unsigned int,unsigned int);
unsigned int _gz_brr_native_context(void),_gz_brr_source_frame(void);
unsigned int _gz_brr_gpu_prepare(unsigned int,unsigned int);
unsigned int _gz_brr_needs_heap(unsigned int,unsigned int);
unsigned int _gz_brr_prepare(unsigned int,unsigned int);
unsigned int _gz_brr_prepared_before(unsigned int,unsigned int);
unsigned int _gz_brr_sealed(unsigned int);
unsigned int _gz_brr_forward(unsigned int,unsigned int);
unsigned int _gz_brr_target(unsigned int,unsigned int);
unsigned int _gz_brr_undo(unsigned int,unsigned int);
unsigned int _gz_brr_before(unsigned int,unsigned int);
unsigned int _gz_brr_commit_scope(unsigned int,unsigned int);
unsigned int _gz_brr_forget_clean(unsigned int);
unsigned int _gz_brr_factory_source_allowed(unsigned int,unsigned int);
void _gz_brr_finish_leaf(unsigned int);
#endif
