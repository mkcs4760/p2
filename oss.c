#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h> //used for sleep right now

#define SHSIZE 100


int main(int argc, char *argv[]) {
	
	int shmid;
	key_t key;
	char * shm;
	char *s;
	
	key = 9876;
	shmid = shmget(key, SHSIZE, IPC_CREAT | 0666); //this is where we create shared memory
	
	if(shmid < 0) {
		perror("shmget");
		exit(1);
	}
	
	shm = shmat(shmid, NULL, 0); //attach ourselves to that shared memory
	
	if(shm == (char *) -1) {
		perror("shmat");
		exit(1);
	}
	
	memcpy(shm, "Hello World", 11); //write something to shared memory
	
	s = shm;
	s += 11; //the pointer is now at the end of the string we put in shared memory
	*s = 0; //add terminating 0 at end of string
	
	while(*shm != '*')
		sleep(1);
	
	int ctl_return = shmctl(shmid, IPC_RMID, NULL);
	if (ctl_return == -1) {
		perror("shmctl for removel: ");
		exit(1);
	}
	
	return 0;
}
