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
struct node {
    char *line;
    struct node *next;
	struct node *prev;
};

//I don't think we actually need this, but I don't know how else to do the pop function.
typedef struct {
	int q_len;
	char* buffer;
	sem_t empty;
	sem_t full;
	pthread_mutex_t lock;
	struct node* head;
	struct node* tail;
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
	struct node *currLine;

	assert(sem_wait(&queue->full) == 0);
	assert(pthread_mutex_lock(&queue->lock) == 0);

	currLine = queue->head->next;
	queue->head->next = currLine->next;
	currLine->next->prev = queue->head;
	currLine->next = currLine->prev = NULL;
	
	assert(pthread_mutex_unlock(&queue->lock) == 0);	
	assert(sem_post(&queue->empty) == 0);

	return currLine->line;

}

void append(Queue* queue, char* inLine){
	printf("Starting Append");
	struct node *newLine = malloc(sizeof(struct node));
	newLine->line[1024];
	printf("%lu", strlen(inLine));
	//strncpy(inLine, newLine->line, 1024);
	assert(sem_wait(&queue->empty) == 0);
	assert(pthread_mutex_lock(&queue->lock) == 0);
	newLine->prev = queue->tail->prev;
	newLine->next = queue->tail;
	queue->tail->prev->next = newLine;
	queue->tail->prev = newLine;
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

void list_init(Queue* quInit) {
	quInit->q_len = 5;
	quInit->buffer[1024];
	sem_init(&quInit->empty, 0, quInit->q_len);
	sem_init(&quInit->full, 0, 0);
	pthread_mutex_init(&quInit->lock, NULL);
	quInit->head = quInit->tail = NULL;
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

	list_init(&lines);

	assert(sem_init(&update, 0, 1) == 0);

	textFile = fopen("./midsummer.txt", "r");
	struct producerData prodData = {&lines, textFile};
	assert(sem_init(&update, 0, 1) == 0);
	assert(pthread_create(&producerThread, NULL, (void*)producerFn, (void*)&prodData) == 0);

	pthread_join(producerThread, NULL);
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
