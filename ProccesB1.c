/*process B*/

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
	/////////////////////////////////////
	Buffers* buffer1;
	
	if(argc <= 1) printf("Give and Shared Memory ID\n");
    else shmem_ID = atoi(argv[1]);
	printf("shmem_ID: %d\n", shmem_ID);

	/* attach to the segment to get a pointer to it: */
	buffer1 = (Buffers*)shmat(shmem_ID, NULL, 0);
	if (buffer1 == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
	}
	printf("Shared memory segment with id %d attached at %p\n", shmem_ID, buffer1);
    /////////////////////////////////////////////////////////////////////

	//create the semaphores
	/////////////////////////////////////////////////////////////////////
	sem_t *semaphore_end_to_write = sem_open(SEM_NAME2, O_RDWR); 
    if (semaphore_end_to_write == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
	sem_t *semaphore_ready_for_read = sem_open(SEM_NAME, O_RDWR); 
    if (semaphore_ready_for_read == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
	//το struct που θα μπεί ως όρισμα στην write_thread
	Messages_Data_Write MDW;
	MDW.bufferWriter = buffer1->bufferB;
	MDW.were_sent = 0;
	//το struct που θα μπεί ως όρισμα στην read_thread
	Messages_Data_Read MDR;
	MDR.bufferReader = buffer1->bufferA;
	MDR.segments = 0;
	MDR.receive_message = 0;
	
	//////////////////////////////////////////////////////////////////////create index
	//char bufferA[15] read for procces B
	res2 = pthread_create(&read_thread_var, NULL, read_thread, &MDR);
	if (res2 != 0) {
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	}
	//σηκώνω σημαφόρο να ξεκολλήσει το procces A και να καλέσει και αυτή το read thread, αφού εγώ μπήκα πρώτος στην read
	sem_post(semaphore_ready_for_read);
	
	//(rendez-vous point)
	
	res1 = pthread_create(&write_thread_var, NULL, write_thread, &MDW);
	if (res1 != 0) {
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	}

	//===>>>>περίμενε το read
	res1 = pthread_join(read_thread_var, NULL);
	if (res1 != 0) {
		perror("Thread join failed");
		exit(EXIT_FAILURE);
	}
	printf("Thread (read_thread_var) with number %u complete the process B\n",(unsigned int)read_thread_var);

	//σταμάτα την write αφού επέστρεψε η read (ήρθε #BYE#)
	pthread_cancel(write_thread_var);

	//το join είναι περιττό γιατί κάνουμε pthread_cancel στο write thread που έχει κολήσει στην getchar()
	/*res2 = pthread_join(write_thread_var, NULL);
	if (res2 != 0) {
		perror("Thread join failed");
		exit(EXIT_FAILURE);
	}*/
	printf("Thread (write_thread_var) with number %u complete the process B\n", (unsigned int)write_thread_var);

	printf("Process B sent %d messages \n", MDW.were_sent);
	printf("Process B receive %d messages \n", MDR.receive_message);
	printf("Process B receive %d segments \n", MDR.segments);
	//μεσος αριθμός πακέτων ανά μήνυμα
	double avg_seg = (double)MDR.segments/(double)MDR.receive_message;
	printf("Average number of segments per message: %f \n",avg_seg);
	printf("End of call\n");
   

	if (shmdt(buffer1) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}

	if (sem_close(semaphore_ready_for_read) < 0)
        perror("sem_close(3) failed");
	if (sem_close(semaphore_end_to_write) < 0)
        perror("sem_close(3) failed");

	exit(EXIT_SUCCESS);
}