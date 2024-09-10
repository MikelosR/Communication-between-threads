#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <pthread.h>

#define SEGMENTPERM 0666
#define SEM_NAME "semaphore_read"
#define SEM_NAME2 "semaphore_write"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define BUFFER_MESSAGE 15

typedef struct {
	//write for procces A, read for procces B
    char bufferA[15];
	//read for procces A, write for procces B
    char bufferB[15];
}Buffers;

typedef struct Messages_Data_Write{
	int were_sent;
	char* bufferWriter;
}Messages_Data_Write;

typedef struct Messages_Data_Read{
	int receive_message;
	int segments;
	char* bufferReader;
}Messages_Data_Read;

extern int exit_all;
void* read_thread(void* buffer);

void* write_thread(void* buffer);