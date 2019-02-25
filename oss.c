//current version does the following
//Runs 10 children, then parent terminates
//next step I believe is to add mandatory 2 second cutoff

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	
	printf("Begin the oss code\n");
	int shmid;
	key_t key;
	int *clockSeconds, *clockNano;
	long clockInc = 10001; //just as an example. Will eventually read from input file
	
	key = 9876;
	shmid = shmget(key, sizeof(int*) + sizeof(long*), IPC_CREAT | 0666); //this is where we create shared memory
	if(shmid < 0) {
		perror("shmget oss side");
		exit(1);
	}
	
	//attach ourselves to that shared memory
	clockSeconds = shmat(shmid, NULL, 0); //attempting to store 2 numbers in shared memory
	clockNano = clockSeconds + 1;
	if((clockSeconds == (int *) -1) || (clockNano == (int *) -1)) {
		perror("shmat");
		exit(1);
	}

	*clockSeconds = 0;
	*clockNano = 0;
	int makeChild = 10; //sample trigger, let's us know if we need to fork off another child or not
	//sleep(0.5);
	printf("Hello from OSS!\n");
	//int status;
	int temp;
	while(makeChild > 0) { //simulated clock is incremented by parent

		//printf("makeChild equals %d\n", makeChild);
		
		//if (*clockNano % 10 == 0)
			//printf("OSS time: %d:%d\n", *clockSeconds, *clockNano);

		//increment clock
		//waitpid to see if a child has ended
		//if (child has ended)
		//launch another child
		
		*clockNano += clockInc;
		if (*clockNano >= 1000000000) { //increment the next unit
			*clockSeconds += 1;
			*clockNano -= 1000000000;
		}
		
		temp = waitpid(0, NULL, WNOHANG);
		//printf("temp equals %d\n", temp);
		if (temp > 0) {
			printf("A child has ended\n");
			makeChild -= 1;
		}
		if (temp > 0 || temp < 0){ //if a child has ended or if no child is running
			if (makeChild > 0) {
				printf("Lets make a child!\n");
				//makeChild -= 1;
				pid_t pid;
				pid = fork();
				
				if (pid == 0) { //child
					execl ("user", "user", "200", NULL);
					//execl ("hw", "hw", NULL); //hello world program, used for testing
					perror("Error, execl function failed: ");
					exit(1);
				}
				else if (pid > 0) {
					continue;
				}
				else {
					perror("Error, could not fork child: ");
					exit(1);
				}				
			}
		}
		if (temp < 0) {
			//perror("waitpid");
			//exit(1);
			printf("No more children to make...\n");
		}
	}
	
	
	//destroy shared memory
	printf("clock time equals %d:%d\n", *clockSeconds, *clockNano);
	int ctl_return = shmctl(shmid, IPC_RMID, NULL);
	if (ctl_return == -1) {
		perror("shmctl for removel: ");
		exit(1);
	}
	printf("The parent is now done\n");

	return 0;
}
