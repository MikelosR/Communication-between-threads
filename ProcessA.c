/*process A*/
#include "task.h"
int exit_all = 0;

int main(int argc, char *argv[] ) {
	
    int retval;
	int shmem_ID;
	//////////////////////////////////////threads
	pthread_t write_thread_var;
	int res1;

	pthread_t read_thread_var;
  	int res2;
	///////////////////////////////////////
	Buffers* buffer1;
	//remove the named semaphores referred to by name.
    //The semaphore name is removed immediately. The semaphore is
    //destroyed once all other processes that have the semaphore open close it.
	sem_unlink(SEM_NAME);
	sem_unlink(SEM_NAME2);
	
	//create shared memory
	shmem_ID = shmget(IPC_PRIVATE, sizeof(Buffers), SEGMENTPERM);
	printf("shmem_ID: %d\n", shmem_ID);

	/* attach to the segment to get a pointer to it: */
	buffer1 = (Buffers*)shmat(shmem_ID, NULL, 0);
	if (buffer1 == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
	}
	memset(buffer1->bufferA, '\0', sizeof(buffer1->bufferA));
	memset(buffer1->bufferB, '\0', sizeof(buffer1->bufferB));
	printf("Shared memory segment with id %d attached at %p\n", shmem_ID, buffer1);
    ///////////////////////////////////////////////////////////////////////////

	//create the semaphores
	/////////////////////////////////////////////////////////////////////
	sem_t* semaphore_end_to_write = sem_open(SEM_NAME2, O_CREAT, SEM_PERMS, 0);
	if (semaphore_end_to_write == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);
    }

	sem_t* semaphore_ready_for_read = sem_open(SEM_NAME, O_CREAT, SEM_PERMS, 0);
	if (semaphore_ready_for_read == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);
    }
	
	//το struct που θα μπεί ως όρισμα στην write_thread
	Messages_Data_Write MDW;
	MDW.bufferWriter = buffer1->bufferA;
	MDW.were_sent = 0;
	//το struct που θα μπεί ως όρισμα στην read_thread
	Messages_Data_Read MDR;
	MDR.bufferReader = buffer1->bufferB;
	MDR.segments = 0;
	MDR.receive_message = 0;

	//////////////////////////////////////////////////////////////////////create threads
	//char bufferA[15] write for procces A
	res1 = pthread_create(&write_thread_var, NULL, write_thread, &MDW);
	if (res1 != 0) {
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	}
	//τεχνητό wait για να προλάβει να πάει πρώτη η διεργασία Β στο read_thread
	//στο main thread του proccesB ξεμπλοκάρω τεχνητά main thread του proccesΑ, αφού το proccesA
	//μπήκε στην read_thread
	sem_wait(semaphore_ready_for_read);

	//char bufferA[15] read for procces A,
	res2 = pthread_create(&read_thread_var, NULL, read_thread, &MDR);
	if (res2 != 0) {
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	}

	//===>>>>περίμενε το read
	res1 = pthread_join(read_thread_var, NULL);
	if (res1 != 0) {
		perror("Thread join failed");
		exit(EXIT_FAILURE);
	}
	printf("Thread (read_thread_var) with number %u complete the process A\n", (unsigned int)read_thread_var);

	//σταμάτα την write αφού επέστρεψε η read (ήρθε #BYE#)
	pthread_cancel(write_thread_var);

	//join write thread
	res2 = pthread_join(write_thread_var, NULL);
	if (res2 != 0) {
		perror("Thread join failed");
		exit(EXIT_FAILURE);
	}
	printf("Thread (write_thread_var) with number %u complete the process A\n",(unsigned int)write_thread_var);

	printf("Process A sent %d messages \n", MDW.were_sent);
	printf("Process A receive %d messages \n", MDR.receive_message);
	printf("Process A receive %d segments \n", MDR.segments);
	//μεσος αριθμός πακέτων ανά μήνυμα
	double avg_seg = (double)MDR.segments/(double)MDR.receive_message;
	printf("Average number of segments per message: %f \n",avg_seg);
	printf("End of call\n");
	//detach shared memory
	if (shmdt(buffer1) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	
	/* Remove segment. */
	int err = shmctl(shmem_ID, IPC_RMID, 0);
	if (err == -1) {
		perror("Removal.");
	} else {
		printf("Removed. %d\n",err);
	}

	//close semaphores
	if (sem_close(semaphore_ready_for_read) < 0)
        perror("sem_close(3) failed");
	if (sem_close(semaphore_end_to_write) < 0)
        perror("sem_close(3) failed");
	if (sem_unlink(SEM_NAME) < 0)
    	perror("sem_unlink(3) failed");
	if (sem_unlink(SEM_NAME2) < 0)
    	perror("sem_unlink(3) failed");
	exit(EXIT_SUCCESS);
}
	