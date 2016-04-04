//RFC 1889

#include "random_unique32.h"


static u_long md_32(char *string, int length)
{
	md5_state_t state;
	union {
		md5_byte_t digest[16];
		u_long x[4];
	}udigest;

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)string, length);
	md5_finish(&state, (md5_byte_t*)&udigest);

	u_long r;
	int i;
	r = 0;
	for (i = 0; i < 3; i++) {
	   r ^= udigest.x[i];
	}
	return r;
}

#ifdef ANDROID
static long gethostid(void)
{   
	return 0;
}
#endif

/*
* Return random unsigned 32-bit quantity. Use 'type' argument if you
* need to generate several different values in close succession.
*/
uint32_t random32(int type)
{
	struct {
	   int     type;
	   struct  timeval tv;
	   clock_t cpu;
	   pid_t   pid;
	   u_long  hid;
	   uid_t   uid;
	   gid_t   gid;
	   struct  utsname name;
	} s;

	gettimeofday(&s.tv, 0);
	uname(&s.name);
	s.type = type;
	s.cpu  = clock();
	s.pid  = getpid();
	s.hid  = gethostid();
	s.uid  = getuid();
	s.gid  = getgid();

	return md_32((char *)&s, sizeof(s));
}