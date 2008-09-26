/*
 * File: treewatch.c Author: Roger Light
 * Date: 2008/02/24
 * Desc: Watch a list of directories and execute a program on a file change.
 *
 * Portions of this file are taken from the GGZ Gaming Zone and are Copyright
 * Josef Spillner.
 *
 * optlist() and simpleexec() come from grubby/grubby/modules/exec.c The bulk
 * of reconfiguration_handle() and main() come from ggzd/ggzd/control.c
 *
 * Copyright (C) 2008 Roger Light.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <fcntl.h>
#include <getopt.h>
#include <libintl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sys/inotify.h>
#include <sys/ioctl.h>

#define INOTIFY_EVENTSIZE sizeof(struct inotify_event)

#define _(String) gettext(String)

static struct option options[] = {
	{"all-files", no_argument, 0, 'a'},
	{"command", required_argument, 0, 'c'},
	{"directory", required_argument, 0, 'd'},
	{"file-endings", required_argument, 0, 'f'},
	{"command-options", required_argument, 0, 'o'},
	{"help", no_argument, 0, 'h'},
	{"verbose", no_argument, 0, 'v'},
	{"wait", no_argument, 0, 'w'},
	{0, 0, 0, 0}
};

/* The inotify file descriptor. */
int reconfigure_fd;
/* The child command to run. */
char *command;
/* The child command options. */
char *command_options;
/* List of file ending strings to watch. */
char **fileendings;
/* Number of file endings in the list. */
int fileendings_count;
/* Should we wait for the child command to finish before continuing. */
int waitforcommand;
/* Should we watch all files for changes rather than specific endings? */
int all_files;
/* Be verbose? */
int verbose;


/* Build up an option list for the simpleexec function */
static char *const *optlist(char *cmdline, char *options)
{
	static char **list = NULL;
	int i;
	char *token;
	int listsize;

	/* Free list first if already initialized*/
	if(list)
	{
		i = 0;
		while(list[i])
		{
			free(list[i]);
			i++;
		}
		free(list);
	}

	/* Create list from words in the option string */
	list = (char**)malloc(2 * sizeof(char*));
	list[0] = (char*)malloc(strlen(cmdline) + 1);
	strcpy(list[0], cmdline);
	list[1] = NULL;
	listsize = 2;
	token = strtok(options, " ");
	while(token)
	{
		listsize++;
		list = (char**)realloc(list, listsize * sizeof(char*));
		list[listsize - 2] = (char*)malloc(strlen(token) + 1);
		strcpy(list[listsize - 2], token);
		list[listsize - 1] = NULL;
		token = strtok(NULL, " ");
	}

	return list;
}


/* Just execute a program without any checking */
static void simpleexec(const char *cmdline, const char *options)
{
	char *const *opts = optlist((char*)cmdline, (char*)options);

	execvp(cmdline, opts);
}


/* Execute the specified command. If this is caused by a file changing, pass
 * the filename in and possibly use it in the command options. */
void execute_command(const char *filename)
{
	char *modified_options;
	char *pos;

	if(filename && strstr(command_options, "%f")){
		modified_options = calloc((strlen(command_options) - 2 + strlen(filename) + 1), sizeof(char));

		if(!modified_options){
			fprintf(stderr, _("Error: Out of memory\n"));
			exit(-1);
		}

		sprintf(modified_options, "%s", command_options);
		pos = strstr(modified_options, "%f");
		pos[0] = '\0';

		modified_options = strcat(modified_options, filename);
		modified_options = strcat(modified_options, strstr(command_options, "%f")+2);
		simpleexec(command, modified_options);
	}else{
		simpleexec(command, command_options);
	}
}


/* Respond to a USR1 signal. */
void sigusr_handle(int signum)
{
	pid_t pid;
	int status;

	pid = fork();

	switch(pid){
		case -1:
			break;
		case 0:
			/* child */
			execute_command(NULL);
			exit(0);
		default:
			if(waitforcommand){
				waitpid(pid, &status, 0);
				if(verbose){
					printf(_("--- %s exited with status %d ---\n"), command, status);
				}
			}
			break;
	}
}


/* Called when there is a change on the inotify fd. */
static void reconfiguration_handle(void)
{
	int pending;
	char buf[4096];
	struct inotify_event ev;
	int offset;
	int diff;
	char *filename;
	pid_t pid;
	int i;
	int status;

	offset = 0;

	ioctl(reconfigure_fd, FIONREAD, &pending);
	while(pending > 0) {
		if (pending > (int)sizeof(buf)) pending = sizeof(buf);
		pending = read(reconfigure_fd, buf, pending);

		while(pending > 0) {
			memcpy(&ev, &buf[offset], INOTIFY_EVENTSIZE);
			filename = malloc(ev.len + 1);
			if(!filename){
				fprintf(stderr, _("Error: Out of memory\n"));
				exit(-1);
			}
			memcpy(filename, &buf[offset + INOTIFY_EVENTSIZE], ev.len);

			diff = INOTIFY_EVENTSIZE + ev.len;
			pending -= diff;
			offset += diff;

			for(i = 0; i < fileendings_count; i++){
				if(all_files || !strncmp(filename + strlen(filename) - strlen(fileendings[i]), 
							fileendings[i], strlen(fileendings[i]))){
					i = fileendings_count;
					pid = fork();
					switch(pid) {
						case -1:
							break;
						case 0:
							execute_command(filename);
							/* If the fork() fails, exit. */
							exit(-1);
							break;
						default:
							if(waitforcommand){
								waitpid(pid, &status, 0);
								if(verbose){
									printf(_("--- %s exited with status %d ---\n"), command, status);
								}
							}
							break;
					}
				}
			}

			free(filename);
		}
	}
}


int main(int argc, char *argv[])
{
	int status;
	int sopt, optindex;
	/* has the user used a -d argument? If not then use the default directory. */
	int have_directory = 0;

	waitforcommand = 1;
	command = NULL;
	command_options = NULL;
	fileendings_count = 0;
	fileendings = NULL;
	all_files = 0;
	verbose = 0;

	/* Initialise inotify */
	fd_set active_fd_set, read_fd_set;
	reconfigure_fd = inotify_init();
	if(reconfigure_fd <= 0) {
		fprintf(stderr, _("Error: inotify initialization failed.\n"));
		return 1;
	}

	/* Parse arguments */
	while(1) {
		sopt = getopt_long(argc, argv, "ac:d:f:ho:vw", options, &optindex);
		if(sopt == -1) break;
		switch(sopt) {
			case 'a':
				all_files = 1;
				break;
			case 'c':
				command = optarg;
				break;
			case 'd':
				/* Specify directories to watch */
				inotify_add_watch(reconfigure_fd, optarg, IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_TO);
				have_directory = 1;
				break;
			case 'f':
				fileendings_count++;
				fileendings = realloc(fileendings, sizeof(char *)*fileendings_count);
				fileendings[fileendings_count-1] = strdup(optarg);
				break;
			case 'h':
	 			printf(_("treewatch version %s (build date: %s)\n\n"), VERSION, BUILDDATE),
				printf(_("Copyright (C) 2008 Roger Light\nhttp://atchoo.org/tools/treewatch/\n\n"));
				printf(_("treewatch comes with ABSOLUTELY NO WARRANTY.  You may distribute treewatch freely\nas described in the COPYING file distributed with this file.\n\n"));
				printf(_("treewatch is a program to watch a directory and execute a program on file changes.\n\n"));
				printf(_("Usage: treewatch -h\n"));
				printf(_("       treewatch [-aw] [-c command] [-d dir] [-f file] [-o \"some options\"]\n\n"));
				printf(_(" -a, --all-files      Watch all files in the directories. Makes any --file-ending \n                      arguments redundant.\n"));
				printf(_(" -c, --command        Specify full path to command to run (default: /usr/bin/make)\n"));
				printf(_(" -d, --directory      Directory to watch. May be specified multiple times.\n                      (default: current directory)\n"));
				printf(_(" -f, --file-ending    File endings to watch. May be specified mutiple times.\n                      (default is to watch all of: .c .cpp .h)\n"));
				printf(_(" -h, --help           Display this help.\n"));
				printf(_(" -o, --options        Options to pass to the command to run.\n"));
				printf(_(" -v, --verbose        Be more verbose (print child exit status).\n"));
				printf(_(" -w, --no-wait        Don't wait for child command to terminate.\n"));
				printf(_("\nSee http://atchoo.org/tools/treewatch/ for updates.\n"));
				exit(0);
				break;
			case 'o':
				command_options = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'w':
				waitforcommand = 0;
				break;
			default:
				exit(0);
				break;
		}
	}

	/* Set up the defaults if the user hasn't supplied the given arguments. */
	if(!command) command = strdup("/usr/bin/make");
	if(!command_options) command_options = strdup("");

	if(!have_directory){
		inotify_add_watch(reconfigure_fd, ".", IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_TO);
	}
	if(!fileendings_count){
		fileendings_count = 3;
		fileendings = malloc(sizeof(char *)*fileendings_count);
		fileendings[0] = strdup(".c");
		fileendings[1] = strdup(".h");
		fileendings[2] = strdup(".cpp");
	}

	FD_ZERO(&active_fd_set);
	FD_SET(reconfigure_fd, &active_fd_set);

	signal(SIGUSR1, sigusr_handle);

	/* Main program loop - broken out of only by user interrupt. */
	while(1){
		read_fd_set = active_fd_set;

		status = select(reconfigure_fd + 1, &read_fd_set, NULL, NULL, NULL);

		if(FD_ISSET(reconfigure_fd, &read_fd_set)) {
			reconfiguration_handle();
		}
	}

	if(command) free(command);

	return 0;
}

