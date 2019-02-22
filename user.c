#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h> //used for sleep right now


//#define SHSIZE 100


int main(int argc, char *argv[]) {
	
	int shmid;
	key_t key;
	//char * shm;
	//char *s;
	int *a, *b;
	
	key = 9876;
	shmid = shmget(key, 2*sizeof(int*), 0666);
	
	if(shmid < 0) {
		perror("shmget user side");
		exit(1);
	}
	
	//shm = shmat(shmid, NULL, 0); //attach ourselves to that shared memory
	a = shmat(shmid, NULL, 0); //attempting to store 2 integers in shared memory
	b = a + 1;
	
	if((*a == -1) || (*b == -1)) {
		perror("shmat");
		exit(1);
	}
	printf("Hello from user!\n");
	while(*a != 25) {
		//printf("%d  %d", *a, *b);
		printf("From user, a = %d\n", *a);		
		sleep(1);
	}
	
	/*for(s = shm; *s != 0; s++) {
		printf("%c", *s);
	}	
	
	printf("\n");
	*shm = '*';*/
	
	return 0;
}
