/*
 *  log.c - logging routines.
 *  Copyright (c) 1998 Alex L. Demidov
 */

/*
 *  $Id: log.c,v 1.1 1999-03-12 22:41:10 alexd Exp $
 *
 *  $Log: log.c,v $
 *  Revision 1.1  1999-03-12 22:41:10  alexd
 *  Initial revision
 *
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "daemon.h"
#include "log.h"

static int syslog_init = 0;
static char *logfile = 0;

/* FIXME: prog name */
static char *program = "tossd";

void initlog( const char *a_program, int facility, const char *a_logfile) {
    program = strdup(a_program);
    logfile = strdup(a_logfile);

    openlog( program, LOG_CONS|LOG_PID , facility);
    syslog_init = 1;
}

void shutdownlog() {
    closelog();

    syslog_init = 0;
    
    free(logfile);
    free(program);
    
    logfile = program = NULL;
}



static void vmessage(  int loglevel, const char *format, va_list ap) {

    /* FIXME: try to log messages of any size */
#define BUFSIZE 256
    
    char buf[BUFSIZE];
    
    /* FIXME: logging to console if errors and logfile still not initilized */
    if( syslog_init )
        vsyslog( loglevel, format, ap );
    
    if( logfile || !daemon_mode ) {
        char str[BUFSIZE];
        char timestamp[BUFSIZE];
        
        time_t t = time(NULL);
        struct tm *ptm = localtime( &t );
        
        vsnprintf( str, BUFSIZE, format, ap );
        strftime( timestamp, BUFSIZE, "%b %d %H:%M:%S", ptm);
        snprintf( buf, BUFSIZE, "%s %s[%d]: %s\n", timestamp, program, (int)getpid(), str );
    }
    
    if( logfile ) {
        FILE *log;
        
        enter_suid();
        log = fopen( logfile, "a");
        leave_suid();
        
        if ( !log ) {
	    /* FIXME: don't try write to file */
            /* error("can't open logfile %s:%m", logfile); */
        }
        else  {
            fprintf( log, buf  );
            fclose( log );
        }
    }
    
    if (!daemon_mode) {
        fprintf( stderr, buf );
    }
}

/* FIXME: maybe cpp define? for those functions */

void debug( const char *format, ... ) {
    va_list ap;

    if( !verbose_mode )
        return;
    va_start( ap, format);
    vmessage( LOG_DEBUG, format, ap );
    va_end(ap);
}
void notice( const char *format, ... ) {
    va_list ap;
    
    va_start( ap, format);
    vmessage( LOG_NOTICE, format, ap );
    va_end(ap);
}

void error( const char *format, ... ) {
    va_list ap;
    
    va_start( ap, format);
    vmessage( LOG_ERR, format, ap );
    va_end(ap);
}

void message( int loglevel, const char *format, ... ) {
    va_list ap;
    
    va_start( ap, format);
    vmessage( loglevel, format, ap );
    va_end(ap);
}

#if 0
void Perror( const char *str) {
    error( "%s:%m", str );
}
#endif
