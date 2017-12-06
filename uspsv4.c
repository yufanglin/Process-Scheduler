/*
Yufang Lin
Duck ID: yufang
CIS 415 Project 1 part 2
Referenced: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html : for fork() and execvp()
Referenced: https://gist.github.com/ArnonEilat/4471278 
			&   https://www.tutorialspoint.com/data_structures_algorithms/dsa_queue.htm  : for Queue help
Referenced: LPE for alarms 
Referenced: http://stackoverflow.com/questions/5288910/sigprocmask-blocking-signals-in-unix     for masking signals
*/

#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>

#include <unistd.h>  
#include <fcntl.h> 
#include <signal.h>
#include <time.h>

#include <stdbool.h>

#include "p1fxns.h"
#include "queue.h"

#define UNUSED __attribute__ ((unused))



/////// Global variables //////
int numberProcs = 0;				//number of processes
int activeProcs = 0;
int quantum;						//global variable for the time slice
bool quantumFlag = false;			//check to make sure there is a quantum (either env or stated)

Queue *currentQueue;
Process *currentProc = NULL;
												//45
char *processInfoLabel = "Name                PID     Status   Virtual Memory Size           Runtime              RCHAR  WCHAR\n";
char *separatorLabel = "------------------------------------------------------------------------------------------------------\n";
volatile int SIGUSR1_check = 0;

sigset_t oldSet;

///////FUNCTIONS/////

//static void print_queue(Queue *queue);
////////// PROC FILE PARSERS FUNCTIONS ///////////
static char *getIOInfo(pid_t pid , int *totalLen);
static char *getStatusInfo(pid_t pid, int *totalLen);
static char *getSchedInfo(pid_t pid, int *totalLen);
static char *getProcessInfo(Process *process);

//SIGNAL
static void signal_handler_sigusr1(int signal);
// static int signal_handler_sigchld_wait(Task *task);
static void subscribe_to_signals();
static void signal_handler_sigchld(UNUSED int signal); 
static void signal_handler_alarm(UNUSED int signal);
// static int kill_sigusr1(Task *task);
// static int kill_sigstop(Task *task);
// static int kill_sigcont(Task *task);

//PARSE
static void split_Commands(char *commandLine, char **splitCommandArray);
static void Arguments(int argc, char *argv[], int *quantum, char **commandArray, int *numCommands);


// static void print_queue(Queue *queue){
// 	/*
// 	* Print queue
// 	*/
// 	printf("queue size: %d\n", queue->size);

// 	printf("queue front commandLine: %s pid: %d\n", queue->front->proc->work, queue->front->proc->pid);
// 	printf("queue rear commandLine: %s pid: %d\n", queue->rear->proc->work, queue->rear->proc->pid);

// }

///////// PARSE FUNCTIONS /////////
static void clear_buffer(char *buffer, int size){
	int i;
	for(i=0; i < size; i++){
		buffer[i] = '\0';
	}
}
static void Arguments(int argc, char *argv[], int *quantum, char **commandArray, int *numCommands){
	/*
	takes in the command line arguments and gets the contents
	*/
	//parsing workload variables
	int file = 0;
	int size = 160;
	char buffer[size];

	char newLine = '\n';

	//getting command line variables
	int index = 0;
	char *arg = NULL;
	
	for(index = 1; index < argc; index +=1){
		arg = argv[index];

		//check for quantum
		if (p1strneq("--quantum=",arg, 10) == 1)
		{	
			//get the actual time
			char *num = &(arg[10]);
			//convert string time to integer time
			*quantum = p1atoi(num);
			quantumFlag = true;
			//printf("%s\n", num);
			//printf("%d\n", quantum);

		}
		//check if file exist
		else if (access(arg, F_OK ) != -1)
		{
			file = open(arg,0);
			//variable for numCommands
			int i = 0;
			clear_buffer(buffer, size);
			while(p1getline(file, buffer, size)){
				//printf("buffer: %s\n",buffer);
			
				//find the length of the buffer to get rid of \n
				int lineEnd = p1strlen(buffer);
				if(p1strchr(buffer, newLine) != -1){
					buffer[lineEnd-1] = 0;
				}
				else{
					buffer[lineEnd]='\0';
				}

				commandArray[i] = p1strdup(buffer);
				//increment num of commands count
				i += 1; 

				clear_buffer(buffer,size);
				
			}

			*numCommands = i;
		}
		//error
		else{
			p1perror(1, "Improper inputs\n");
			exit(1);
		}


	}

	//check to see if file is still 0 (meaning workload file)
	if(file == 0){
		int i = 0;
			clear_buffer(buffer, size);
			while(p1getline(file, buffer, size)){
				//printf("buffer: %s\n",buffer);
			
				//find the length of the buffer to get rid of \n
				int lineEnd = p1strlen(buffer);
				if(p1strchr(buffer, newLine) != -1){
					buffer[lineEnd-1] = 0;
				}
				else{
					buffer[lineEnd]='\0';
				}

				commandArray[i] = p1strdup(buffer);
				//increment num of commands count
				i += 1; 

				clear_buffer(buffer,size);
				
			}

			*numCommands = i;
	}

	//close file
	if(file >= 0){
		close(file);
	}
	
}



static void split_Commands(char *commandLine, char **splitCommandArray){
	/*
	outputs command line into an array.
	*/
	//commandline index
	int index = 0;
	//the position in the command line
	int position = 0;
	//temporary storage for the split command
	char *commandPart = (char *)malloc(sizeof(char *)*(p1strlen(commandLine)+1));


	//check if memory allocated
	if(commandPart){
		//loop until position reaches end of commandline
		while(position >-1){

			position = p1getword(commandLine, position, commandPart);
			//printf("%d position command: %s\n", position, commandPart);
			if(position > -1){
				//if(commandPart[position])
				splitCommandArray[index] = p1strdup(commandPart);
				//printf("split command - index: %d, word: %s\n", index, *splitCommandArray);
			}
			index++;
		}

		//add a NULL to the array
		splitCommandArray[index-1] = NULL;
		// index--;
		// while(index >= 0){
		// 	printf("commandWords: %s at Index: %d\n", splitCommandArray[index], index);
		// 	index--;
		// }

		//printf("free commandPart\n");
		free(commandPart);
		// for(int i= 0; splitCommandArray[i] != '\0'; i++){
		// 	printf("entire splitCommandArray after free: %s\n: ", splitCommandArray[i]);
		// }
		
	}
	else{
		p1perror(1, "Malloc failed\n");
		exit(1);
	}
	

}


////////// PROC FILE PARSERS FUNCTIONS ///////////
static char *getIOInfo(pid_t pid , int *totalLen){
	/*
	* function returns formatted rchar and wchar
	*
	*/
	char procDir[] = "/proc/";
	char ioInfo[] = "/io";


	// open file variables
	int fd = 0;
	int size = 160;
	char filePath[80] = {'\0'};
	//buffers
	char buffer[size];		//overall buffer
	char emptySpace[5] = {'\0'};		//empty space for formatting
	char rchar[80];			//rchar buffer
	char wchar[80];			//wchar buffer

	//clear buffer & filePath to prevent bad overwrites
	clear_buffer(filePath, 80);

	//make complete filepath
	//first change pid to a string
	p1itoa(pid, filePath); 

	//next, combine string pid (which is no in filepath) with procDir
	p1strcat(procDir, filePath);

	//now procDir with ioInfo stringprintf("combining procDir and ioInfo\n");
	p1strcat(procDir, ioInfo);

	fd = open(procDir, O_RDONLY);

	int rLineEnd = 0;
	int wLineEnd = 0;
	//check if file opened
	if(fd > 0){
		int lineCounter = 0;
		//clear buffer to prevent bad overwrites 
		clear_buffer(buffer, size);

		while(p1getline(fd, buffer, size)){
			//this line is rchar
			if(lineCounter == 0){
				rLineEnd = p1strlen(buffer);
				buffer[rLineEnd - 1] = 0;
				//write the buffer into rchar;
				p1getword(buffer, 6, rchar);
				lineCounter++;
			}
			//this line is wchar
			else if(lineCounter == 1){
				wLineEnd = p1strlen(buffer);
				buffer[wLineEnd - 1] = 0;
				//write the buffer into rchar;
				p1getword(buffer, 6, wchar);
				lineCounter++;
			}
			else{
				break;
			}

			clear_buffer(buffer, size);
		}
	}
	else{
		p1perror(2, "can't open file");
		exit(-1);
	}

	//find the total length to malloc to
	*totalLen = rLineEnd + wLineEnd + 1;
	//malloc ioOut
	char *ioOut = (char *)malloc(sizeof(char *)**totalLen);
	char *ioIn = ioOut;		//point to the front of ioOut
	
	//write rchar into ioOut
	ioIn = p1strpack(rchar, -18, ' ', ioIn);
	//empty space inbetween
	ioIn = p1strpack(emptySpace, 2, ' ', ioIn); 
	//write wchar into io outwards
	ioIn = p1strpack(wchar, 20, ' ', ioIn);

	close(fd);

	return ioOut;
}


static char *getStatusInfo(pid_t pid, int *totalLen){
	/*
	* returns process state and virtual memory size
	*/

	char procDir[] = "/proc/";
	char statusInfo[] = "/status";


	// open file variables
	int fd = 0;
	int size = 160;
	char filePath[80] = {'\0'};

	//buffers	
	char buffer[size];			//overall buffer
	char state[80];				//state buffer 
	char vmsize[80];			//vmsize buffer

	//clear buffer & filePath to prevent bad overwrites
	clear_buffer(filePath, 80);

	//make complete filepath
	//first change pid to a string
	p1itoa(pid, filePath); 

	//next, combine string pid (which is no in filepath) with procDir
	p1strcat(procDir, filePath);

	//now procDir with ioInfo stringprintf("combining procDir and ioInfo\n");
	p1strcat(procDir, statusInfo);

	fd = open(procDir, O_RDONLY);

	int stateLineEnd = 0;
	int vmsizeLineEnd = 0;
	//check if file opened
	if(fd > 0){
		int lineCounter = 0;
		//clear buffer to prevent bad overwrites 
		clear_buffer(buffer, size);

		while(p1getline(fd, buffer, size)){
			if(lineCounter == 1){
				stateLineEnd = p1strlen(buffer);
				buffer[stateLineEnd - 1] = 0;
				//write the buffer into state;
				p1getword(buffer, 6, state);
			}
			else if(lineCounter == 12){
				vmsizeLineEnd = p1strlen(buffer);
				buffer[vmsizeLineEnd - 1] = 0;
				//write the buffer into vmsize;
				p1getword(buffer, 7, vmsize);
			}
			else if (lineCounter > 12){
				break;
			}

			lineCounter++;

			clear_buffer(buffer, size);
		}
	}
	else{
		p1perror(2, "can't open file");
		exit(-1);
	}

	*totalLen = stateLineEnd + vmsizeLineEnd + 1;
	char *statusOut = (char *)malloc(sizeof(char *)**totalLen);
	char *statusIn = statusOut;		//point to the front of ioOut

	statusIn = p1strpack(state, -2, ' ', statusIn);
	statusIn = p1strpack(vmsize, -15, ' ', statusIn);

	close(fd);

	return statusOut;
}


static char *getSchedInfo(pid_t pid, int *totalLen){
	/*
	* returns sum of execution runtime
	*/

	char procDir[] = "/proc/";
	char schedInfo[] = "/sched";


	// open file variables
	int fd = 0;
	int size = 160;
	char filePath[80] = {'\0'};

	//buffers	
	char buffer[size];			//overall buffer
	char runtime[80];			//exec runtim buffer

	//clear buffer & filePath to prevent bad overwrites
	clear_buffer(filePath, 80);

	//make complete filepath
	//first change pid to a string
	p1itoa(pid, filePath); 

	//next, combine string pid (which is no in filepath) with procDir
	p1strcat(procDir, filePath);

	//now procDir with ioInfo stringprintf("combining procDir and ioInfo\n");
	p1strcat(procDir, schedInfo);

	fd = open(procDir, O_RDONLY);

	int runtimeLineEnd = 0;
	//check if file opened
	if(fd > 0){
		int lineCounter = 0;
		//clear buffer to prevent bad overwrites 
		clear_buffer(buffer, size);

		while(p1getline(fd, buffer, size)){
			if(lineCounter == 4){
				runtimeLineEnd = p1strlen(buffer);
				buffer[runtimeLineEnd - 1] = 0;
				//write the buffer into rchar;
				p1getword(buffer, 46, runtime);
			}
			else if (lineCounter > 4){
				break;
			}

			lineCounter++;

			clear_buffer(buffer, size);
		}
	}
	else{
		p1perror(2, "can't open file");
		exit(-1);
	}

	*totalLen = runtimeLineEnd + 1;
	char *schedOut = (char *)malloc(sizeof(char *)**totalLen);
	char *schedIn = schedOut;		//point to the front of ioOut

	schedIn = p1strpack(runtime, -30, ' ', schedIn);

	close(fd);

	return schedOut;
}

static char *getProcessInfo(Process *process){
	/*
	*  get the overall info from three functions:
	*			getSchedInfo(), getIOInfo(), getStatus()
	*/

	//length of each property
	int ioLen = 0;
	int statusLen = 0;
	int schedLen = 0;
	int totalLen = 0;

	pid_t pid = process->pid;
	//get all the info
	char *ioInfo = getIOInfo(pid, &ioLen);
	char *statusInfo = getStatusInfo(pid, &statusLen);
	char *schedInfo = getSchedInfo(pid, &schedLen);

	//pid to string
	char pidStr[20] = {'\0'};
	p1itoa(pid, pidStr);

	//program name
	char *name = process->work;

	//newline
	char newLine[] = "\n";

	//get total length
	totalLen = ioLen + statusLen + schedLen + 1;
	//malloc for total info
	char *totalOut = (char *)malloc(sizeof(char *)*totalLen);
	char *totalIn = totalOut;

	///// combine strings 
	// first, name
	totalIn = p1strpack(name, 20, ' ', totalIn);
	//pids
	totalIn = p1strpack(pidStr, 8, ' ', totalIn);
	//state and vmsize
	totalIn = p1strpack(statusInfo, 5, ' ', totalIn);
	//exec_runtime
	totalIn = p1strpack(schedInfo, 5, ' ', totalIn);
	//rchar & wchar
	totalIn = p1strpack(ioInfo, 10, ' ', totalIn);
	//newline
	totalIn = p1strpack(newLine, 2, ' ', totalIn);

	free(ioInfo);
	free(statusInfo);
	free(schedInfo);

	return totalOut;

}


///////// SIGNAL FUNCTIONS /////////
static void signal_handler_sigusr1(int signal){
	/*
	* trap SIGUSR1 signal
	*/
	sigprocmask(SIG_BLOCK, &oldSet, NULL);

	if(signal == SIGUSR1){
		p1putstr(1, "Caught signal for SIGUSR1\n");
		SIGUSR1_check = 1;

	}

	sigprocmask(SIG_UNBLOCK,&oldSet, NULL);
}

static void signal_handler_alarm(UNUSED int signal){
	/*
	* Catches SIGALRM
	* schedules 
	*/

	sigprocmask(SIG_BLOCK, &oldSet, NULL);

	/// STEP 1: STOP PROCESS
	//first check if there is a current process
	if(currentProc){
		//check if currentProc is alive
		if(currentProc->isAlive){
			//take current process and send SIGSTOP
			kill(currentProc->pid, SIGSTOP);
			//printf("sending SIGSTOP signal to: %d %s\n",currentProc->pid, currentProc->work);
		//enqueue the stopped process to end of queue
			enqueue_queue(currentQueue, currentProc);
		}
	}

	/// STEP 2: CONTINUE/START next process on queue
	//dequeue next process to currentProcs
	currentProc = dequeue_queue(currentQueue);
	//dequeue_queue would return NULL if queue is null, so check for that
	if(currentProc){
		if(currentProc->isAlive){
			//process info
			char *processInfo = getProcessInfo(currentProc);
			//check to make sure current process already recieved the SIGUSRI signal
			if(currentProc->SIGUSR1_recieved){
				//if yes, then continue process
				kill(currentProc->pid, SIGCONT);
				//printf("sending SIGCONT signal to: %d %s\n", currentProc->pid, currentProc->work);
			}
			else{
				//if not, send SIGUSR1, set property to true
				currentProc->SIGUSR1_recieved = true;
				kill(currentProc->pid, SIGUSR1);
				//printf("sending SIGUSR1 signal to: %d %s\n",currentProc->pid, currentProc->work);
			}

			//print header
			p1putstr(1, processInfoLabel);
			p1putstr(1, separatorLabel);
			//print process info
			p1putstr(1, processInfo);
			//end with separator
			p1putstr(1, separatorLabel);
			free(processInfo);
		}
	}

	sigprocmask(SIG_UNBLOCK,&oldSet, NULL);
}


static void signal_handler_sigchld(UNUSED int signal){
	/*
	* When SIGCHLD is recieved from the child process
	* terminate child 
	* decrement number pf active processes
	*/

	sigprocmask(SIG_BLOCK, &oldSet, NULL);

	pid_t pid;
	int status;

	//wait for all dead process
	while((pid = waitpid(-1, &status, WNOHANG)) > 0){
		if(WIFEXITED(status) || WIFSIGNALED(status)){
			activeProcs--;
			if(pid == currentProc->pid){
				currentProc->isAlive = false;
			}
			//printf("waitpid successful: %d activeProcs: %d\n", pid, activeProcs);
		}
	}

	//free the current process
	free_process(&currentProc, numberProcs);

	sigprocmask(SIG_UNBLOCK,&oldSet, NULL);
}


static void subscribe_to_signals(){
	/*
	* call signal and check for error
	* prints error if there is one
	*/

	if(signal(SIGUSR1, signal_handler_sigusr1) == SIG_ERR){
		p1perror(1, "can't catch signal error for SIGUSR1\n");
		exit(1);
	}
	if(signal(SIGALRM, signal_handler_alarm) == SIG_ERR){
		p1perror(1, "can't catch signal error for SIGALRM\n");
		exit(1);
	}
	if(signal(SIGCHLD, signal_handler_sigchld) == SIG_ERR){
		p1perror(1, "can't catch signal error for SIGCHLD\n");
		exit(1);
	}

}


static void Execute(char **commands, int numCommands){

	pid_t pid = 0;
	//pid_t pids[160];
	//int status;
	char *commandWords[160];

	int i;
 
	struct timespec tm = {0, 20000000}; //20,000,000 ns == 20 ms
	//create empty queue
	currentQueue = create_queue();

	//loop through commands
	for(i = 0; i < numCommands; i++){
		//printf("About to fork %s\n", commands[i]);
		pid = fork();
		//pids[i]=pid;
		//printf("Command: %s || pid: %d\n",commands[i], pid);
	
		//Error case
		if (pid<0)
		{
			//error print & exit
			p1perror(1, "Fork");
			exit(1);
		}
		//parent mode
		else if (pid > 0)
		{
			//create process
			create_process(commands[i], pid, currentQueue);
			numberProcs++;
			activeProcs++;
			
			
		}
		//child mode 
		else if (pid == 0)
		{
			while(SIGUSR1_check != 1){
				(void)nanosleep(&tm, NULL);
			}

		 	split_Commands(commands[i], commandWords);
			//printf("child pid command: %s\n", *commandWords);
			int runCommand = execvp(*commandWords, commandWords);
			if(runCommand ==-1){
				//printf("ERROR child pid command: %s %s\n", *commandWords, commandWords[1]);
				p1perror(1, "Fork");
				exit(1);
			}

		}
	}

	struct itimerval it_val; // for setting itimer
	it_val.it_value.tv_sec = quantum/1000;
	it_val.it_value.tv_usec = (quantum*1000) % 1000000;
	it_val.it_interval = it_val.it_value;
	if(setitimer(ITIMER_REAL, &it_val, NULL) == -1){
		p1perror(1, "error calling setitimer()");
		exit(1);
	}


	while(activeProcs){
		(void)nanosleep(&tm, NULL);
	}

	for(i=0; i < numCommands; i++){
		//printf("freeing command: %s\n", commands[i]);
		free(commands[i]);
		commands[i] = NULL;
	}
	//print_queue(currentQueue);
	//printf("free queue\n");
	free(currentQueue);
}


int main(int argc, char *argv[]){

	//commands per line
	char *commands[160];
	int numCommands=0;
	char *envVariable;

	sigemptyset(&oldSet);

	sigaddset(&oldSet, SIGCHLD);
	sigaddset(&oldSet, SIGALRM);

	//printf("before subscribe_to_signals\n");
	subscribe_to_signals();

	//check for environment variable
	if((envVariable=getenv("USPS_QUANTUM_MSEC")) != NULL){
		//printf("envVariable: %s\n",  envVariable);
		quantumFlag = true;
		quantum = p1atoi(envVariable);
		Arguments(argc, argv, &quantum, commands, &numCommands);
	}
	else{
		Arguments(argc, argv, &quantum, commands, &numCommands);
	}
	
	//printf("quantum: %d\n", quantum);
	//check if quantum is still Null, if so, exit with error
	if(!quantumFlag){
		p1perror(1, "No environment variable or specified quantum");
		exit(1);
	}
	
	//printf("free queue\n");


	//printf("about to execute: %d\n", numCommands);
	Execute(commands, numCommands);
	//freeProccess(&numberOfProcess);


	//free commands
	int i;
	for(i=0; i < numCommands; i++){
		//printf("freeing command: %s\n", commands[i]);
		free(commands[i]);
		commands[i] = NULL;
	}
	return 0;
}