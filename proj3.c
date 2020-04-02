#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAXSIZE 1024              



int wordsNum;
int count;


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
//this was redefined below to make life easier
/*
struct producerData {
Queue* quOut;
FILE* textFile;
};

*/
//queue structure
typedef struct {
	int             use;        
	int             fill;      
	int             q_len;       
	char            **buffer1; 
	pthread_mutex_t lock;
	sem_t           full;
	sem_t           empty;
} Queue;

//thread information
typedef struct {
	int task;
	Queue* q;
} threadData;

//push to queue
void push(Queue *queues, char *line){
	//printf("Start push");
	assert(sem_wait(&queues->empty) == 0);
	assert(pthread_mutex_lock(&queues->lock) == 0);

	queues->buffer1[queues->fill] = (char*) line;
	queues->fill = (queues->fill + 1) % (queues->q_len + 1);

	assert(pthread_mutex_unlock(&queues->lock) == 0);
	assert(sem_post(&queues->full) == 0);
	//printf("End push");
}

//pop from queue
char* pop(Queue *queues) {
	//printf("Start pop");
	assert(sem_wait(&queues->full) == 0);
	assert(pthread_mutex_lock(&queues->lock) == 0);

	char *t = queues->buffer1[queues->use];
	queues->use = (queues->use + 1) % (queues->q_len + 1);
	int currentLine = queues->use;
	//printf("Current line#%d\n", currentLine);

	assert(pthread_mutex_unlock(&queues->lock) == 0);
	assert(sem_post(&queues->empty) == 0);
	//printf("End pop");
	return t;
}




//Prints out the number of words in a line, might need to change this later to not include white spaces, haven't decided yet
//Honestly, this probably will definitely need to be changed. probably maybe idk (I WAS RIGHT :D)
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

void *countWords(void *args) {


	threadData *info = (threadData *) args;
	int taskNum = info->task;
	int size = info->q->q_len;


	while(count < size - 1){
		count++;
		char *item = pop(info->q);
		printf("Current line: %s\n", item);


		int line = 0;



		for(int i = 0; item[i] != '\0'; i++){


			if(item[i] == ' ' || item[i + 1] == '\0')
			{
				line++;
			}
		}
		wordsNum = wordsNum + line;
		printf("Task %d: %d words\n", taskNum, line);
		printf("--------------------------------------------------------------------------------\n");
	}



}


int main(int argc, char **arg) {

	//we need to make sure the user enters the current number of variables
	//num of task, text file
	if (argc != 2) {
		printf("--------------------------------------------------------------------------------\n");
		printf("Uh oh! Missing parameter\n");
		printf("--------------------------------------------------------------------------------\n");	
	}


	/* //just changed all of this, sorry


	FILE* textFile;
	Queue lines;

	list_init(&lines, tasks);

	assert(sem_init(&update, 0, 1) == 0);


	char * anotherBuff[numLines];
	Queue lines;

	assert(sem_init(&lines.empty, 0, numLines) == 0);
	assert(sem_init(&lines.full, 0, 0) == 0);
	*/

	wordsNum = 0;
	int lineCount = 0;
	char *line = calloc(1,1), buffer1[MAXSIZE];

	while (fgets(buffer1, MAXSIZE, stdin)) {
		line = realloc( line, strlen(line)+1+strlen(buffer1));


		lineCount++;

		//if the line doesn't exist basically, then something is wrong and quit
		if (line == 0) {
			printf("--------------------------------------------------------------------------------\n");
			printf("Uh oh! Something went wrong...figure it out.\n");
			printf("--------------------------------------------------------------------------------\n");


			exit(1);
		}
		strcat(line, buffer1);


	}



	strcat(line, "\0");


	
	char *Buffer1[lineCount];
    Queue q = { 0, 0, lineCount, Buffer1, PTHREAD_MUTEX_INITIALIZER};
    assert(sem_init(&q.empty, 0, lineCount) == 0);
    assert(sem_init(&q.full, 0, 0) == 0);




	
	char *nextLine = malloc(MAXSIZE);
	nextLine = strtok(line, "\n");
	nextLine = strcat(nextLine, "\0");
	push(&q, nextLine);


	for (int i = 0; i < lineCount - 2; i++) {
		nextLine = strtok(NULL, "\n");
		nextLine = strcat(nextLine, "\0");
		push(&q, nextLine);
	}


	int nextTask = atoi(arg[1]);
	pthread_t threadID[nextTask];
	threadData threadX[nextTask];


	int nums[nextTask];

	//i don't really like all of these for loops but it won't flipping work when put together.
	//Christian if you want to look at this, please do
	for (int i = 0; i < nextTask; i++) {
		nums[i] = i;
	}


	for (int i = 0; i < nextTask; i++) {
		threadX[i].task = nums[i];
		threadX[i].q = &q;
	}


	for (int i = 0; i < nextTask; i++){
		assert(pthread_create(&threadID[i], NULL, &countWords, (void *) &threadX[i]) == 0);
	}


	for (int i = 0; i < nextTask; i++){
		pthread_join(threadID[i], NULL);
	}

	printf("--------------------------------------------------------------------------------\n");
	printf("Final word count is %d\n", wordsNum);
	printf("--------------------------------------------------------------------------------\n");

	free(line);


	return 0;
}


