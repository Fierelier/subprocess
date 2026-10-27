#include <subprocess/subprocess.h>
#include <subprocess/subprocess.c>

#define rtnif(comp,call) \
	rtn = call; \
	if (comp) { \
		printf("%s:%i - rtnif triggered, quitting with status %i\n* call: %s\n* comp: %s\n",__FILE__,__LINE__,rtn,#call,#comp); \
		return rtn; \
	}

#ifndef T
	#define T TEXT
#endif

#ifndef _WIN32
	#define PATHSEP T("/")
#else
	#define PATHSEP T("\\")
#endif

#include <stdlib.h>
#include <string.h>
int main(int argc,char * argv[]) {
	int rtn;
	rtnif(rtn,subprocess_init());
	
	// type
	subprocess_proc_t proc_type;
	subprocess_return_t proc_type_rtn;
	subprocess_pipe_t pipe_type;
	rtnif(rtn,subprocess_pipe_create(&pipe_type));
	rtnif(rtn,subprocess_open(&proc_type,
		(subprocess_char_t *[]){T("command.com"),T("/c"),T("type"),T("pkg") PATHSEP T("include")PATHSEP T("subprocess") PATHSEP T("test-w95.c"),NULL},
	&pipe_type,NULL,NULL));
	
	// more
	subprocess_proc_t proc_more;
	subprocess_return_t proc_more_rtn;
	rtnif(rtn,subprocess_open(&proc_more,
		(subprocess_char_t *[]){T("command.com"),T("/c"),T("more"),NULL},
	NULL,NULL,&pipe_type));
	
	// wait for processes to exit
	rtnif(rtn,subprocess_wait(&proc_type_rtn,&proc_type,0));
	rtnif(rtn,subprocess_wait(&proc_more_rtn,&proc_more,0));
	
	// free the pipe
	subprocess_pipe_free(&pipe_type);
	
	// raise errors, if they happened
	rtnif(proc_type_rtn,proc_type_rtn);
	rtnif(proc_more_rtn,proc_more_rtn);
	return 0;
}
