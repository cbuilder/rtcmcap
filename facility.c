/*
 * File: facility.c
 * Description: Special functions
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

extern FILE * log_file, *debug_file;

void p_errno(FILE *fd, const char *format, ...)
{
    char *errmsg = strerror(errno);
    va_list arglist;
    va_start(arglist, format);
    vfprintf(fd, format, arglist);
    va_end(arglist);
    fprintf(fd, ": %s\n", errmsg);
}

void conf_read(char *config)
{
    ;
}
