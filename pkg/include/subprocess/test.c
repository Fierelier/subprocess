// To use this, use -Wl,--wrap=malloc,--wrap=free
#include "malloc_count.c"

//#define clnup_dbg
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

#ifdef _WIN32
	int get_open_descriptors() {
		DWORD count;
		GetProcessHandleCount(GetCurrentProcess(),&count);
		return (int)count;
	}
#else
	#include <dirent.h>
	int get_open_descriptors() {
		int count = 0;
		DIR *dir = opendir("/proc/self/fd");
		if (!dir) return -1;
		while (readdir(dir)) { count++; }
		closedir(dir);
		return count - 2; // subtract "." and ".."
	}
#endif

#include <stdlib.h>
#include <string.h>
int main(int argc,char * argv[]) {
	int rtn;
	subprocess_proc_t proc;
	subprocess_return_t proc_rtn;
	if ((rtn = subprocess_init())) { return rtn; }
	
	printf("* TEST - Summoning a command and letting it run\n\n");
	rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND1,NULL,NULL,NULL));
	rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
	printf("return: %i\n\n",(int)proc_rtn);
	
	printf("------------------------\n");
	printf("* TEST - Summoning a command and terminating it after 1 second\n\n");
	rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND1,NULL,NULL,NULL));
	rtnif(rtn && rtn != SUBPROCESS_EOF,subprocess_wait(&proc_rtn,&proc,1000));
	rtnif(rtn,subprocess_terminate(&proc));
	rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
	printf("return: %i\n\n",(int)proc_rtn);
	
	
	{
		printf("------------------------\n");
		printf("* TEST - Summoning a command and capturing its stdout\n\n");
		subprocess_pipe_t p;
		rtnif(rtn,subprocess_pipe_create(&p));
		rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND2,&p,NULL,NULL));
		char * data = malloc(32768);
		if (data == NULL) { return 1; };
		subprocess_pipesize_t read = 0;
		subprocess_pipesize_t s;
		while (1) {
			rtnif(rtn && rtn != SUBPROCESS_EOF,subprocess_pipe_read(&p,1024,&s,data + read));
			read += s;
			if (rtn == SUBPROCESS_EOF) { break; }
		}
		rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
		subprocess_pipe_free(&p);
		printf("Captured data:\n%s",data);
		free(data);
		printf("return: %i\n\n",(int)proc_rtn);
		
		printf("------------------------\n");
		printf("* TEST - Summoning a command and printing into stdin\n\n");
		rtnif(rtn,subprocess_pipe_create(&p));
		rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND3,NULL,NULL,&p));
		SLEEP(1000);
		printf("echo hello world\n");
		rtnif(rtn,subprocess_pipe_write(&p,strlen("echo hello world\n"),"echo hello world\n"));
		SLEEP(1000);
		printf("exit\n");
		rtnif(rtn,subprocess_pipe_write(&p,strlen("exit\n"),"exit\n"));
		rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
		subprocess_pipe_free(&p);
		printf("return: %i\n\n",(int)proc_rtn);
	}
	
	{
		printf("------------------------\n");
		printf("* TEST - Memory leaks\n\n");
		char buf[4096];
		int i = 0;
		int before = get_open_descriptors();
		subprocess_pipe_t pipe_stdout;
		subprocess_pipe_t pipe_stderr;
		while (i < 100) {
			printf(".");
			fflush(stdout);
			rtnif(rtn,subprocess_pipe_create(&pipe_stdout));
			rtnif(rtn,subprocess_pipe_create(&pipe_stderr));
			rtnif(rtn,subprocess_open(&proc,(char *[])TEST_COMMAND2,&pipe_stdout,&pipe_stderr,NULL));
			
			// Hide output of commands
			while (1) {
				rtnif(rtn && rtn != SUBPROCESS_EOF,subprocess_pipe_read(&pipe_stdout,4096,(subprocess_pipesize_t *)buf,buf));
				if (rtn == SUBPROCESS_EOF) { break; }
			}
			
			while (1) {
				rtnif(rtn && rtn != SUBPROCESS_EOF,subprocess_pipe_read(&pipe_stderr,4096,(subprocess_pipesize_t *)buf,buf));
				if (rtn == SUBPROCESS_EOF) { break; }
			}
			
			rtnif(rtn,subprocess_wait(&proc_rtn,&proc,0));
			rtnif(rtn,subprocess_pipe_free(&pipe_stdout));
			rtnif(rtn,subprocess_pipe_free(&pipe_stderr));
			++i;
		}
		printf("\n");
		printf("Additional (file) descriptors: %d\n",get_open_descriptors() - before);
	}
	
	return 0;
}
