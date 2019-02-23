#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h> //used for sleep right now


//#define SHSIZE 100


int main(int argc, char *argv[]) {
	
	printf("%s\n", argv[1]);
	
	int shmid;
	key_t key;
	//char * shm;
	//char *s;
	int *clockSeconds, *clockNano;
	
	key = 9876;
	shmid = shmget(key, sizeof(int*) + sizeof(long*), 0666);
	
	if(shmid < 0) {
		perror("shmget user side");
		exit(1);
	}
	
	//shm = shmat(shmid, NULL, 0); //attach ourselves to that shared memory
	clockSeconds = shmat(shmid, NULL, 0); //attempting to store 2 integers in shared memory
	clockNano = clockSeconds + 1;
	
	if((clockSeconds == (int *) -1) || (clockNano == (int *) -1)) {
		perror("shmat");
		exit(1);
	}
	
	printf("Hello from user!\n");
	while(*clockSeconds != 3) {
		printf("User time: %d:%d\n", *clockSeconds, *clockNano);		
		sleep(1);
	}
	
	/*for(s = shm; *s != 0; s++) {
		printf("%c", *s);
	}	
	
	printf("\n");
	*shm = '*';*/
	
	return 0;
}
