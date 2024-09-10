#include "task.h"

void* read_thread(void* buffer) {
	char* BigBuffer = malloc(4096*sizeof(char));
	//βάζω όλες οι θέσεις να είναι κενές
	memset(BigBuffer, '\0', sizeof(BigBuffer));
	//παίρνω το struct για γράψιμο ή της διεργασίας Β (bufferB[15]) ή της διεργασίας A (bufferA[15])
	//ανάλογα ποια διεργασία με καλέσει
	Messages_Data_Read* bufferX = (Messages_Data_Read*)buffer;
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
	while (!exit_all){       
		//ζητάω τον σημαφόρο
		//εδώ περιμένουν τα read threads των 2 processes
		if (sem_wait(semaphore_ready_for_read) < 0) {
			perror("sem_wait(3) failed on semaphore_read");
			break;
		}
		//αν δεν έχει δεδομένα το buffer σου, δώσε τον σημαφόρο στo thread της άλλης διεργασίας
		if (strlen(bufferX->bufferReader) == 0) {
			sem_post(semaphore_ready_for_read);
		}
		else{
			//βάλε το string στον μεγάλο buffer
			strcat(BigBuffer, bufferX->bufferReader);
			//ήρθε ένα segment - μηνύμα
			bufferX->segments++;
			//αν υπάρχει αλλαγή γραμμής σε αυτή την θέση σημαίνει ότι το έστειλε το μήνυμα ο sender
			int len = strlen(bufferX->bufferReader);
			if(bufferX->bufferReader[len-1] == '\n'){
				printf("Other terminal: >> ");
				printf("%s", BigBuffer);
				//μετρητής εισερχόμενων μηνυμάτων
				bufferX->receive_message++;
				//καθάρισε τον BigBuffer
				memset(BigBuffer, '\0', sizeof(BigBuffer));
			} 
			//τσέκαρε για exit message
			if(strcmp(bufferX->bufferReader, "#BYE#\n") == 0){
				memset(BigBuffer, '\0', sizeof(BigBuffer));
				exit_all = 1;//break while
				//άφησε και το thread της άλλης διεργασίας να περάσει για να τερματήσει
				sem_post(semaphore_ready_for_read);
			}
			//καθάρισε τον buffer
			memset(bufferX->bufferReader, '\0', sizeof(bufferX->bufferReader));

			//σειρά του write
			if (sem_post(semaphore_end_to_write) < 0) {
				perror("sem_post(3) error on semaphore_write");
			}
		}
	}
	free(BigBuffer);
		
	pthread_exit(0);
}
