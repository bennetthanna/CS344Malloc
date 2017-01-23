all: my_malloc.c my_malloc.h
	gcc -Wall -o hw2 my_malloc.c

clean:
	$(RM) hw2

