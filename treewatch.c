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

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/inotify.h>
#include <sys/ioctl.h>
#define INOTIFY_EVENTSIZE sizeof(struct inotify_event)

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int reconfigure_fd;

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
			free(filename);

			printf("----------------\n");
			pid = fork();
			switch(pid) {
				case -1:
					break;
				case 0:
					simpleexec("/usr/bin/make", "");
					exit(-1);
					break;
			}
		}
	}
}


int main(int argc, char *argv[])
{
	char *watchdir;
	int status, select_max = 0;

	fd_set active_fd_set, read_fd_set;
	reconfigure_fd = inotify_init();
	if(reconfigure_fd <= 0)
	{
		fprintf(stderr, "Reconfiguration: Error: initialization failed\n");
		return;
	}

	/* Test - FIXME: add etc/ggzd/rooms? */
	inotify_add_watch(reconfigure_fd, ".", IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_TO);

	FD_ZERO(&active_fd_set);
	FD_SET(reconfigure_fd, &active_fd_set);
	select_max = MAX(select_max, reconfigure_fd);

	while(1){
		read_fd_set = active_fd_set;

		status = select((select_max + 1), &read_fd_set, NULL, NULL, NULL);

			if(FD_ISSET(reconfigure_fd, &read_fd_set)) {
				reconfiguration_handle();
			}
	}

	return 0;
}

