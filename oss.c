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
#include <signal.h>
#include <sys/time.h>

int shmid;

static void myhandler(int s) {
    char message[41] = "Program reached 2 second limit. Program ";
    int errsave;
    errsave = errno;
    write(STDERR_FILENO, &message, 40);
    errno = errsave;
   
    //destroy shared memory
	int ctl_return = shmctl(shmid, IPC_RMID, NULL);
	if (ctl_return == -1) {
		perror("shmctl for removel: ");
		exit(1);
	}
   
   kill(-1*getpid(), SIGKILL);
}
//function taken from textbook as instructed by professor
static int setupinterrupt(void) {          /* set up myhandler for  SIGPROF */
    struct sigaction act;
    act.sa_handler = myhandler;
    act.sa_flags = 0;
    return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
}
//function taken from textbook as instructed by professor
static int setupitimer(void) {    /* set ITIMER_PROF for 2-second intervals */
    struct itimerval value;
    value.it_interval.tv_sec = 2;
    value.it_interval.tv_usec = 0;
    value.it_value = value.it_interval;
    return (setitimer(ITIMER_PROF, &value, NULL));
}


int main(int argc, char *argv[]) {
	//set up 2 second timer first thing
    if (setupinterrupt()) {
		perror("Failed to set up handler for SIGPROF");
		return 1;
    }
    if (setupitimer() == -1) {
		perror("Failed to set up the ITIMER_PROF interval timer"); 
		return 1;
    }
	
	char inputFileName[] = "input.txt";
	char outputFileName[] = "output.txt";
	int maxKidsTotal = 4;
	int maxKidsAtATime = 2;
	
	//need to add code to allow us to print program name in error messages
	
	//process getopt arguments
	

	
	int option;
	while ((option = getopt(argc, argv, "hn:s:i:o:")) != -1) {
		switch (option) {
			case 'h' :	printf("Help page for OS_Klein_project2\n"); //for h, we print data to the screen
						printf("Blah blah blah write stuff here\n");
						//WRITE STUFF HERE BEFORE YOU SUBMIT!!
						exit(0);
						break;
			case 'n' :	maxKidsTotal = atoi(optarg);
						break;
			case 's' :	if (atoi(optarg) <= 20) {
							maxKidsAtATime = atoi(optarg);
						}
						else {
							perror("Cannot allow more then 20 process at a time.");
							exit(1);
						}
						break;
			case 'i' :	strcpy(inputFileName, optarg); //for i, we specify input file name
						break;
			case 'o' :	strcpy(outputFileName, optarg); //for o, we specify output file name
						break;
			default :	errno = 22; //anything else is an invalid argument
						perror("You entered an invalid argument");
						exit(1);
		}
	}
	
	
	printf("Our values are %s : %s : %d : %d\n", inputFileName, outputFileName, maxKidsTotal, maxKidsAtATime);
	
	//open input file
	FILE *input;
	input = fopen(inputFileName, "r");
	if (input == NULL) {
		printf("Can't open file %s\n", inputFileName);
		perror("Can't open file");
		exit(1);
	}
	printf("Opened file %s\n", inputFileName);
		
	
	
	
	
	
	
	
	printf("Begin the oss code\n");
	//int shmid;
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
	int makeChild = 10000; //sample trigger, let's us know if we need to fork off another child or not
	//sleep(0.5);
	printf("Hello from OSS!\n");
	//int status;
	int temp;
	while(makeChild > 0) { //simulated clock is incremented by parent

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
			//printf("A child has ended\n");
			makeChild -= 1;
		}
		if (temp > 0 || temp < 0){ //if a child has ended or if no child is running
			if (makeChild > 0) {
				//printf("Lets make a child!\n");
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
