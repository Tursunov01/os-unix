#ifndef DAEMONIZE_H
#define DAEMONIZE_H

#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define	MAXLINE	4096

void daemonize(const char *cmd);

void err_quit(const char *fmt, ...);

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);

#endif