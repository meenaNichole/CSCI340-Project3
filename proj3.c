#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>


int totalWordCount = 0;
//Christian, you can change any variable names if you don't like them or feel like they don't make sense.

//I wanted to name this struct 'entry' but I figured we should go about this like a linked list of sorts...if that makes sense
struct node {
    char *line;
    struct node *next;
	struct node *prev;
};

//I don't think we actually need this, but I don't know how else to do the pop function.
typedef struct {
	sem_t *empty;
	sem_t *full;
	pthread_mutex_t *lock;
	struct node* head;
	struct node* tail;
}Queue;

struct threadData {
	char* currLine;
	sem_t* updating;
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
	struct node *currLine;

	assert(sem_wait(queue->full) == 0);
	assert(pthread_mutex_lock(queue->lock) == 0);

	currLine = queue->head->next;
	queue->head->next = currLine->next;
	currLine->next->prev = queue->head;
	currLine->next = currLine->prev = NULL;
	
	assert(pthread_mutex_unlock(queue->lock) == 0);	
	assert(sem_post(queue->empty) == 0);

	return currLine->line;

}

void append(Queue* queue, char* line){
	struct node *newLine = malloc(sizeof(struct node));
	strcpy(line, newLine->line);
	assert(sem_post(queue->full) == 0);
	assert(pthread_mutex_lock(queue->lock) == 0);
	newLine->prev = queue->tail->prev;
	newLine->next = queue->tail;
	queue->tail->prev->next = newLine;
	queue->tail->prev = newLine;
	assert(pthread_mutex_unlock(queue->lock) == 0);
	assert(sem_wait(queue->empty) == 0);
}


//Prints out the number of words in a line, might need to change this later to not include white spaces, haven't decided yet
//Honestly, this probably will definitely need to be changed. probably maybe idk

int wordCount(void *threaddata){
	struct threadData* wordCount = (struct threadData *) threaddata;
	int numOfWords = 0;
	int i = 0;
	char* currLine = wordCount->currLine;
	sem_t* update = wordCount->updating;
	for (i = 0; currLine[i]; i++){
		if (currLine[i] == 32){
			numOfWords++;		
		}
	}

	if (i > 0){
		numOfWords++;
	}

	sem_wait(update);
	totalWordCount += numOfWords;
	sem_post(update);
	
	return 0;
}


int main(int argc, char *argv[]){
	
	//number of tasks to run
	int maxLength = 128;
	int tasks = atoi(argv[1]);
	pthread_t listOfThreads[sizeof(pthread_t) * tasks];
	int numLines = 0;
	int numWords = 0;
	FILE* textFile;
	textFile = fopen("midsummer.txt", "r");
	char* line[maxLength];
	sem_t update;
	assert(sem_init(&update, 0, 1) == 0);

	//TEST LINE
	printf("%d tasks total", tasks);
	//Scans until end of file (ideally) fgets runs until it encounters EOF so spaces should be fine
	while (fgets(*line, 128, textFile) != NULL) {
		struct threadData* threaddata = malloc(sizeof(struct threadData));
		threaddata->currLine = *line;
		threaddata->updating = update;
		for(int i = 0; i < tasks; i++) {
		
			assert(pthread_create(&listOfThreads[i], NULL, (void*)wordCount, (void*)threaddata) == 0);
			
		}
	}

	printf("Number of words: %d", totalWordCount);

}
