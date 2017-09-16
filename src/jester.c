/*
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as
 *      published by the Free Software Foundation.
 *
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: brabo
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>

#define MAXLEN 64

int interactive = 1;
char exec[MAXLEN];

/* Parse our arguments */
static int parse_opts(int argc, char *args[])
{
	int count = 0;
	int c;

	if( argc < 2 ){
		fprintf(stderr, "Usage: %s [-i|-x CMD] /dev/serial\r\n", args[0]);
		exit(1);
	}

	while ((c = getopt(argc, args, "ix:")) != -1) {
		switch (c) {
			case 'i':
				interactive = 1;
				count++;
				break;
			case 'x':
				strncpy(exec, optarg, MAXLEN - 1);
				strncat(exec, "\n", 1);
				interactive = 0;
				count += 2;
				break;
			default:
				fprintf(stderr, "%s: invalid option -'%c'\r\n", args[0], c);
				break;
			}
	}

	return count;
}

int tty_connect(char *tty)
{
	int fd;

	if (interactive) {
		fd = open(tty, O_RDWR | O_NONBLOCK);
	} else {
		fd = open(tty, O_RDWR);
	}


	if (fd < 0) {
		perror("Open");
		exit(2);
	}

	if (!isatty(fd)) {
		printf("Not a TTY!\n");
		exit(3);
	}

	char *name = ttyname(fd);
	printf("Connected to device %s\n", name);

	struct termios tio;
	memset(&tio, 0, sizeof (struct termios));

    tio.c_cflag = CS8;
 	tio.c_cc[VMIN]  = 1;

 	if (cfsetispeed(&tio, B115200) < 0 || cfsetospeed(&tio, B115200) < 0) {
 		printf("Failed to set baudrate!\n");
 		exit(4);
 	}

 	tcsetattr(fd, TCSANOW, &tio);

 	return fd;
}

int tty_send(int fd, char *cmd)
{
 	int j;
 	char c;
 	int ret = 0;

 	for (j = 0; j < strlen(cmd); j++) {
 		if (ret = write(fd, &cmd[j], 1) < 1) {
 			printf("Write fail ret %d\n", ret);
 			return -1;
 		}

 		if (ret = read(fd, &c, 1) < 0) {
 			printf("Read fail ret %d\n", ret);
 			return -1;
 		}

 		write(STDIN_FILENO, &c, 1);
 	}

 	return ret;
}

unsigned char tty_recv(int fd)
{
	unsigned char c;
	if (read(fd, &c, 1) > 0)
		write(STDOUT_FILENO, &c, 1);

	return c;
}

int main(int argc, char *argv[])
{
	int i = 1;
	i += parse_opts(argc, argv);

	int fd = tty_connect(argv[i]);

	struct termios stdio;
	if (interactive) {
		fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	    memset(&stdio, 0, sizeof (stdio));
    	stdio.c_cc[VMIN] = 1;
    	tcsetattr(STDOUT_FILENO, TCSANOW, &stdio);
    	tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio);
	}

 	unsigned char c;
 	unsigned char last;
 	char getstatus[64] = "echo $?\n";
 	int done = 0, end = 0;

 	while (2 > 1) {
 		c = tty_recv(fd);

 		if (interactive) {
 			if (read(STDIN_FILENO, &c, 1) > 0)  write(fd, &c, 1);
 		} else {
	 		if ((last == '#') && (c == ' ')) {
	 			if (!done) {
 					if (!tty_send(fd, exec)) {
 						done++;
	 				}
					usleep(100000);
				} else if (!end) {
 					if (!tty_send(fd, getstatus)) {
 						end++;
 					}
	 			} else {
	 				break;
	 			}
	 		}
 		}
 		last = c;
	}
	printf("\n");

	// if you're happy and you know it,
	exit(0);
}