#ifndef clnup_h
#define clnup_h
#include <setjmp.h>

#define clnup_start(area) \
	jmp_buf _clnup_jmpbuf_##area; \
	int _clnup_status_##area = setjmp(_clnup_jmpbuf_##area);

#ifndef clnup_debug
	#define clnup_action(action) action
#else
	#define clnup_action(action) \
		printf("[clnup_debug] %s:%d: "#action";\n",__FILE__,__LINE__); \
		action
#endif

#define clnup_var(area,type,key,value,cleanup) \
	volatile type key; \
	if (_clnup_status_##area == 0) { \
		key = value; \
	} else if (key != value) { \
		clnup_action(cleanup); \
	}

#define clnup_malloc(area,type,key) clnup_var(area,type,key,NULL,free(key))

#define clnup_checkvar(key,value,cleanup) if (key != value) { clnup_action(cleanup); }

#define clnup_checkmalloc(key) if (key != NULL) { clnup_action(free(key)); }

#define clnup_table(area,type,key) volatile type key;
#define clnup_tableset(area,key,value,cleanup) \
	if (_clnup_status_##area == 0) { \
		key = value; \
	} else if (key != value) { \
		clnup_action(cleanup); \
	}

#define clnup_end(area,rtn) if ((_clnup_status_##area != 0)) { return rtn; }

#define clnup_call(area) clnup_action(longjmp(_clnup_jmpbuf_##area,1));
#endif
