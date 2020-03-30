#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>

//Christian, you can change any variable names if you don't like them or feel like they don't make sense.

//I wanted to name this struct 'entry' but I figured we should go about this like a linked list of sorts...if that makes sense
struct node {
    char *line;
    struct node *next;
};

//I don't think we actually need this, but I don't know how else to do the pop function.
typedef struct {
	sem_t empty;
	sem_t full;
	pthread_mutex_t lock;
	
}Queue;

struct node *head = NULL;
struct node *tail = NULL;
sem_t empty;
sem_full;
pthread_mutex_t lock;


//this should be tested, I might've forgotten a step or miscalculated something...
char* pop(Queue* queue){
	struct node *currLine;

	assert(sem_wait(&queue->full) == 0);
	assert(pthread_mutex_lock(&queue->lock) == 0);

	currLine = head;
	head = head->next;
	
	assert(pthread_mutex_unlock(&queue->lock) == 0);	
	assert(sem_post(&queue->empty) == 0);

	return currLine->line;

}

void append(char* line){
	struct node *newLine = malloc(sizeof(struct node));
	newLine->line = line;
	newLine->next = NULL;
	if (tail == NULL){
		head = newLine;
		tail = newLine;
	}
	else{
		tail->next = newLine;
		tail = tail->next;	
	}
	assert(pthread_mutex_unlock(&lock) == 0);
}


//Prints out the number of words in a line, might need to change this later to not include white spaces, haven't decided yet
//Honestly, this probably will definitely need to be changed. probably maybe idk

int wordCount(char* currLine){
	int numOfWords = 0;
	int i = 0;

	for (i = 0; currLine[i]; i++){
		if (currLine[i] == 32){
			numOfWords++;		
		}
	}

	if (i > 0){
		numOfWords++;
	}

	return numOfWords;
}



int main(int argc, char *argv[]){
	
	//number of tasks to run
	int tasks = atoi(argv[1]);

	//TEST LINE
	printf("%d tasks total", tasks);

	//number of lines in file
	int numLines = 0;

	//pointer to current line read
	char *linePointer = NULL;

	//size of line
	size_t linePointerSize = 0;


	//TO-DO: everything here, was working on it, but got tired ¯\_(ツ)_/¯
}
