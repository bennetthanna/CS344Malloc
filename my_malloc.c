#include "my_malloc.h"

void *ptr;
free_list_node *free_list;
free_list_node *current_node;
free_list_node *tempNext;
free_list_node *previous_node;
free_list_node *free_list_head;
free_list_node *free_list_end;
free_list_node *allocated_node;
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
            printf("ptr = 0x%x\n", ptr);
            printf("free_list = 0x%x\n", free_list);
            
            //move free list to point to actual beginning of memory block ignoring node with size and next
            free_list = free_list + sizeof(free_list_node);
            printf("new free_list = 0x%x\n", free_list);
            
            fprintf(stderr, "new PB after sbrk() = 0x%x\n", sbrk(0));
            fprintf(stderr, "address = 0x%x\n", free_list);
            
            //set free list size to the size you called sbrk with minus the size of the node that holds size and next
            free_list->size = SIZE - sizeof(free_list_node);
            fprintf(stderr, "size = %i\n", free_list->size);
            //set free list next to NULL
            free_list->next = NULL;
            fprintf(stderr, "next = 0x%x\n", free_list->next);

        }

        //set current node to the free list
        current_node = free_list;
        //set previous node to NULL
        previous_node = NULL;
        
        //while current node is not NULL
        while(current_node != NULL) {
            
            //if the available memory is the same as size requested
            if(current_node->size == size) {
                fprintf(stderr, "my_malloc: Sizes are equal\n");
                //if there is no previous node - the current node is the head of the free list
                if(previous_node == NULL) {
                    printf("Previous node is NULL\n");
                    printf("Current_node->next = 0x%x\n", current_node->next);
                    //set the current node to the next node
                    current_node = current_node->next;
                    //set head of free list to the new current node as that will be the new head
                    free_list = current_node;
                    return current_node;
                } else {
                    printf("Else");
                    //else set the take the previous node's next to the current node's next
                    previous_node->next = current_node->next;
                    return previous_node;
                }

            }
            
            //if the available memory is greater than the size requested
            else if ((current_node->size) > size) {
                fprintf(stderr, "my_malloc: Scanning free list... Sufficient space on free list\n");
                
                //set current node's next to temp to hold onto
                tempNext = current_node->next;
                //set current node's size to tempSize to hold onto
                tempSize = current_node->size;
                
                allocated_node = current_node;
                allocated_node->size = size;
                
                fprintf(stderr, "current address = 0x%x\n", current_node);
                //move pointer to new head of free list by adding size requested plus size of node
                //you must add size of node as well to ignore the part that is just size and next pointer
                //this puts the current node address to the beginning of usable memory
                current_node += (size + sizeof(free_list_node));
                fprintf(stderr, "new address = 0x%x\n", current_node);
                
                //set free list next and size
                current_node->size = (tempSize - (size + sizeof(free_list_node)));
                fprintf(stderr, "size = %i\n", current_node->size);
                current_node->next = tempNext;
                fprintf(stderr, "next = 0x%x\n", current_node->next);
                
                //WHAT IS THIS LINE DOING?
                /* Should I do that if previous node is NULL then current node will be new head of free list
                 * free_list = current_node, return current_node
                 * else just return current_node because that means it was in the middle and we don't have to change
                 * head of free list?
                 */
                
                if(previous_node == NULL) {
                    printf("free_list = current_node\n");
                    free_list = current_node;
                    printf("returning current node\n");
                    return current_node;
                } else {
                    previous_node->next = current_node;
                    printf("else returning current node\n");
                    printf("current node = 0x%x\n", current_node);
                    printf("current node size = %i\n", current_node->size);
                    return current_node;
                }
            }
            
            //if there is not enough space on the free list and you are at the end of the free list
            else if (((current_node->size) < size) && ((current_node->next) == NULL)) {
                fprintf(stderr, "my_malloc: Scanning free list... Not enough space on free list\n");
                fprintf(stderr, "my_malloc: Calling sbrk() to expand heap\n");
                //grab a chunk of the heap
                sbrk(SIZE);
                //set the pointer to next to the address of the current node + size of the current node + size of node
                current_node->next = (current_node + current_node->size + sizeof(free_list_node));
                fprintf(stderr, "new next = 0x%x\n", current_node->next);
                //set the current node to the newly allocated memory
                current_node = current_node->next;
                //set the new node's size to the size allocated - size of node
                current_node->size = SIZE - sizeof(free_list_node);
                fprintf(stderr, "new size = %i\n", current_node->size);
                //now that you've allocated more memory call my_malloc again
                my_malloc(size);
//                return current_node;
            }
            
            //set previous node to the current node
            previous_node = current_node;
            //increment the current node to the next
            current_node = current_node->next;
        }
    }
}

free_list_node *freeing_node;

void my_free(void *ptr) {
    //set address wanting to be freed to free_list_node
    //because the address given to us is the address that was returned by malloc when called
    //the address is past the address of the allocated memory so we have to move the address
    //back by taking address give (freeing_node) minus the size minus the size of the node
    //where the size and next pointer are held
    freeing_node = ptr;
    fprintf(stderr, "my_free: Called with 0x%x\n", (freeing_node - freeing_node->size - sizeof(free_list_node)));
    fprintf(stderr, "size to put back on free list = %i\n", freeing_node->size);
    //the current end of free list next will become the current address plus size of node
    //plus size of current node
    free_list_end->next = free_list_end + free_list_end->size + sizeof(free_list_node);
    free_list_end = free_list_end->next;
    freeing_node = freeing_node - freeing_node->size - sizeof(free_list_node);
    free_list_end->size = freeing_node->size;
    fprintf(stderr, "free list end size = %i\n", free_list_end->size);
    free_list_end->next = NULL;
    
}

void print_free_list() {
    //set number of nodes to 0
    nodeNum = 0;

    /* This is just getting the current node that is returned rather than iterating through the whole free list
     * I must figure out the head of the free list, set that to some variable free_list_head
     * Then set free_list_head = free_list
     * Then iterate through while free_list_head != NULL
     * And incrememnt free_list_head to free_list_head->next
     */
    
    free_list_head = free_list;
    
    if (free_list == NULL) {
        printf("======================= FREE LIST =======================\n");
        printf("NODE #        |ADDRESS       |SIZE          |NEXT           \n");
        printf("=========================================================\n");
    } else {
        printf("======================= FREE LIST =======================\n");
        printf("NODE #        |ADDRESS       |SIZE          |NEXT           \n");
        while (free_list_head != NULL) {
            printf("%-14i%c%s%-12x%c%-14d%c%s%-12x\n", nodeNum, '|' , "0x", free_list_head ,'|', free_list_head->size, '|', "0x", free_list_head->next);
            
            //if the node's next is null that means it is the end of the free list
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

int main() {
//    printf("main: Printing free_list\n");
//    print_free_list();
//    printf("main: Error checking size input\n");
//    my_malloc(-12);
//    my_malloc(3048);
//    printf("main: Allocating an array of 100 doubles\n");
    print_free_list();
    my_malloc(800);
    print_free_list();
    printf("main: Allocating ints\n");
    int *x = (int*)my_malloc(100 * sizeof(int));
    printf("int *x = 0x%x\n", x);
    print_free_list();
    my_malloc(1000);
    print_free_list();
    
    printf("main: Freeing ints\n");
    my_free(x);
    print_free_list();
}
