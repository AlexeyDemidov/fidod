/*
 *  fidod.c - fido spool monitor.
 *  Copyright (c) 1998 Alex L. Demidov
 */

/*
 *  $Id: fidod.c,v 1.6 2001-03-24 20:25:01 alexd Exp $
 *
 *  $Log: fidod.c,v $
 *  Revision 1.6  2001-03-24 20:25:01  alexd
 *  program name cleanup
 *
 *  Revision 1.5  2001/03/24 17:51:13  alexd
 *  Added RCS strings to .c files
 *
 *  Revision 1.4  2001/03/24 17:48:40  alexd
 *  Changed *program for logging
 *
 *  Revision 1.3  2001/03/24 17:03:57  alexd
 *  fixed time( NULL ) alls
 *
 *  Revision 1.2  2000/04/23 09:19:12  alexd
 *  version 0.2
 *
 *  Revision 1.1.1.1  1999/03/12 22:41:10  alexd
 *  imported fidod
 *
 *
 */

#include <unistd.h>
#include <string.h>			/* strrchr */
#include <sys/wait.h>			/* wait */
#include <dirent.h>
#include <errno.h>
#include <syslog.h>
#include <regex.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>

#include "config.h"
#include "global.h"
#include "daemon.h"
#include "log.h"
#include "readcfg.h"

/*
   config file
   outbound dir
   ifcico

 */

const static char *rcsid = "$Id: fidod.c,v 1.6 2001-03-24 20:25:01 alexd Exp $";
const static char *revision = "$Revision: 1.6 $";

char *program = PACKAGE;

/* configureation variables */
const char *config_file = CONFIGFILE;

const char *euser = USER;
const char *pid_file = PIDFILE;
const int facility = LOGFACILITY;
const char *logfile = LOGFILE;

const char *unpack_program = UNPACKPROGRAM;
const char *pack_program = PACKPROGRAM;

const char *inbound_dir = INBOUNDDIR;
const char *prot_inbound_dir = INBOUNDDIR;;

const int CHECK_PERIOD = 10;

const time_t RUN_PERIOD = 60;

const int wait_child = 0;

int daemon_mode = 1;
int debug_mode = 0;
int verbose_mode = 0;

var_t vars[] =
{
	{"user", &euser, getvarstr, 0},
	{"pidfile", &pid_file, getvarstr, 0},
	{"logfile", &logfile, getvarstr, 0},
	{"inbound", &inbound_dir, getvarstr, 0},
	{"prot_inbound", &prot_inbound_dir, getvarstr, 0},
	{"unpack", &unpack_program, getvarstr, 0},
	{"pack", &pack_program, getvarstr, 0},
	{"debug", &debug_mode, getvarbool, 0},
	{"verbose", &verbose_mode, getvarbool, 0},
	{NULL, NULL, NULL, 0}
};

/* runtime global var */

pid_t unpack_running = 0;

regex_t reg_outpkt;
regex_t reg_inpkt;
regex_t reg_bundle;

const char *mask_outpkt = "^[0-9A-F]{8}\\.OUT$";
const char *mask_inpkt = "^[0-9A-F]{8}\\.PKT$";

const char *mask_bundle =
"^[0-9A-F]{8}\\.MO|TU|WE|TH|FR|SA|SU[0-9A-Z]$";

void free_masks();

void shutdown(int rc)
{
	pid_t pid;

	free_masks();

	pid = check_pid_file();
	if (!pid)
		remove_pid_file();

	notice("shutdown");
	shutdownlog();
	exit(rc);
}

/* signal handlers */

void sig_int(int sig)
{
	shutdown(0);
}

void sig_term(int sig)
{
	shutdown(0);
}

void sig_chld(int sig)
{
	pid_t child;
	int status;

	while ((child = waitpid(-1, &status, WNOHANG)) != 0) {
		if (child == -1) {
			Perror("waitpid");
			break;
		}
		notice("child process %d exited with status %d",
			   child, WEXITSTATUS(status));
		if (unpack_running == child)
			unpack_running = 0;

	}
	signal(SIGCHLD, sig_chld);
}

int check_exec()
{
	if (access(unpack_program, X_OK)) {
		error("unpack program %s not found", unpack_program);
		return 1;
	}
	if (access(pack_program, X_OK)) {
		error("pack program %s not found", pack_program);
		return 1;
	}
	return 0;
}

void run_unpack()
{
	pid_t pid;
	int status;

	static time_t last_run = 0;

	if (unpack_running) {
		debug("unpack already running with pid %d", unpack_running);
		return;
	}

	if ((last_run + RUN_PERIOD) > time( NULL )) {
		debug("respawning unpack too fast.");
		return;
	}

	notice("running unpack program %s", unpack_program);
	if (!(pid = fork())) {
		if (execl(unpack_program, unpack_program, NULL)) {
			error("exec of unpack program %s failed: %m", unpack_program);
		}
	}
	if (pid == -1) {
		error("fork for unpack program failed: %m");
	}
	else {
		unpack_running = pid;

		if (wait_child) {
			wait(&status);
			debug("unpack program returned with status %d", status);
		}
		last_run = time( NULL );
	}
}

/* regexp's */

int compile_mask(regex_t * preg, const char *mask)
{

	char errbuf[80];
	int rc = regcomp(preg, mask, REG_ICASE | REG_EXTENDED);

	if (rc != 0) {
		regerror(rc, preg, errbuf, sizeof errbuf);
		debug("regcomp(%s): %s", mask, errbuf);
	}
	return rc;
}

int compile_masks()
{
	return compile_mask(&reg_outpkt, mask_outpkt)
		|| compile_mask(&reg_inpkt, mask_inpkt)
		|| compile_mask(&reg_bundle, mask_bundle);
}

void free_masks()
{
	regfree(&reg_outpkt);
	regfree(&reg_inpkt);
	regfree(&reg_bundle);
}

int match_mask(regex_t * preg, const char *filename)
{

	int match = REG_NOMATCH;
	size_t nmatch = 0;
	regmatch_t *pmatch = 0;

	match = regexec(preg, filename, nmatch, pmatch, 0);
	debug("match mask on %s : %s", filename, match ? "no" : "yes");

	return !match;
}

int check_inbound(const char *path)
{
	DIR *inbound;

	struct dirent *file;

	/* check inbound directory */
	if (!access(path, X_OK)) {
		if ((inbound = opendir(path)) != NULL) {
			while ((file = readdir(inbound)) != NULL) {
				debug("found file %s", file->d_name);
				/* FIXME: check only files */
				if (match_mask(&reg_inpkt, file->d_name)
					|| match_mask(&reg_bundle, file->d_name)
					) {
					/* FIXME: unpack only prot */
					run_unpack();
				}
			}
			closedir(inbound);
		}
		else
			debug("can't read inbound dir %s", inbound);
	}
	else {
		debug("can't access inbound dir %s", inbound);
		return 1;
	}
	return 0;

}

void main_loop()
{
	while (1) {
		check_inbound(prot_inbound_dir);
		check_inbound(inbound_dir);
		sleep(CHECK_PERIOD);
	}
}

void usage()
{

}

void parse_args(int argc, char **argv)
{

	int c;

	while (1) {
		c = getopt(argc, argv, "dVvc:l:p:");
		if (c == -1)
			break;

		switch (c) {
			case 'd':
				debug_mode = 1;
				verbose_mode = 1;
				daemon_mode = 0;
				break;
			case 'V':
				printf( PACKAGE " " VERSION " " __DATE__ "\n");
				exit(0);
				break;
			case 'v':
				verbose_mode = 1;
				break;
			case 'c':
				config_file = strdup(optarg);
				break;
			case 'l':			/* FIXME: lock var from config */
				logfile = strdup(optarg);
				break;
			case 'p':			/* FIXME: lock var from config */
				pid_file = strdup(optarg);
				break;
			case ':':
				break;
			case '?':
				break;

			default:
				printf("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}

	return;
}

int main(int argc, char **argv)
{
	char *program;

	leave_suid();

	if ( (program = strrchr(argv[0], '/')) == 0 )
		program = argv[0];
	else
		program += 1;

	if ( program == NULL || *program == 0 )
		program = PACKAGE;

	parse_args(argc, argv);

	debug("starting");

	/* FIXME: don't overwrite vars settled by command-line options */
	read_config();

	initlog(program, facility, logfile);

	if (daemon_mode) {
		if (daemon(0, 0))
			shutdown(1);
	}
	set_signals();
	if (create_pid_file())		/* bug - don't call shutdown - it removes pidfile */
		shutdown(1);

	if (check_exec())
		shutdown(1);

	if (compile_masks())
		shutdown(2);

	notice("started");

	main_loop();

	shutdown(0);
	return 0;
}
