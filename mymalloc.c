#define _DEFAULT_SOURCE
#define _BSD_SOURCE 
#include <malloc.h> 
#include <stdio.h> 
#include <unistd.h>
#include <assert.h>
#include <string.h>

// Include any other headers we need here

// NOTE: You should NOT include <stdlib.h> in your final implementation

#include <debug.h> // definition of debug_printf

struct block *findNextFree(struct block **recent, size_t s);
struct block *addMoreSpace(struct block* recent, size_t s);
struct block *findMemPointer(void *p);

void *list_top = NULL;

// given struct from assignment
typedef struct block {
  size_t size;        // How many bytes beyond this block have been allocated in the heap
  struct block *next; // Where is the next block in your linked list
  int free;           // Is this memory free, i.e., available to give away?
  int debug;          // (optional) Perhaps you can embed other information--remember,
                      // you are the boss!
} block_t;

// finds the next free block to use for mallocs
struct block *findNextFree(struct block **recent, size_t s){
  // get linked list head
  struct block *current = list_top;

  // check if current index exists + is not free and not full
  while(current
	&& !(current->free && current->size >= s)){
    // set current
    *recent = current;
    current = current -> next;
  }
  // return current (if it fails above, it is free)
  return current;
}

// requests more space from sbrk
struct block *addMoreSpace(struct block* recent, size_t s){
  // create new memory block
  struct block *newBlock;

  // gets the memory using sbrk
  newBlock = sbrk(0);
  void *req = sbrk(s + sizeof(block_t));
  assert((void*)newBlock == req);
  if(req == (void*) -1){
    debug_printf("sbrk error :(");
    return NULL;
  }

  // this should be null on the first run bc its a linked list
  if(recent) {
    recent->next = newBlock;
  }

  // update the new memory block
  newBlock->size = s;
  newBlock->next = NULL;
  newBlock->free = 0;
  newBlock->debug = 1;
  return newBlock;
}

// helper to get address of the memory block
struct block *findMemPointer(void *p){
  return (struct block*) p - 1;
}

// malloc implementation
void *mymalloc(size_t s) {
  // illegal malloc call
  // void *p = (void *) malloc(s); // In your solution no calls to malloc should be
                               // made! Determine how you will request memory :)

  // 2 cases: first call and free block

  // create the block we're allocating 
  struct block *memoryBlock;

  // Case 1: first call of malloc
  if(!list_top) {
    // No space exists in the linked list, so we immediately just ask for more
    memoryBlock = addMoreSpace(NULL, s);
    // getting more space failed :(
    if(!memoryBlock){
      debug_printf("Adding more memory on first call failed.");
      return NULL;
    }
    // assign the top of the linked list to the next item
    list_top = memoryBlock;
  } else {
    // Not the first call, so we look for any free space in the list
    struct block *recent = list_top;
    memoryBlock = findNextFree(&recent, s);
    // No free space, so we ask for more
    if(!memoryBlock){
      memoryBlock = addMoreSpace(recent, s);
      // Asking for more failed :(
      if(!memoryBlock){
	debug_printf("Asking for more space failed on the not-first call.");
	return NULL;
      }
      // A free block of memory was found, we'll assign that
    } else {
      memoryBlock->free = 0;
      memoryBlock->debug = 2;
    }
  }

  debug_printf("Malloc %zu bytes\n", s);
  // return the pointer (p* from earlier)
  return(memoryBlock + 1);
}

void *mycalloc(size_t nmemb, size_t s) {
  /*
  void *p = (void *) calloc(nmemb, s); // In your solution no calls to calloc should be
                                       // made! Determine how you will request memory :)

  if (!p) {
    // We are out of memory
    // if we get NULL back from malloc
  }


  return p;
  */

  // gets size of call
  size_t mallocSize = nmemb * s;

  // gets memory
  void *p = mymalloc(mallocSize);
  
  // sets all memory from malloc to 0
  memset(p, 0, mallocSize);

  
  if(!p){
    debug_printf("Calloc failed, out of memory");
    // do something here, memory overflow
  }
  
  debug_printf("calloc %zu bytes\n", s);
  return p;
}

void myfree(void *ptr) {
  //debug_printf("Freed some memory\n");

  // Replace the free below with your own free implementation
  //free(ptr);
  
  // if pointer doesn't exist?
  if(!ptr) {
    debug_printf("Tried to free memory from a location that doesn't exist.");
    return;
  }

  // update pointer for freeing
  struct block* memPtr = findMemPointer(ptr);
  assert(memPtr->free == 0);
  assert(memPtr->debug == 2 || memPtr->debug == 1);
  memPtr->free = 1;
  memPtr->debug = 0;

  debug_printf("Freed %zu bytes\n", memPtr->size);
}
