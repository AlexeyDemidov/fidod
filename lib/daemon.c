/*
 *  daemon mode support routines.
 *  Copyright (c) 1998 Alex L. Demidov
 */

/*
 *  $Id: daemon.c,v 1.1 1999-03-12 22:41:10 alexd Exp $
 *
 *  $Log: daemon.c,v $
 *  Revision 1.1  1999-03-12 22:41:10  alexd
 *  Initial revision
 *
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>

#include "daemon.h"
#include "log.h"

/*
 * daemon mode support
 */

int enter_suid()
{
    return setuid(0);
}

int leave_suid()
{

    /* FIXME check this */
    struct passwd *pwd = NULL;

    if (geteuid() != 0) {
	Perror("geteuid");
	return 1;
    }

    if ((pwd = getpwnam(euser)) == NULL) {
	Perror("getpwnam");
	return 1;
    }
    if (setgid(pwd->pw_gid) < 0) {
	Perror("setgid");
	return 1;
    }
    if (seteuid(pwd->pw_uid) < 0) {
	Perror("seteuid");
	return 1;
    }
    return 0;
}

/* returns pid number from pid file */

int check_pid_file()
{
    int fd = -1;
    int count = 0;
    char buf[32];

    debug("checking for pid file %s", pid_file);

    enter_suid();
    fd = open(pid_file, O_RDONLY);
    leave_suid();

    if (fd != -1) {

	debug("trying read pidfile %s", pid_file);

	enter_suid();
	count = read(fd, buf, sizeof(buf));
	close(fd);
	leave_suid();

	if (count > 0) {
	    pid_t pid = 0;

	    buf[count] = 0;
	    pid = atoi(buf);

	    debug("found pidfile %s pointing to process %d",
		  pid_file, pid);

	    if (pid) {
		if (pid != getpid()) {
		    if (kill(pid, 0) == 0) {
			return pid;
		    }
		    else {
			notice("Stale pidfile found %s", pid_file);
		    }
		}
	    }
	}
    }
    else {
	if (errno != ENOENT) {
	    error("unable to open pidfile for reading %s : %m", pid_file);
	    return -1;
	}
    }

    return 0;
}

int create_pid_file()
{
    int fd = -1;
    int count = 0;
    char buf[32];
    pid_t pid;

    pid = check_pid_file();
    if (pid) {
	if (pid != -1)
	    error("Already running with PID = %d", (int) pid);
	return 1;
    }

    remove_pid_file();

    debug("trying open pidfile %s for writing", pid_file);
    enter_suid();
    fd = open(pid_file, O_WRONLY | O_CREAT | O_EXCL | O_TRUNC, NULL);
    leave_suid();

    if (fd == -1) {
	error("unable to open pidfile for writing %s : %m", pid_file);
	return 1;
    }
    snprintf(buf, sizeof(buf), "%d\n", (int) getpid());

    enter_suid();
    count = write(fd, buf, strlen(buf));
    close(fd);
    leave_suid();
    if (count != strlen(buf)) {
	error("error writing to pidfile %s:%m", pid_file);
	remove_pid_file();
    }
    debug("pidfile %s for PID = %s created", pid_file, buf);
    return 0;
}

int remove_pid_file()
{
    int rc = 0;

    debug("trying unlink pidfile %s", pid_file);
    enter_suid();
    if (access(pid_file, F_OK) == 0)
	rc = unlink(pid_file);
    leave_suid();

    if (rc == -1) {
	error("unable to remove pidfile %s : %m", pid_file);
	return -1;
    }
    return 0;
}

#ifndef HAVE_DAEMON

int daemon(int nochdir, int noclose)
{
    pid_t pid;

    if (!nochdir)
	chdir("/");

    if ((pid = fork()) == -1) {
	Perror("fork");
	return 1;
    }
    else {
	if (pid)
	    exit(0);
	else {
	    if (!noclose) {
		/* close stdin/stdout/stderr */

		/* FIXME close all fd's, not only 1,2,3 */
		/* maybe, look at bsd daemon() */
		close(0);
		close(1);
		close(2);
	    }

	    /* become process group leader */
	    setsid();
	}
    }
    return 0;
}

#endif

void set_signals()
{
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    signal(SIGINT, sig_int);
    signal(SIGTERM, sig_term);
    signal(SIGCHLD, sig_chld);
}
