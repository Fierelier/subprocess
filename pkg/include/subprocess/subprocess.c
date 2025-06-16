#ifndef SUBPROCESS_C
#define SUBPROCESS_C

#include "subprocess.h"
#ifdef _WIN32
	#include "windows.c"
#else
	#include "unix.c"
#endif

#endif
