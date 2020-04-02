#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <semaphore.h>

#define BUFFERSIZE 1024              //the max size of a line

int totalWords;
int counter;

typedef struct {
	int             fill;        //next point to fill in the queue
	int             use;         //next point to get in the queue
	int             q_len;       //The MAX size of the queue
	char            **bufferLine; //buffer for adding and removing line values
	pthread_mutex_t queue_lock;
	sem_t           empty;
	sem_t           full;
} QUEUE;

//for possible argument parameters into threads
typedef struct {
	int taskNumber;
	QUEUE* que;
} ontoThread;

void put(QUEUE *wholeText, char *line){

	//signal semaphore and lock
	assert(sem_wait(&wholeText->empty) == 0);
	assert(pthread_mutex_lock(&wholeText->queue_lock) == 0);

	wholeText->bufferLine[wholeText->fill] = (char*) line;
	wholeText->fill = (wholeText->fill + 1) % (wholeText->q_len + 1);

	// signal semaphore and unlock
	assert(pthread_mutex_unlock(&wholeText->queue_lock) == 0);
	assert(sem_post(&wholeText->full) == 0);
}

char* get(QUEUE *wholeText) {
	// signal semaphore and lock
	assert(sem_wait(&wholeText->full) == 0);
	assert(pthread_mutex_lock(&wholeText->queue_lock) == 0);

	char *temp = wholeText->bufferLine[wholeText->use];
	wholeText->use = (wholeText->use + 1) % (wholeText->q_len + 1);
	int currLineNum = wholeText->use;
	printf("Current line number is %d\n", currLineNum);

	// signal semaphore and lock
	assert(pthread_mutex_unlock(&wholeText->queue_lock) == 0);
	assert(sem_post(&wholeText->empty) == 0);
	return temp;
}


//the THREAD FUNCTION

void *wordCount(void *taskInfo) {

	ontoThread *argsPassed = (ontoThread *) taskInfo;
	int Tn = argsPassed->taskNumber;
	int size = argsPassed->que->q_len;

	while(counter < size - 1){
		counter++;
		char *big = get(argsPassed->que);
		printf("Current line content : %s\n", big);

		int lineWords = 0;

		// word detection, either a trailing space or a trailing null byte
		for(int i = 0; big[i] != '\0'; i++){

			if(big[i] == ' ' || big[i + 1] == '\0')
			{
				lineWords++;
			}
		}
		totalWords = totalWords + lineWords;
		printf("Line contains %d words. Calculated by task number %d.\n\n", lineWords, Tn);
	}

	pthread_exit(NULL);
}

int main(int argc, char **arg) {

	if (argc != 2) {
		printf("Please enter int:<tasksToRun>\n");
	}

	//set up total words and get linecount for the correct size of the queue
	totalWords = 0;
	int lineCount = 0;
	char *text = calloc(1,1), buffer[BUFFERSIZE];
	//now to get stuff from stdin black magic dont question
	while (fgets(buffer, BUFFERSIZE, stdin)) {
		text = realloc( text, strlen(text)+1+strlen(buffer));

		lineCount++;
		if (!text) {
			printf("Error text pointer is null");

			exit(1);
		}
		strcat(text, buffer);

	}

	//making sure text string has trailing null
	strcat(text, "\0");

	//creating the queue
	char *cBuffer[lineCount];
        QUEUE q = { 0, 0, lineCount, cBuffer, PTHREAD_MUTEX_INITIALIZER};
        assert(sem_init(&q.empty, 0, lineCount) == 0);
        assert(sem_init(&q.full, 0, 0) == 0);


	//now turning the text string into lines by delimiting with \n then adding on a null
	char *lineOnQueue = malloc(BUFFERSIZE);
	lineOnQueue = strtok(text, "\n");
	lineOnQueue = strcat(lineOnQueue, "\0");
	put(&q, lineOnQueue);

	for (int i = 0; i < lineCount - 2; i++) {
		lineOnQueue = strtok(NULL, "\n");
		lineOnQueue = strcat(lineOnQueue, "\0");
		put(&q, lineOnQueue);
	}

	// initializing task numbers and amount of tasks
	int tasksToRun = atoi(arg[1]);
	pthread_t threadID[tasksToRun];
	ontoThread onto[tasksToRun];

	//for task number so each task has unique value in a unique location
	int numSeq[tasksToRun];
	for (int i = 0; i < tasksToRun; i++) {
		numSeq[i] = i;
	}

	for (int i = 0; i < tasksToRun; i++) {
		//putting task number and queue into thread argument structure
		onto[i].taskNumber = numSeq[i];
		onto[i].que = &q;
	}

	for (int i = 0; i < tasksToRun; i++){
		assert(pthread_create(&threadID[i], NULL, &wordCount, (void *) &onto[i]) == 0);
	}

	for (int i = 0; i < tasksToRun; i++){
		pthread_join(threadID[i], NULL);
	}

	printf("\n\nTotal word count: %d\n\n", totalWords);

	free(text);

	return 0;
}
