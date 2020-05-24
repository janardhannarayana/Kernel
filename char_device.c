/*
 * char_device.c -- This file creates a character device file
 *
 * Written on Thursday, 21 May 2020.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/sysmacros.h>


int main(int argc, char *argv[])
{
    if (argc < 2) {
	printf("Usage: executable <major number>\n");
	return 0;
    }	

    dev_t dev = 0;
    int major = atoi(argv[1]);

    dev = makedev(major, 0);
    mknod("./chardevice1", S_IFCHR, dev);

    int fd = open("./chardevice1", O_RDWR);
    printf("fd = %d\n", fd);
    char buff[128];
    write(fd, "THIS IS A SAMPLE CHARACTER DEVICE DRIVER\n", sizeof("THIS IS A SAMPLE CHARACTER DEVICE DRIVER\n"));
    read(fd, buff, sizeof("THIS IS A SAMPLE CHARACTER DEVICE DRIVER\n"));
    printf("%s\n", buff);
    return 0;
}
