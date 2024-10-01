#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct MemoryBlock {
    size_t size; // the size of the block memo
    bool is_free; // weather the block is used or not
    struct MemoryBlock* next; //a pointer to the next memo block
}MemoryBlock;


static void* memory_pool = NULL;
static MemoryBlock* free_lists = NULL;



void mem_init(size_t size) {
    size += sizeof(MemoryBlock);

    memory_pool = malloc(size);

    if (!memory_pool) {
        printf("It has failed to locate");
    }

    else {

    free_lists = (MemoryBlock*)memory_pool;
    free_lists -> size = size - sizeof(MemoryBlock);
    free_lists -> is_free = true;
    free_lists -> next = NULL;
    }
}


void* mem_alloc(size_t size) {
    MemoryBlock* current = free_lists;
    if (size >=sizeof(MemoryBlock)) {
        size -=sizeof(MemoryBlock);
    }
    //current -> size = current -> size + sizeof(MemoryBlock);

    // Traverse the free list to find a suitable block
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Split the block if it's larger than the requested size + overhead
            if (current->size >= size + sizeof(MemoryBlock)) {
                MemoryBlock* new_block = (MemoryBlock*)((char*)current + sizeof(MemoryBlock) + size);
                new_block->size = current->size - size - sizeof(MemoryBlock);
                new_block->is_free = true;
                new_block->next = current->next;
                
                current->next = new_block;
                current->size = size;
            }
            current->is_free = false;
            return (void*)((char*)current + sizeof(MemoryBlock));
        }
        current = current->next;
    }
    return NULL; // No suitable block found
}



void mem_free(void* block) {
        if (block == NULL) {
        return;
    }

    // Get the memory block metadata
    MemoryBlock* header = (MemoryBlock*)((char*)block - sizeof(MemoryBlock));
    header->is_free = true;

    // Coalesce adjacent free blocks
    MemoryBlock* current = free_lists;
    while (current != NULL) {
        if (current->is_free && current->next && current->next->is_free) {
            current->size += sizeof(MemoryBlock) + current->next->size;
            current->next = current->next->next;

        } else{
            current = current->next;
        }

    }
}


void* mem_resize(void* block, size_t size) {
    if (block == NULL) {
        return mem_alloc(size);
    }

    MemoryBlock* header = (MemoryBlock*)((char*)block - sizeof(MemoryBlock));
    if (header->size >= size) {
        return block; // No need to resize, current block is large enough
    }

    // Try to allocate a new block
    void* new_block = mem_alloc(size);
    if (new_block) {
        // Copy the old content to the new block
        memcpy(new_block, block, header->size);
        mem_free(block);
    }

    return new_block;
}



void mem_deinit() {
    free(memory_pool); // Free the entire memory pool
    memory_pool = NULL;
    free_lists = NULL;
}