#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <unistd.h>

//#define SHSIZE 100


int main(int argc, char *argv[]) {
	
	int shmid;
	key_t key;
	//char * shm;
	//char *s;
	int *clockSeconds, *clockNano;
	long clockInc = 100000001; //just as an example. Will eventually read from input file
	
	key = 9876;
	shmid = shmget(key, sizeof(int*) + sizeof(long*), IPC_CREAT | 0666); //this is where we create shared memory
	
	if(shmid < 0) {
		perror("shmget oss side");
		exit(1);
	}
	
	//attach ourselves to that shared memory
	clockSeconds = shmat(shmid, NULL, 0); //attempting to store 2 integers in shared memory
	clockNano = clockSeconds + 1;
	
	if((clockSeconds == (int *) -1) || (clockNano == (int *) -1)) {
		perror("shmat");
		exit(1);
	}
	
	pid_t pid;
	pid = fork();
	if (pid > 0) {//parent
		printf("I am the parent\n");
		//wait(NULL);
		//printf("Parent done waiting\n");
	
		*clockSeconds = 0;
		*clockNano = 0;
	
		printf("Hello from OSS!\n");
		while(*clockSeconds != 3) {
			*clockNano += clockInc;
			if (*clockNano >= 1000000000) { //increment the next unit
				*clockSeconds += 1;
				*clockNano -= 1000000000;
			}
			//printf("OSS time: %d:%d\n", *clockSeconds, *clockNano);
			//printf("From oss, clockSeconds = %d\n", *clockSeconds);
			sleep(1);
		}
	
		int ctl_return = shmctl(shmid, IPC_RMID, NULL);
		if (ctl_return == -1) {
			perror("shmctl for removel: ");
			exit(1);
		}
		printf("The parent is now done\n");
		//continue;
	}
	else if (pid == 0) { //child
		printf("I am the child\n");
		//sleep(4);
		execl ("user", "user", NULL);
		perror("Error, execl function failed: ");
		exit(1);
	}
	else {
		perror("Error, could not fork child: ");
		exit(1);
	}
	
	
	return 0;
}
