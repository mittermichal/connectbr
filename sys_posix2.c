/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sched.h>
#include <errno.h>
#include <dirent.h>

#include <SDL/SDL.h>
#include <dlfcn.h>

#include "quakedef.h"
//#include "server.h"
#include "pcre.h"
#include "q_shared.h"
#include "cvar.h"
#include "localtime.h"
// BSD only defines FNDELAY:
#ifndef O_NDELAY
#  define O_NDELAY	FNDELAY
#endif

int noconinput = 0;

qbool stdin_ready;
int do_stdin = 1;

cvar_t sys_nostdout = {"sys_nostdout", "0"};

void Sys_Printf (char *fmt, ...)
{
#ifdef DEBUG
	va_list argptr;
	char text[2048];
	unsigned char *p;

	return;

	va_start (argptr,fmt);
	vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	if (sys_nostdout.value)
		return;

	for (p = (unsigned char *) text; *p; p++)
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
#else
	return;
#endif
}

void Sys_Quit(void)
{
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~O_NDELAY);
	exit(0);
}

void Sys_Init(void)
{
#ifdef __APPLE__
       extern void init_url_handler();
       init_url_handler();
#endif
}

void Sys_Error(char *error, ...)
{
	extern FILE *qconsole_log;
	va_list argptr;
	char string[1024];

	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~O_NDELAY);	//change stdin to non blocking

	va_start (argptr, error);
	vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);
	fprintf(stderr, "Error: %s\n", string);
    //if (qconsole_log)
    //	fprintf(qconsole_log, "Error: %s\n", string);

    //Host_Shutdown ();
	exit(1);
}

void Sys_mkdir (const char *path)
{
	mkdir (path, 0777);
}

/*
================
Sys_remove
================
*/
int Sys_remove(char *path)
{
	return unlink(path);
}

int Sys_rmdir(const char *path)
{
	return rmdir(path);
}

int Sys_FileSizeTime(char *path, int *time1)
{
	struct stat buf;
	if (stat(path, &buf) == -1)
	{
		*time1 = -1;
		return 0;
	}
	else
	{
		*time1 = buf.st_mtime;
		return buf.st_size;
	}
}


#if (_POSIX_TIMERS > 0) && defined(_POSIX_MONOTONIC_CLOCK)
#include <time.h>
double Sys_DoubleTime(void)
{
	static unsigned int secbase;
	struct timespec ts;

#ifdef __linux__
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#else
	clock_gettime(CLOCK_MONOTONIC, &ts);
#endif

	if (!secbase) {
		secbase = ts.tv_sec;
		return ts.tv_nsec / 1000000000.0;
	}

	return (ts.tv_sec - secbase) + ts.tv_nsec / 1000000000.0;
}
#else
double Sys_DoubleTime(void)
{
	struct timeval tp;
	struct timezone tzp;
	static int secbase;

	gettimeofday(&tp, &tzp);

	if (!secbase) {
	    secbase = tp.tv_sec;
	    return tp.tv_usec/1000000.0;
	}

	return (tp.tv_sec - secbase) + tp.tv_usec / 1000000.0;
}
#endif
/*
int main(int argc, char **argv)
{
	double time, oldtime, newtime;
	int i;

#ifdef __linux__
	extern void InitSig(void);
	InitSig();
#endif

	COM_InitArgv (argc, argv);

	// let me use -condebug C:\condebug.log before Quake FS init, so I get ALL messages before quake fully init
	if ((i = COM_CheckParm("-condebug")) && i < COM_Argc() - 1) {
		extern FILE *qconsole_log;
		char *s = COM_Argv(i + 1);
		if (*s != '-' && *s != '+')
			qconsole_log = fopen(s, "a");
	}

	signal(SIGFPE, SIG_IGN);

	// we need to check for -noconinput and -nostdout before Host_Init is called
	if (!(noconinput = COM_CheckParm("-noconinput")))
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

	if (COM_CheckParm("-nostdout"))
		sys_nostdout.value = 1;

	Host_Init (argc, argv, 32 * 1024 * 1024);

	oldtime = Sys_DoubleTime ();
	while (1) {
		// find time spent rendering last frame
		newtime = Sys_DoubleTime();
		time = newtime - oldtime;
		oldtime = newtime;

		Host_Frame(time);
	}
}
*/
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length) {
	int r;
	unsigned long addr;
	int psize = getpagesize();

	addr = (startaddr & ~(psize - 1)) - psize;
	r = mprotect((char*) addr, length + startaddr - addr + psize, 7);
	if (r < 0)
    		Sys_Error("Protection change failed");
}

void _splitpath(const char *path, char *drive, char *dir, char *file, char *ext)
{
    const char *f, *e;

    if (drive)
    drive[0] = 0;

    f = path;
    while (strchr(f, '/'))
    f = strchr(f, '/') + 1;

    if (dir)
    {
    strncpy(dir, path, min(f-path, _MAX_DIR));
        dir[_MAX_DIR-1] = 0;
    }

    e = f;
    while (*e == '.')   // skip dots at beginning
    e++;
    if (strchr(e, '.'))
    {
    while (strchr(e, '.'))
        e = strchr(e, '.')+1;
    e--;
    }
    else
    e += strlen(e);

    if (file)
    {
        strncpy(file, f, min(e-f, _MAX_FNAME));
    file[min(e-f, _MAX_FNAME-1)] = 0;
    }

    if (ext)
    {
    strncpy(ext, e, _MAX_EXT);
    ext[_MAX_EXT-1] = 0;
    }
}

// full path
char *Sys_fullpath(char *absPath, const char *relPath, int maxLength)
{
    // too small buffer, copy in tmp[] and then look is enough space in output buffer aka absPath
    if (maxLength-1 < PATH_MAX)	{
			 char tmp[PATH_MAX+1];
			 if (realpath(relPath, tmp) && absPath && strlen(tmp) < maxLength+1) {
					strlcpy(absPath, tmp, maxLength+1);
          return absPath;
			 }

       return NULL;
		}

    return realpath(relPath, absPath);
}
// kazik <--

int Sys_EnumerateFiles (char *gpath, char *match, int (*func)(char *, int, void *), void *parm)
{
	DIR *dir, *dir2;
	char apath[MAX_OSPATH];
	char file[MAX_OSPATH];
	char truepath[MAX_OSPATH];
	char *s;
	struct dirent *ent;

	//printf("path = %s\n", gpath);
	//printf("match = %s\n", match);

	if (!gpath)
		gpath = "";
	*apath = '\0';

	strncpy(apath, match, sizeof(apath));
	for (s = apath+strlen(apath)-1; s >= apath; s--)
	{
		if (*s == '/')
		{
			s[1] = '\0';
			match += s - apath+1;
			break;
		}
	}
	if (s < apath)  //didn't find a '/'
		*apath = '\0';

	snprintf(truepath, sizeof(truepath), "%s/%s", gpath, apath);


	//printf("truepath = %s\n", truepath);
	//printf("gamepath = %s\n", gpath);
	//printf("apppath = %s\n", apath);
	//printf("match = %s\n", match);
	dir = opendir(truepath);
	if (!dir)
	{
		Com_DPrintf("Failed to open dir %s\n", truepath);
		return true;
	}
	do
	{
		ent = readdir(dir);
		if (!ent)
			break;
		if (*ent->d_name != '.')
			if (wildcmp(match, ent->d_name))
			{
				snprintf(file, sizeof(file), "%s/%s", truepath, ent->d_name);
				//would use stat, but it breaks on fat32.

				if ((dir2 = opendir(file)))
				{
					closedir(dir2);
					snprintf(file, sizeof(file), "%s%s/", apath, ent->d_name);
					//printf("is directory = %s\n", file);
				}
				else
				{
					snprintf(file, sizeof(file), "%s%s", apath, ent->d_name);
					//printf("file = %s\n", file);
				}

				if (!func(file, -2, parm))
				{
					closedir(dir);
					return false;
				}
			}
	} while(1);
	closedir(dir);

	return true;
}

/*************************** INTER PROCESS CALLS *****************************/
#define PIPE_BUFFERSIZE		1024

FILE *fifo_pipe;

static char *Sys_PipeFile(void) {
	static char pipe[MAX_PATH] = {0};

	if (*pipe)
		return pipe;

	snprintf(pipe, sizeof(pipe), "/tmp/ezquake_fifo_%s", getlogin());
	return pipe;
}

void Sys_InitIPC(void)
{
	int fd;
	mode_t old;

	/* Don't use the user's umask, make sure we set the proper access */
	old = umask(0);
	if (mkfifo(Sys_PipeFile(), 0600)) {
		umask(old);
		// We failed ... 
		return;
	}
	umask(old); // Reset old mask

	/* Open in non blocking mode */
	if ((fd = open(Sys_PipeFile(), O_RDONLY | O_NONBLOCK)) == -1) {
		// We failed ...
		return;
	}
	if (!(fifo_pipe = fdopen(fd, "r"))) {
		// We failed ...
		return;
	}
}

