/*
* methods for the queue
* Referenced: https://gist.github.com/ArnonEilat/4471278 
*		&   https://www.tutorialspoint.com/data_structures_algorithms/dsa_queue.htm  : for Queue help
*/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#define UNUSED __attribute__ ((unused))
#include <stdbool.h>

typedef struct process{
	char *work;
	pid_t pid;
	int status;
	bool isAlive;
	bool isSuspended;
	bool SIGUSR1_recieved;
}Process;

typedef struct task {
	Process *proc;
	struct task *next;
}Task;

typedef struct queue {
	Task *front;
	Task *rear;
	int size;
}Queue;


/*
	* initiates process structure with the commandline and pid
	* 
*/
Process *create_process(char *work, pid_t pid, Queue *queue);

/*
	*  free process memory
	*
*/

void free_process(Process **process, UNUSED int numberProcs);
/*
	* Queue constructor that takes in the size of commands
	* return queue (will return null of memory not allocated)
*/
Queue *create_queue(void);

/*
	* Put task into queue
	* set task to end(rear) of queue (FIFO)
	* increment size of queue
	* return 0 if didn't enqueue, 1 if engueue worked
*/
int enqueue_queue(Queue *queue, Process *process);

/*
	* Dequeue task 
	* take the queue->front and replace it with the next task
	* then decrease the size of queue
	* return the task that was removed from the queue
*/

Process *dequeue_queue(Queue *queue);

/*
	* check to see if queue is empty,
	* if yes -> return True || if no -> return False
	*/


bool is_queue_empty(Queue *queue);

/*
	* free the memory allocated to the queue with free()
*/


void free_queue(Queue *queue, int numberProcs);



#endif	