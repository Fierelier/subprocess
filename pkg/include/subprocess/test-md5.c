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
	
	// cat
	subprocess_proc_t proc_cat;
	subprocess_return_t proc_cat_rtn;
	subprocess_pipe_t pipe_cat;
	rtnif(rtn,subprocess_pipe_create(&pipe_cat));
	rtnif(rtn,subprocess_open(&proc_cat,
		(subprocess_char_t *[]){T("cat"),T("pkg") PATHSEP T("include") PATHSEP T("subprocess") PATHSEP T("test-md5.c"),NULL},
	&pipe_cat,NULL,NULL));
	
	// md5sum
	subprocess_proc_t proc_md5;
	subprocess_return_t proc_md5_rtn;
	rtnif(rtn,subprocess_open(&proc_md5,
		(subprocess_char_t *[]){T("md5sum"),NULL},
	NULL,NULL,&pipe_cat));
	
	// wait for processes to exit
	rtnif(rtn,subprocess_wait(&proc_cat_rtn,&proc_cat,0));
	rtnif(rtn,subprocess_wait(&proc_md5_rtn,&proc_md5,0));
	
	// free the pipe
	subprocess_pipe_free(&pipe_cat);
	
	// raise errors, if they happened
	rtnif(proc_cat_rtn,proc_cat_rtn);
	rtnif(proc_md5_rtn,proc_md5_rtn);
	return 0;
}
