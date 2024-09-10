#include "task.h"
#define BUFFER_MESSAGE 15


void* write_thread(void* buffer) {
	int index = 0;
	char token;
	//παίρνω το struct για γράψιμο ή της διεργασίας Β (bufferB[15]) ή της διεργασίας A (bufferA[15])
	//ανάλογα ποια διεργασία με καλέσει
	Messages_Data_Write* bufferX = (Messages_Data_Write*)buffer;

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
	printf("text: >>\n");
	int val = -1;
	while (!exit_all){
		//διάβασε από terminal
		token = getchar();
		bufferX->bufferWriter[index] = token;
		index++;        
        //όταν το buffer γεμίσει
        if (index == 15){
			index = 0;
			//απελευθέρωση σημαφόρου για διάβασμα
			if (sem_post(semaphore_ready_for_read) < 0) {
       			perror("sem_post(3) error on semaphore_read");
    		}
			//περίμενε να τελειώσει η read
			if (sem_wait(semaphore_end_to_write) < 0) {	//====================================//1st stop
       			perror("sem_post(3) error on semaphore_write");
    		}
        }
        else if(token == '\n'){
			bufferX->bufferWriter[index] = '\0';
			index = 0;
			fflush(stdin);
			bufferX->were_sent++;
			if(strcmp(bufferX->bufferWriter, "#BYE#\n") == 0){
				exit_all = 1; //break while
			}
			//απελευθέρωση σημαφόρου για διάβασμα
			if (sem_post(semaphore_ready_for_read) < 0) {
       			perror("sem_post(3) error on semaphore_read");
    		}
			//περίμενε να τελειώσει η read
			if (sem_wait(semaphore_end_to_write) < 0) {	//====================================//2st stop
       			perror("sem_post(3) error on semaphore_write");
    		}
		}
    }
	pthread_exit(0);
}
