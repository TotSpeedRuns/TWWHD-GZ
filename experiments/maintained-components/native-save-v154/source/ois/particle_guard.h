#ifndef GZ_PARTICLE_GUARD_H
#define GZ_PARTICLE_GUARD_H
typedef unsigned int JpgU;
enum {JPG_METHODS=9,JPG_CALLS=32,JPG_WAIT=5,JPG_OWNER=40,JPG_CORRUPT=1690};
/* Coordination only. A closed/empty guard covers these nine native methods,
 * not all indirect producers, renderer resources or GPU completion. */
JpgU _gz_jpg_acquire(JpgU token,JpgU frame);
JpgU _gz_jpg_check(JpgU token,JpgU frame);
void _gz_jpg_poll(void);
JpgU _gz_jpg_gpu_restore(JpgU token,JpgU frame);
JpgU _gz_jpg_gpu_restore_ready(JpgU token,JpgU frame);
JpgU _gz_jpg_capture_begin(JpgU mode,JpgU frame);
JpgU _gz_jpg_capture_check(JpgU mode,JpgU frame);
JpgU _gz_jpg_capture_gpu(JpgU mode,JpgU frame);
JpgU _gz_jpg_capture_gpu_ready(JpgU mode,JpgU frame);
JpgU _gz_jpg_capture_end(JpgU mode,JpgU frame);
extern JpgU _gz_jpg_report[96];
JpgU _gz_jpg_calc(JpgU,JpgU);
JpgU _gz_jpg_setup(JpgU,JpgU,JpgU),_gz_jpg_substep(JpgU,JpgU,JpgU);
JpgU _gz_jpg_after(JpgU,JpgU,JpgU),_gz_jpg_drawstep(JpgU,JpgU,JpgU);
JpgU _gz_jpg_draw(JpgU,JpgU,JpgU);
JpgU _gz_jpg_create(JpgU,JpgU,JpgU,JpgU,JpgU,JpgU,JpgU);
JpgU _gz_jpg_remove(JpgU,JpgU),_gz_jpg_debug(JpgU);
#endif
