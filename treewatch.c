/*
 * File: treewatch.c Author: Roger Light Date: 2008/02/24 Desc: Watch a list of
 * directories and execute a program on a file change.  $Id: control.c 9702
 * 2008-02-11 19:18:17Z josef $
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
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/inotify.h>
#include <sys/ioctl.h>

#define INOTIFY_EVENTSIZE sizeof(struct inotify_event)
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static struct option options[] = {
	{"command", required_argument, 0, 'c'},
	{"directory", required_argument, 0, 'd'},
	{"file-endings", required_argument, 0, 'f'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};

int reconfigure_fd;
char *command;
// FIXME char *fileending;

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
	token = strtok(options, " ,.");
	while(token)
	{
		listsize++;
		list = (char**)realloc(list, listsize * sizeof(char*));
		list[listsize - 2] = (char*)malloc(strlen(token) + 1);
		strcpy(list[listsize - 2], token);
		list[listsize - 1] = NULL;
		token = strtok(NULL, " ,.");
	}

	return list;
}

/* Just execute a program without any checking */
static void simpleexec(const char *cmdline, const char *options)
{
	char *const *opts = optlist((char*)cmdline, (char*)options);

	execvp(cmdline, opts);
}


static void reconfiguration_handle(void)
{
	int pending;
	char buf[4096];
	struct inotify_event ev;
	int offset;
	int diff;
	char *filename;
	pid_t pid;

	offset = 0;

	ioctl(reconfigure_fd, FIONREAD, &pending);
	while(pending > 0) {
		if (pending > (int)sizeof(buf)) pending = sizeof(buf);
		pending = read(reconfigure_fd, buf, pending);

		while(pending > 0) {
			memcpy(&ev, &buf[offset], INOTIFY_EVENTSIZE);
			filename = malloc(ev.len + 1);
			memcpy(filename, &buf[offset + INOTIFY_EVENTSIZE], ev.len);

			diff = INOTIFY_EVENTSIZE + ev.len;
			pending -= diff;
			offset += diff;

			if(strncmp(filename + strlen(filename) - 2, ".c", 2)
					&& strncmp(filename + strlen(filename) - 2, ".h", 2)
					&& strncmp(filename + strlen(filename) - 4, ".cpp", 4)
					) {
				free(filename);
				continue;
			}

			/*
			switch(ev.mask){
				case IN_CLOSE_WRITE:
					printf("%s (IN_CLOSE_WRITE)\n", filename);
					break;
				case IN_DELETE:
					printf("%s (IN_DELETE)\n", filename);
					break;
				case IN_MOVED_TO:
					printf("%s (IN_MOVED_TO)\n", filename);
					break;
			}
			*/
			free(filename);

			printf("----------------\n");
			pid = fork();
			switch(pid) {
				case -1:
					break;
				case 0:
					simpleexec(command, "");
					exit(-1);
					break;
			}
		}
	}
}


int main(int argc, char *argv[])
{
	int status, select_max = 0;
	int sopt, optindex;
	int have_directory = 0;

	command = NULL;
	// FIXME fileending = NULL;

	/* Initialise inotify */
	fd_set active_fd_set, read_fd_set;
	reconfigure_fd = inotify_init();
	if(reconfigure_fd <= 0) {
		fprintf(stderr, "Error: inotify initialization failed.\n");
		return 1;
	}

	/* Parse arguments */
	while(1) {
		sopt = getopt_long(argc, argv, "c:d:f:h", options, &optindex);
		if(sopt == -1) break;
		switch(sopt) {
			case 'c':
				command = optarg;
				break;
			case 'd':
				/* Specify directories to watch */
				inotify_add_watch(reconfigure_fd, optarg, IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_TO);
				have_directory = 1;
				break;
			case 'f':
				// FIXME fileending = optarg;
				break;
			case 'h':
	 			printf("treewatch\n\n"),
				printf("Copyright (C) 2008 Roger Light\nhttp://atchoo.org/tools/treewatch/\n\n");
				printf("treewatch comes with ABSOLUTELY NO WARRANTY.  You may distribute treewatch freely\nas described in the COPYING file distributed with this file.\n\n");
				printf("treewatch is a program to watch a directory and execute a program on file changes.\n\n");
				printf("Usage: treewatch \n\n");

				printf(" -c, --command        Specify full path to command to run (default: /usr/bin/make)\n");
				printf(" -d, --directory      List of directories to watch, separated by a semicolon ;.\n");
				printf("                      (default: current directory)\n");
				// FIXME printf(" -f, --file-ending    List of file endings to watch, separated by a semicolon ;.\n");
				// FIXME printf("                      (default: .c;.cpp;.h)\n");
				printf(" -h, --help           Display this help.\n");
				printf("\nSee http://atchoo.org/tools/treewatch/ for updates.\n");
				exit(0);
				break;
			default:
				exit(0);
				break;
		}
	}

	if(!command) command = strdup("/usr/bin/make");
	if(!have_directory){
		inotify_add_watch(reconfigure_fd, ".", IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_TO);
	}
	//if(!fileending) fileending = strdup("


	FD_ZERO(&active_fd_set);
	FD_SET(reconfigure_fd, &active_fd_set);
	select_max = MAX(select_max, reconfigure_fd);

	/* Main program loop - broken out of only by user interrupt. */
	while(1){
		read_fd_set = active_fd_set;

		status = select((select_max + 1), &read_fd_set, NULL, NULL, NULL);

		if(FD_ISSET(reconfigure_fd, &read_fd_set)) {
			reconfiguration_handle();
			}
	}

	if(command) free(command);

	return 0;
}

