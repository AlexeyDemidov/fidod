#ifndef _LOG_H
#define _LOG_H

/*
 *  log.h - logging routines.
 *  Copyright (c) 1998 Alex L. Demidov
 */

/*
 *  $Id: log.h,v 1.1 1999-03-12 22:41:10 alexd Exp $
 *
 *  $Log: log.h,v $
 *  Revision 1.1  1999-03-12 22:41:10  alexd
 *  Initial revision
 *
 *
 */

extern int debug_mode;
extern int daemon_mode;
extern int verbose_mode;


void initlog( const char *program, int facility, const char *logfile);
void shutdownlog();

void debug ( const char *format, ...);
void error ( const char *format, ...);
void notice( const char *format, ...);

#if 0
void Perror( const char *str);
#else
#define Perror( str ) error( "(%s:%d) %s:%m", __FILE__, __LINE__, str )
#endif

#endif /* _LOG_H */
