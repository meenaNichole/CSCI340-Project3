all: proj3

clean: 
	rm proj3

proj3: proj3.c
	gcc -pthread -o proj3 proj3.c
