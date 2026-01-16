#ifndef clnup_h
#define clnup_h
#include <setjmp.h>

#define clnup_srt(name) \
	jmp_buf name; \
	int name##_int = setjmp(name); \

#ifndef clnup_dbg
	#define clnup_act(action) action
#else
	#define clnup_act(action) \
		printf("[clnup_dbg] %s:%d: "#action";\n",__FILE__,__LINE__); \
		action
#endif

#define clnup_var(name,type,key,value,cleanup) \
	volatile type key; \
	if (name##_int == 0) { \
		key = value; \
	} else if (key != value) { \
		clnup_act(cleanup); \
	}

#define clnup_mlc(name,type,key) clnup_var(name,type,key,NULL,free(key))

#define clnup_cvr(key,value,cleanup) if (key != value) { clnup_act(cleanup); }

#define clnup_cml(key) if (key != NULL) { clnup_act(free(key)); }

#define clnup_tbl(name,type,key) volatile type key;
#define clnup_tbs(name,key,value,cleanup) \
	if (name##_int == 0) { \
		key = value; \
	} else if (key != value) { \
		clnup_act(cleanup); \
	}

#define clnup_end(name,rtn) if ((name##_int != 0)) { return rtn; }

#define clnup_cll(name) clnup_act(longjmp(name,1));
#endif
