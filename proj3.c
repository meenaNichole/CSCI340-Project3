#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

//Global variable for word count, just to avoid carrying around an extra variable.
int totalWordCount = 0;
//Global semaphore for updating the variable
sem_t update;
//Christian, you can change any variable names if you don't like them or feel like they don't make sense.

//I wanted to name this struct 'entry' but I figured we should go about this like a linked list of sorts...if that makes sense
/*
struct node {
    char line[1024];
    struct node *next;
	struct node *prev;
};
*/
//I don't think we actually need this, but I don't know how else to do the pop function.
typedef struct {
	int fill;
	int use;
	int q_len;
	char** buffer;
	sem_t empty;
	sem_t full;
	pthread_mutex_t lock;
}Queue;

struct threadData {
	char* currLine;
	sem_t updating;
};

struct producerData {
	Queue* quOut;
	FILE* textFile;
};

/*
struct node *head = NULL;
struct node *tail = NULL;
sem_t empty;
sem_t sem_full;
pthread_mutex_t lock;
*/

//this should be tested, I might've forgotten a step or miscalculated something...
char* pop(Queue* queue){

	char line[1024];
	assert(sem_wait(&queue->full) == 0);
	assert(pthread_mutex_lock(&queue->lock) == 0);

	strncpy(line, queue->buffer[queue->use], 1024);
	queue->buffer[queue->use] = NULL;
	queue->use = (queue->use + 1) % queue->q_len;
	assert(pthread_mutex_unlock(&queue->lock) == 0);	
	assert(sem_post(&queue->empty) == 0);

	return line;

}

void append(Queue* queue, char* inLine){
	
	assert(sem_wait(&queue->empty) == 0);
	assert(pthread_mutex_lock(&queue->lock) == 0);
	strncpy(queue->buffer[queue->fill], inLine, 1024);
	queue->fill = (queue->fill + 1) % queue->q_len;
	assert(pthread_mutex_unlock(&queue->lock) == 0);
	assert(sem_post(&queue->full) == 0);
	printf("Ending Append");
}

void producerFn(void* args) {
	printf("Starting Producer Fn");
	struct producerData* prodData = (struct producerData*) args;
	Queue* lines = prodData->quOut;
	FILE* textFile = prodData->textFile;
	char line[1024];
	while (fgets(line, 1024, textFile)) {
		append(lines, line);
		printf("%s", line);
	}
	printf("Ending Producer Fn");
}

//Prints out the number of words in a line, might need to change this later to not include white spaces, haven't decided yet
//Honestly, this probably will definitely need to be changed. probably maybe idk

int wordCount(char* line){
	int numOfWords = 0;
	int i = 0;
	char* currLine = line;
	for (i = 0; currLine[i]; i++){
		if (currLine[i] == 32){
			numOfWords++;		
		}
	}

	sem_wait(&update);
	totalWordCount += numOfWords;
	sem_post(&update);
	
	return 0;
}

void list_init(Queue* quInit, int numTasks) {
	quInit->q_len = numTasks;
	quInit->fill = 0;
	quInit->use = 0;
	char* buffer1[numTasks];
	quInit->buffer = buffer1;
	for(int i = 0; i < numTasks; i++) {
		char* arrayToInit = quInit->buffer[i];
		arrayToInit[1024];
	}
	sem_init(&quInit->empty, 0, quInit->q_len);
	sem_init(&quInit->full, 0, 0);
	pthread_mutex_init(&quInit->lock, NULL);
}


int main(int argc, char *argv[]){
	
	
	//number of tasks to run
	int maxLength = 1024;
	int tasks = atoi(argv[1]);
	int numLines = 0;
	int numWords = 0;
	char* line[maxLength];

	pthread_t producerThread;
	pthread_t listOfThreads[sizeof(pthread_t) * tasks];

	FILE* textFile;
	Queue lines;

	list_init(&lines, tasks);

	assert(sem_init(&update, 0, 1) == 0);

	textFile = fopen("./midsummer.txt", "r");
	struct producerData prodData = {&lines, textFile};
	assert(sem_init(&update, 0, 1) == 0);
	assert(pthread_create(&producerThread, NULL, (void*)producerFn, (void*)&prodData) == 0);
	printf("Producer Created\n");
	assert(pthread_join(&producerThread, NULL) == 0);
	//TEST LINE
	printf("%d tasks total\n", tasks);
	//Scans until end of file (ideally) fgets runs until it encounters EOF so spaces should be fine
/*
	for(int i = 0; i < sizeof(listOfThreads) / sizeof(pthread_t*); i++) {
		assert(pthread_join(listOfThreads[i], NULL) == 0);
	}
*/
	printf("Number of words: %d\n", totalWordCount);

}
