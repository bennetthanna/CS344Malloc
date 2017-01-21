#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define SIZE 2048

typedef struct free_list_node {
    unsigned int size;
    struct free_list_node *next;
} free_list_node;

void *my_malloc(int size);
void my_free(void *ptr);
void print_free_list();
