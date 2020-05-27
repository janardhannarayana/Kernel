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
#include <sys/wait.h>


void *pthread_func1(void *major)
{
    dev_t dev;
    
    dev = makedev(*(int *)major, 0);
    mknod("./chardevice1", S_IFCHR, dev);
    
    int fd = open("./chardevice1", O_RDWR);
    printf("func1: tid = %lu, fd = %d\n",pthread_self(), fd);
    char buff[128];
//`    write(fd, "THIS IS A SAMPLE CHARACTER DEVICE 1\n", sizeof("THIS IS A SAMPLE CHARACTER DEVICE 1\n"));
    sleep(3);
    read(fd, buff, sizeof("THIS IS A SAMPLE CHARACTER DEVICE 1\n"));
    printf("tid = %lu, %s\n", pthread_self(), buff);

    return NULL;
}

void *pthread_func2(void *major)
{
    dev_t dev;
    dev = makedev(*(int *)major, 1);
    mknod("./chardevice2", S_IFCHR, dev);
    
    int fd = open("./chardevice2", O_RDWR);
    printf("func2: tid = %lu, fd = %d\n", pthread_self(), fd);
    char buff[128];
    write(fd, "THIS IS A SAMPLE CHARACTER DEVICE 2\n", sizeof("THIS IS A SAMPLE CHARACTER DEVICE 2\n"));
    read(fd, buff, sizeof("THIS IS A SAMPLE CHARACTER DEVICE 2\n"));
    printf("tid = %lu, %s\n", pthread_self(), buff);

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
	printf("Usage: executable <major number>\n");
	return 0;
    }

    dev_t dev = 0;
    int major = atoi(argv[1]);
    pthread_t pid1;
    pthread_t pid2;

    pthread_create(&pid1, NULL, pthread_func1, &major);
    pthread_create(&pid2, NULL, pthread_func2, &major);
    pthread_join(pid1, NULL);
    pthread_join(pid2, NULL);
    return 0;
}
