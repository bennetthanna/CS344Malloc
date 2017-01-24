#include "my_malloc.h"

void *ptr;
free_list_node *free_list;
free_list_node *current_node;
free_list_node *tempNext;
free_list_node *previous_node;
free_list_node *free_list_head;
free_list_node *free_list_end;
free_list_node *allocated_node;
free_list_node *freeing_node;
int tempSize;
int nodeNum;

void *my_malloc(int size) {
    
    //error check input size
    if (size < 0) {
        fprintf(stderr, "my_malloc: Size must be greater than 0\n");
    } else if (size > SIZE) {
        fprintf(stderr, "my_malloc: Size must be less than 2048\n");
    } else {
        fprintf(stderr, "my_malloc: Called with size = %i\n", size);
        
        //if free list is 0 empty
        if (free_list == NULL) {
            fprintf(stderr, "my_malloc: Allocating new free list\n");

            //get chunk of heap
            ptr = sbrk(SIZE);
            
            //error check and return NULL on error
            if (ptr == (void*) -1) {
                return NULL;
            }
            
            //set beginning of free list to beginning of heap
            free_list = (free_list_node*) ptr;
            
            //move free list to point to actual beginning of memory block ignoring node with size and next
            free_list = free_list + sizeof(free_list_node);
            
            //set free list size to the size you called sbrk with minus the size of the node that holds size and next
            free_list->size = SIZE - sizeof(free_list_node);
            
            //set free list next to NULL
            free_list->next = NULL;
        }

        //set current node to the free list
        current_node = free_list;
        //set previous node to NULL
        previous_node = NULL;
        
        //while current node is not NULL
        while(current_node != NULL) {
            
            //if the available memory is the same as size requested
            if(current_node->size == size) {
                fprintf(stderr, "my_malloc: Scanning free list... Found exact match\n");

                //if there is no previous node then the current node is the head of the free list
                if(previous_node == NULL) {
                    
                    //set the allocated node to where your current address is
                    allocated_node = current_node;
                    allocated_node->size = size;
                    allocated_node->next = current_node->next;
                    
                    //set the current node to the next node
                    current_node = current_node->next;
                    
                    //set head of free list to the new current node as that will be the new head
                    free_list = current_node;
                    
                    return allocated_node;
                    
                } else {

                    //set allocated node to where your current address is
                    allocated_node = current_node;
                    allocated_node->size = size;
                    allocated_node->next = current_node->next;
                    
                    //else set the previous node's next to the current node's next
                    previous_node->next = current_node->next;
                    
                    return allocated_node;
                }
            }
            
            //if the available memory is greater than the size requested
            else if ((current_node->size) > size) {
                fprintf(stderr, "my_malloc: Scanning free list... Sufficient space on free list\n");
                
                //set current node's next to temp to hold onto
                tempNext = current_node->next;
                //set current node's size to tempSize to hold onto
                tempSize = current_node->size;
                
                //set allocated node to where your current address is
                allocated_node = current_node;
                allocated_node->size = size;

                //move pointer to new head of free list by adding size requested plus size of node
                //you must add size of node as well to ignore the part that is just size and next pointer
                //this puts the current node address to the beginning of usable memory
                current_node += (size + sizeof(free_list_node));
                
                //set the next and size
                current_node->size = (tempSize - (size + sizeof(free_list_node)));
                current_node->next = tempNext;
                
                //set the allocated node's next to the other part of the node you just split
                allocated_node->next = current_node;
                
                //if the previous node is NULL then it is the head of the free list
                if(previous_node == NULL) {
                    //set free list to current node
                    free_list = current_node;
                    return allocated_node;
                } else {
                    //else set the previous node next to your current node
                    previous_node->next = current_node;
                    return allocated_node;
                }
            }
            
            //if there is not enough space on the free list and you are at the end of the free list
            else if (((current_node->size) < size) && ((current_node->next) == NULL)) {
                fprintf(stderr, "my_malloc: Scanning free list... Not enough space on free list\n");
                fprintf(stderr, "my_malloc: Calling sbrk() to expand heap\n");

                //grab a chunk of the heap
                ptr = sbrk(SIZE);
                
                //error check and return NULL on error
                if (ptr == (void*) -1) {
                    return NULL;
                }
                
                //set the pointer to next to the address of the current node + size of the current node + size of node
                current_node->next = (current_node + current_node->size + sizeof(free_list_node));
                
                //set the allocated node the newly called sbrk memory
                allocated_node = current_node->next;
                allocated_node->size = SIZE - sizeof(free_list_node);
                allocated_node->next = NULL;
                
                //set the current node to the newly allocated memory
                current_node = current_node->next;
                
                //set the new node's size to the size allocated - size of node
                current_node->size = SIZE - sizeof(free_list_node);
                
                //now that you've allocated more memory call my_malloc again
                my_malloc(size);
                return allocated_node;
            }
            
            //set previous node to the current node
            previous_node = current_node;
            //increment the current node to the next
            current_node = current_node->next;
        }
    }
}

void my_free(void *ptr) {
    
    //take the address and assign it as a free list node
    freeing_node = ptr;
    
    fprintf(stderr, "my_free: Called with 0x%x\n", freeing_node);
    
    //the current end of free list next will become the current address plus size of node
    //plus size of current node
    free_list_end->next = free_list_end + free_list_end->size + sizeof(free_list_node);
    
    //re-set the new free list end
    free_list_end = free_list_end->next;
    free_list_end->size = freeing_node->size;
    free_list_end->next = NULL;
}

void print_free_list() {
    
    //set number of nodes to 0
    nodeNum = 0;

    //set the head of the free list
    free_list_head = free_list;
    
    //if free list is empty then print out empty chart
    if (free_list == NULL) {
        printf("======================= FREE LIST =======================\n");
        printf("NODE #        |ADDRESS       |SIZE          |NEXT           \n");
        printf("=========================================================\n");
    } else {
        printf("======================= FREE LIST =======================\n");
        printf("NODE #        |ADDRESS       |SIZE          |NEXT           \n");
        
        //while the free list head is not NULL print out node's information
        while (free_list_head != NULL) {
            printf("%-14i%c%s%-12x%c%-14d%c%s%-12x\n", nodeNum, '|' , "0x", free_list_head ,'|', free_list_head->size, '|', "0x", free_list_head->next);
            
            //if the node's next is null that means it is the end of the free list and set free list end
            if (free_list_head->next == NULL) {
                free_list_end = free_list_head;
            }
            
            //increment current node and number of nodes
            free_list_head = free_list_head->next;
            nodeNum += 1;
        }
        
        printf("=========================================================\n");
    }
}
