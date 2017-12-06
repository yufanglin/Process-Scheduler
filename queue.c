/*
* methods for the queue
* Referenced: https://gist.github.com/ArnonEilat/4471278 
*		&   https://www.tutorialspoint.com/data_structures_algorithms/dsa_queue.htm  : for Queue help
*/

#include <stdio.h>
#include <stdlib.h>

#include "p1fxns.h"
#include "queue.h"
#include <stdbool.h>
#define UNUSED __attribute__ ((unused))

// struct process{
// 	char *work;
// 	pid_t pid;
// 	int status;
// 	bool isAlive;
// 	bool SIGUSR1_recieved;
	
// };

// struct task{
// 	Process *proc;
// 	struct task *next;
// };

// struct queue{
// 	Task *front;
// 	Task *rear;
// 	int size;
// };



Process *create_process(char *work, pid_t pid, Queue *queue){
	/*
	* initiates process structure with the commandline and pid
	* 
	*/

	//allocate memory
	Process *newProcess = (Process *)malloc(sizeof(Process));

	//check if memory allocated
	if(newProcess){

		newProcess->work = work;
		newProcess->pid = pid;
		newProcess->status = 0;
		newProcess->isAlive = true;
		newProcess->isSuspended = false;
		newProcess->SIGUSR1_recieved = false;
		//enqueue, check to make sure it is successful. Use global queue = currentQueue
		int enqueueCheck = enqueue_queue(queue, newProcess);
		if(enqueueCheck == 0){
			p1perror(1, "Enqueue unsuccessful in creating Process\n");
			exit(1);
		}
		//increment number of process (global variable)
	}
	else{
		//malloc unsuccessful, exit
		p1perror(1, "Malloc unsuccessful\n");
		exit(1);

	}

	return newProcess;
}

void free_process(Process **process, UNUSED int numberProcs){
	//int i;
	if((*process) && !((*process)->isAlive)){
		free(*process);
		*process = NULL;
	}
	// for(i=0; i < numberProcs; i++){
	// 	free(process->work);
	// }
}

Queue *create_queue(void){
	/*
	* Queue constructor that takes in the size of commands
	* return queue (will return null of memory not allocated)
	*/

	//allocate memory
	Queue *queue = (Queue *)malloc(sizeof(Queue));

	//check for if memory was allocated
	if(queue){
		queue->front = NULL;
		queue->rear = NULL;
		queue->size = 0;
		return queue;
	}
	else{
		//return NULL if memory not allocated to queue
		return NULL;
	}
	
}
bool is_queue_empty(Queue *queue){
	/*
	* check to see if queue is empty,
	* if yes -> return True || if no -> return False
	*/

	return (queue->size == 0);
}

int enqueue_queue(Queue *queue, Process *process){
	/*
	* Put task into queue
	* set task to end(rear) of queue (FIFO)
	* increment size of queue
	* return 0 if didn't enqueue, 1 if engueue worked
	*/

	//make sure queue or process isn't NULL
	if((queue == NULL) || (process == NULL)){
		p1perror(1,"Enqueue: queue or task is NULL\n");
		return 0;
	}

	//then allocate memory for the Task
	Task *task = (Task *)malloc(sizeof(Task));

	//make sure space was allocated
	if(task == NULL){
		return 0;
	}

	//if queue is empty
	if(is_queue_empty(queue)){
		queue->front = task;
		queue->rear = task;
	}
	//if queue already has some tasks
	else{
		//add task to current rear's next
		queue->rear->next = task;
		//add task to end(rear) of queue (FIFO)
		queue->rear = task;
	}

	//since task is the new rear, it's next is NULL
	task->next = NULL;
	task->proc = process;
	//increment queue's size
	queue->size++;

	//successful engueue
	return 1;


}

Process *dequeue_queue(Queue *queue){
	/*
	* Dequeue task 
	* take the queue->front and replace it with the next task
	* then decrease the size of queue
	* return the task that was removed from the queue
	*/

	//task variable for the current head
	Task *task;
	Process *process;

	//make sure queue isn't already empty
	if(!is_queue_empty(queue)){
		//initiate task with queue's head
		task = queue->front;
		//initiate process with task's process adt
		process = task->proc;
		//set queue's front with next
		queue->front = queue->front->next;
		//free task
		//printf("free task\n");
		free((void *)task);
		//decrement size of queue
		queue->size--;
		//if new queue size is empty set rear to NULL
		if(is_queue_empty(queue)){
			queue->rear = NULL;
		}
		return process;
	}
	else{
		//return NULL if queue is already empty
		return NULL;
	}
}


void free_queue(Queue *queue, int numberProcs){
	/*
	* free the memory allocated to the queue with free()
	*/

	if(!queue){
		p1putstr(1,"queue is nul\n");
	}
	//variable to hold dequeued element
	Process *process;

	//loop through queue and dequeue until empty
	while(!is_queue_empty(queue)){
		process = dequeue_queue(queue);
		//p1putstr(1,"queue is nul\n");
		free_process(&process, numberProcs);
	}

	//printf("Free queue in free_queue\n");
	free(queue);
}