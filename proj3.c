#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

//Global variable for word count and counter, just to avoid carrying around an extra variables.
int totalWordCount = 0;
int i;
#define MAXSIZE 1024


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
	char **buffer;
	sem_t empty;
	sem_t full;
	pthread_mutex_t lock;
} Queue;

/*
//this was redefined below to make life easier
struct threadData {
	char* currLine;
	sem_t updating;
};

struct producerData {
	Queue* quOut;
	FILE* textFile;
};
*/

typedef struct{
	int task;
	Queue* q;
}threadData;

//Based off of producer-consumer notes
char* pop(Queue *q){

	assert(sem_wait(&q->full) == 0);
	assert(pthread_mutex_lock(&q->lock) == 0);

	char *line = q->buffer[q->use];
	//strncpy(line, queue->buffer[queue->use], 1024);
	//queue->buffer[queue->use] = NULL;
	q->use = (q->use + 1) % (q->q_len + 1);
	int lineNum = q->use;

	
	printf("Line %d\n", lineNum);

	
	
	assert(pthread_mutex_unlock(&q->lock) == 0);
	assert(sem_post(&q->empty) == 0);
	return line;
}

void push(Queue *q, char *line){
	assert(sem_wait(&q->empty) == 0);
//THE ASSERTION BELOW IS FAILING
	assert(pthread_mutex_lock(&q->lock) == 0);
	
	//strncpy(queue->buffer[queue->fill], inLine, 1024);
	q->buffer[q->fill] = (char*) line;
	q->fill = (q->fill +1) % (q->q_len + 1);

	assert(pthread_mutex_lock(&q->lock) == 0);
	assert(sem_post(&q->full) == 0);
}


//Prints out the number of words in a line, might need to change this later to not include white spaces, haven't decided yet
//Honestly, this probably will definitely need to be changed. probably maybe idk
/*
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
*/

//this is the thread function, which basically is just the producerFn and wordCount functions above this updated and combined.
void *wordCount(void *attr){
	printf("Starting Thread Function");
	threadData *info = (threadData *) attr;
	int taskNum = info->task;
	int size = info->q->q_len;

	while(i < size - 1){
		i++;
		char *text = pop(info->q);
		printf("Line: %s\n", text);
	
		int wordsInLine = 0;	
		
		for (int x = 0; text[x] != '\0'; x++){
			//figured we'd have to do this at some point...
			if (text[x] == ' ' || text[x + 1] == '\0'){
				wordsInLine++;
			}
		}
		printf("Task num %d, %d words", taskNum, wordsInLine);
		totalWordCount = totalWordCount + wordsInLine;
	}
	
	printf("Ending Thread Function");
	//need	
	pthread_exit(NULL);
}
/* //need?
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
*/

int main(int argc, char *argv[]){
	
	//we need to make sure the user enters the current number of variables
	//num of task, text file
	if (argc != 2){
		printf("Incorrect number of parameters\n");
	}

	//variables

	//int maxLength = 1024; //this was defined above as MAXSIZE
	
	int numLines = 0;
	int numWords = 0;
	//char* line[maxLength];
	char * line = calloc(1,1), buff[MAXSIZE];
	

	//reads a line and stores it
	while (fgets(buff, MAXSIZE, stdin)){
		line = realloc(line, strlen(line) + 1 + strlen(buff));
		numLines++;
		if (line == 0){
			printf("Does not exist");
			exit(1);
		}

		strcat(line, buff);
	}
	
	strcat(line, "\0");


	/* //just changed all of this, sorry
	

	FILE* textFile;
	Queue lines;

	list_init(&lines, tasks);

	assert(sem_init(&update, 0, 1) == 0);
	*/
	
	char * anotherBuff[numLines];
	Queue lines;

	assert(sem_init(&lines.empty, 0, numLines) == 0);
	assert(sem_init(&lines.full, 0, 0) == 0);
	/*
	textFile = fopen("./midsummer.txt", "r");
	struct producerData prodData = {&lines, textFile};
	assert(sem_init(&update, 0, 1) == 0);
	assert(pthread_create(&producerThread, NULL, (void*)producerFn, (void*)&prodData) == 0);
	printf("Producer Created\n");
	assert(pthread_join(&producerThread, NULL) == 0);
	//TEST LINE
	printf("%d tasks total\n", tasks);
	//Scans until end of file (ideally) fgets runs until it encounters EOF so spaces should be fine
*/

	char *linesNext = malloc(MAXSIZE);
	linesNext = strtok(line, "\n");
	linesNext = strcat(linesNext, "\0");
	
	push(&lines, linesNext);

	
	for(int i = 0; i < numLines - 2; i++) {
		//assert(pthread_join(listOfThreads[i], NULL) == 0);
		linesNext = strtok(NULL, "\n");
		linesNext = strcat(linesNext, "\0");
		push(&lines, linesNext);
	}

	int tasks = atoi(argv[1]);	
	pthread_t producerThread[tasks];
	//pthread_t listOfThreads[sizeof(pthread_t) * tasks];
	
	//kind of similar to the commented out line
	threadData listOfThreads[tasks];
	
	int nums[tasks];
	for (int i = 0; i <tasks; i++){
		
		nums[i] = i;	
	}

	for (int i = 0; i < tasks; i++){
			
		listOfThreads[i].task = nums[i];
		listOfThreads[i].q = &lines;			
	}
	

	for (int i = 0; i < tasks; i++){
		assert(pthread_create(&producerThread[i], NULL, &wordCount, (void *) &listOfThreads[i]) == 0);

	}

	for (int i = 0; i < tasks; i++){
		pthread_join(producerThread[i], NULL);
	}
	
	printf("Number of words: %d\n", totalWordCount);

	free(line);
	return 0;
}

