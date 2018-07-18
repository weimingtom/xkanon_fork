#include "music.h"
/*
typedef int boolean;
typedef short           pid_t; 
typedef struct {
	int t,m,s,f;
} cd_time;
*/

boolean cdrom_enable;
boolean pcm_enable;
int  mus_init(){return 0;}
int  mus_exit(int is_abort){return 0;}
boolean mus_get_pcm_state(){return 0;}
boolean mus_get_cdrom_state(){return 0;}
void mus_set_pcm_state(boolean _bool){return;}
void mus_set_cdrom_state(boolean _bool){return;}
int mus_set_8to16_usetable(int is_use){return 0;}
void mus_setLoopCount(int cnt){return;}
int  mus_cdrom_start(int track){return 0;}
int  mus_cdrom_stop(){return 0;}
int  mus_cdrom_getPlayStatus(cd_time *info){return 0;}
int  mus_effec_start(const char* buf, int loop){return 0;}
int  mus_bgm_start(char* buf, int loop){return 0;}
int  mus_koe_start(const char* fname){return 0;}
int  mus_effec_stop(){return 0;}
int  mus_bgm_stop(){return 0;}
int  mus_koe_stop(){return 0;}
int  mus_effec_getStatus(int *pos){return 0;}
int  mus_bgm_getStatus(int *pos){return 0;}
int  mus_koe_getStatus(int *pos){return 0;}
void mus_pcm_set_mix(int is_mix){return 0;}
void mus_pcmserver_stop(void){return 0;}
void mus_pcmserver_resume(void){return 0;}
void mus_set_default_rate(int arg1){return 0;}
int mus_get_default_rate(void){return 0;}
/*
int  mus_pcm_start(int no, int loop){return 0;}
int  mus_pcm_mix(int noL, int noR, int loop){return 0;}
int  mus_pcm_load(int no){return 0;}
*/
void mus_mixer_fadeout_start(int device, int time, int volume, int stop){return 0;}
int  mus_mixer_get_fadeout_state(int device){return 0;}
void mus_mixer_stop_fadeout(int device){return 0;}
int  mus_mixer_get_level(int device){return 0;}
void mus_mixer_set_default_level(int device, int volume){return 0;}
int mus_mixer_get_default_level(int device){return 0;}

void mixer_setvolume(int device, int vol){return 0;}

//pid_t fork_local(void){return 0;}
//pid_t forkpg_local(void){return 0;}

/* movie functions */
int mus_movie_start(const char* fname, int window_id, int x1, int y1, int x2, int y2, int loop_count){return 0;}
int mus_movie_stop(void){return 0;}
int mus_movie_pause(void){return 0;}
int mus_movie_resume(void){return 0;}
int mus_movie_getStatus(int* pos){return 0;}
void mus_movie_informend(void){return;}
void mus_kill_allchildren(void){return;}

/* device name */
void mixer_setDeviceName(char *name){return;}
void pcm_setDeviceName(char *name){return;}
void cdrom_setDeviceName(char *name){return;}

/* music packet receive / send */
void SendMsgServerToClient(SRVMSG *msg){return;}
void RecvMsgServerToClient(SRVMSG *msg, int is_wait){return;}
void SendMsgClientToServer(SRVMSG *msg){return;}
void RecvMsgClientToServer(SRVMSG *msg, int is_wait){return;}

/* music shared memory lock / unlock */
void mus_shmem_lock(void){return;}
void mus_shmem_unlock(void){return;}

void cd_set_devicename(char *name) {
}
