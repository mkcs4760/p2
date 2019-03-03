//everything seems to work save the 2 second timer... not always...
//also test a bunch

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
#include <ctype.h>
#include <stdbool.h>

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
   
    kill(-1*getpid(), SIGKILL); //doesn't always kill right away
}
//function taken from textbook as instructed by professor
static int setupinterrupt(void) { //set up myhandler for  SIGPROF
    struct sigaction act;
    act.sa_handler = myhandler;
    act.sa_flags = 0;
    return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
}
//function taken from textbook as instructed by professor
static int setupitimer(void) { // set ITIMER_PROF for 2-second intervals
    struct itimerval value;
    value.it_interval.tv_sec = 2;
    value.it_interval.tv_usec = 0;
    value.it_value = value.it_interval;
    return (setitimer(ITIMER_PROF, &value, NULL));
}

//takes in program name and error string, and runs error message procedure
void errorMessage(char programName[100], char errorString[100]){
	char errorFinal[200];
	sprintf(errorFinal, "%s : Error : %s", programName, errorString);
	perror(errorFinal);
	
	//destroy shared memory
	int ctl_return = shmctl(shmid, IPC_RMID, NULL);
	if (ctl_return == -1) {
		perror("shmctl for removel: ");
		exit(1);
	}
	kill(-1*getpid(), SIGKILL);
}

//a function to check if string contains ending whitespace, and if so remove it
void removeSpaces(char* s) {
	int length = strlen(s);
	char test = s[length - 1];
	if (isspace(test)) {
		s[strlen(s)-1] = '\0'; //remove ending whitespace
	}
}

//a function designed to read a line containing a single number and process it
int readOneNumber(FILE *input, char programName[100]) {
	char line[100];
	char *token;
	fgets(line, 100, input);
	if (line[0] == '\0') { //if there are no more lines, then we have an error
		errno = 1;
		perror("Error reading in one number");
		//errorMessage(programName, "Invalid input file format. Expected more lines then read. ");
	}
	token = strtok(line, " "); //read in a single numer
	removeSpaces(token); //remove any hanging whitespace
	int ourValue = atoi(token);
	if ((token = strtok(NULL, " ")) != NULL) { //check if there is anything after this number
		if (token[0] == '\n') { //if what remains is just new line character, no problem
			return ourValue;
		}
		else {
			return -1; //else, the function failed and -1 is returned
		}
	}
	else {
		return ourValue;
	}
}


int main(int argc, char *argv[]) {
	//this section of code allows us to print the program name in error messages
	char programName[100];
	strcpy(programName, argv[0]);
	if (programName[0] == '.' && programName[1] == '/') {
		memmove(programName, programName + 2, strlen(programName));
	}
	
	//set up 2 second timer
    if (setupinterrupt()) {
		errno = 125;
		errorMessage(programName, "Failed to set up 2 second timer. ");
    }
    if (setupitimer() == -1) {
		errno = 125;
		errorMessage(programName, "Failed to set up 2 second timer. ");
    }
	
	char inputFileName[] = "input.txt";
	char outputFileName[] = "output.txt";
	int maxKidsTotal = 10;
	int maxKidsAtATime = 2;

	//first we process the getopt arguments
	int option;
	while ((option = getopt(argc, argv, "hn:s:i:o:")) != -1) {
		switch (option) {
			case 'h' :	printf("Help page for OS_Klein_project2\n"); //for h, we print data to the screen
						printf("Blah blah blah write stuff here\n");
						//WRITE STUFF HERE BEFORE YOU SUBMIT!!
						exit(0);
						break;
			case 'n' :	maxKidsTotal = atoi(optarg); //for n, we set the maximum number of children we will fork
						break;
			case 's' :	if (atoi(optarg) <= 19) { //for s, we set the maximum of child processes we will have at a time
							maxKidsAtATime = atoi(optarg);
						}
						else {
							perror("Cannot allow more then 19 process at a time."); //the parent is running, so there's already 1 process running
							exit(1);
						}
						break;
			case 'i' :	strcpy(inputFileName, optarg); //for i, we specify input file name
						break;
			case 'o' :	strcpy(outputFileName, optarg); //for o, we specify output file name
						break;
			default :	errno = 22; //anything else is an invalid argument
						errorMessage(programName, "You entered an invalid argument. ");
		}
	}
	
	//open input file
	FILE *input;
	input = fopen(inputFileName, "r");
	if (input == NULL) {
		errno = 2;
		errorMessage(programName, "Cannot open desired file. ");
	}

	//int shmid;
	key_t key;
	int *clockSeconds, *clockNano;
	long clockInc = readOneNumber(input, programName); //10001; //just as an example. Will eventually read from input file
	
	key = 9876;
	shmid = shmget(key, sizeof(int*) + sizeof(long*), IPC_CREAT | 0666); //this is where we create shared memory
	if(shmid < 0) {
		errorMessage(programName, "Function shmget failed. ");
	}
	
	//attach ourselves to that shared memory
	clockSeconds = shmat(shmid, NULL, 0); //attempting to store 2 numbers in shared memory
	clockNano = clockSeconds + 1;
	if((clockSeconds == (int *) -1) || (clockNano == (int *) -1)) {
		errorMessage(programName, "Function shmat failed. ");
	}

	*clockSeconds = 0;
	*clockNano = 0;
	int numKidsRunning = 0;
	int numKidsDone = 0;
	bool lineWaiting = false;
	int counter = 0;
	int singleNum;
	int fullLine[3];
	char* token;
	bool endOfFile = false;
	//int status;
	int temp;
	FILE *output;
	output = fopen(outputFileName, "w");
	//loop until we're done
	while((numKidsDone < maxKidsTotal) && !((numKidsRunning == 0) && endOfFile)) { //simulated clock is incremented by parent
		//increment clock
		//waitpid to see if a child has ended
		//if (child has ended)
		//launch another child
		
		*clockNano += clockInc;
		if (*clockNano >= 1000000000) { //increment the next unit
			*clockSeconds += 1;
			*clockNano -= 1000000000;
		}
		temp = waitpid(-1, NULL, WNOHANG);
		//if a child has ended, return pid
		//if children are running, return 0
		//if no children are running, return -1
		if (temp > 0) { //a child has ended
			//write to output file the time this process ended
			//printf("Child %d ended at %d:%d\n", temp, *clockSeconds, *clockNano);
			fprintf(output, "Child %d ended at %d:%d\n", temp, *clockSeconds, *clockNano);
			numKidsDone += 1;
			numKidsRunning -= 1;
		}
		if (((numKidsDone + numKidsRunning) < maxKidsTotal) && (numKidsRunning < maxKidsAtATime)) { //if you still have kids to run and there's room to run them
			if (lineWaiting == false) {
				char line[100];
				lineWaiting = true;
				counter = 0;
				char *value = fgets(line, 100, input); //get line of numbers
				if (value == NULL) {
					//if there are no more lines, then we reached the EOF
					endOfFile = true;
				}
				else {
					token = strtok(line, " ");
					while (token != NULL && token[0] != '\n' && counter < 3) {
						singleNum = atoi(token);
						fullLine[counter] = singleNum;
						counter++;
						token = strtok(NULL, " ");
					}
				}
				//what if there are too many numbers?
				//what if there are too few numbers?
			}
				
			if (endOfFile == false) {
					
				if ((*clockSeconds > fullLine[0]) || ((*clockSeconds == fullLine[0]) && (*clockNano >= fullLine[1]))) {
					//it's time to make a child
					lineWaiting = false;
					pid_t pid;
					pid = fork();
					
					if (pid == 0) { //child
						char buffer[11];
						sprintf(buffer, "%d", fullLine[2]);
						execl ("user", "user", buffer, NULL);
						perror("Error, execl function failed: ");
						exit(1);
					}
					else if (pid > 0) {
						numKidsRunning += 1;
						//write to output file the time this process was launched
						//printf("Created child %d at %d:%d to last %d\n", pid, *clockSeconds, *clockNano, fullLine[2]);
						fprintf(output, "Created child %d at %d:%d to last %d\n", pid, *clockSeconds, *clockNano, fullLine[2]);
						continue;
					}
				}
			}
		}
	}
	
	fclose(output);
	
	//destroy shared memory
	printf("Parent terminating %d:%d\n", *clockSeconds, *clockNano);
	int ctl_return = shmctl(shmid, IPC_RMID, NULL);
	if (ctl_return == -1) {
		errorMessage(programName, "Function scmctl failed. ");
	}

	return 0;
}