// To use this, use -Wl,--wrap=malloc,--wrap=free
#include "malloc_count.c"

#include <subprocess/subprocess.h>
#include <subprocess/subprocess.c>

#ifndef _WIN32
	#define TEST_COMMAND1 {"ping","127.0.0.1","-c","5","-i","1",NULL}
	#define TEST_COMMAND2 {"ls",NULL}
	#define TEST_COMMAND3 {"sh",NULL}
	
	#include <unistd.h>
	#define SLEEP(ms) usleep(ms * 1000);
#else
	#define TEST_COMMAND1 {"ping","127.0.0.1","-n","5",NULL}
	#define TEST_COMMAND2 {"cmd","/c","dir",NULL}
	#define TEST_COMMAND3 {"cmd",NULL}
	
	#include <windows.h>
	#define SLEEP(ms) Sleep(ms);
#endif

#define rtnif(comp,call) \
	rtn = call; \
	if (comp) { \
		printf("%s:%i - rtnif triggered, quitting with status %i\n* call: %s\n* comp: %s\n",__FILE__,__LINE__,rtn,#call,#comp); \
		return rtn; \
	}

#include <stdlib.h>
#include <string.h>
int main(int argc,char * argv[]) {
	int rtn;
	subprocess_proc_t proc;
	subprocess_return_t proc_rtn;
	if ((rtn = subprocess_init())) { return rtn; }
	
	printf("* TEST - Summoning a command and letting it run\n\n");
	rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND1,0,0,0));
	rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
	printf("return: %i\n\n",(int)proc_rtn);
	
	printf("------------------------\n");
	printf("* TEST - Summoning a command and terminating it after 1 second\n\n");
	rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND1,0,0,0));
	rtnif(rtn && rtn != SUBPROCESS_EOF,subprocess_wait(&proc_rtn,&proc,1000));
	rtnif(rtn,subprocess_terminate(&proc));
	rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
	printf("return: %i\n\n",(int)proc_rtn);
	
	printf("------------------------\n");
	printf("* TEST - Summoning a command and capturing its stdout\n\n");
	rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND2,1,0,0));
	char * data = malloc(32768);
	if (data == NULL) { return 1; };
	subprocess_pipesize_t read = 0;
	subprocess_pipesize_t s;
	while (1) {
		rtnif(rtn && rtn != SUBPROCESS_EOF,subprocess_read_stdout(data + read,1024,&s,&proc));
		read += s;
		if (rtn == SUBPROCESS_EOF) { break; }
	}
	rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
	printf("Captured data:\n%s",data);
	free(data);
	printf("return: %i\n\n",(int)proc_rtn);
	
	printf("------------------------\n");
	printf("* TEST - Summoning a command and printing into stdin\n\n");
	rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND3,0,0,1));
	rtnif(rtn,subprocess_write_stdin("echo hello world\n",strlen("echo hello world\n"),&proc));
	SLEEP(1000);
	rtnif(rtn,subprocess_write_stdin("exit\n",strlen("exit\n"),&proc));
	rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
	printf("return: %i\n\n",(int)proc_rtn);
	
	if (1) {
		char buf[4096];
		printf("------------------------\n");
		printf("* TEST - Memory leaks\n\n");
		int i = 0;
		while (i < 100) {
			printf("%i ...\n",i);
			rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND2,1,1,1));
			while (1) {
				rtnif(rtn && rtn != SUBPROCESS_EOF,subprocess_read_stdout(buf,4096,(subprocess_pipesize_t *)buf,&proc));
				if (rtn == SUBPROCESS_EOF) { break; }
			}
			
			while (1) {
				rtnif(rtn && rtn != SUBPROCESS_EOF,subprocess_read_stderr(buf,4096,(subprocess_pipesize_t *)buf,&proc));
				if (rtn == SUBPROCESS_EOF) { break; }
			}
			
			rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
			++i;
		}
	}
	
	return 0;
}
