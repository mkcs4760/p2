#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h> 

int main(int argc, char *argv[]) {
	
	//printf("%s\n", argv[1]); //proof of concept, that the argument passes over
	
	int shmid;
	key_t key;

	int *clockSeconds, *clockNano;
	
	key = 9876;
	shmid = shmget(key, sizeof(int*) + sizeof(long*), 0666);
	
	if(shmid < 0) {
		perror("shmget user side");
		exit(1);
	}
	
	//attach ourselves to that shared memory
	clockSeconds = shmat(shmid, NULL, 0); //attempting to store 2 numbers in shared memory
	clockNano = clockSeconds + 1;
	
	if((clockSeconds == (int *) -1) || (clockNano == (int *) -1)) {
		perror("shmat");
		exit(1);
	}
	
	//printf("Hello from user! We're at time %d:%d\n", *clockSeconds, *clockNano);
	//while(*clockSeconds != 2) {
	//	printf("User time: %d:%d\n", *clockSeconds, *clockNano);		
	//}
	
	return 0;
}
