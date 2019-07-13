#include <unistd.h>
#include "memory.h"

#ifndef NULL
#undef NULL

#define NULL ((void*) -1)

#endif

#define align4(x) (((((x) - 1) >> 2) << 2) + 4)

#define BLOCK_SIZE (sizeof(struct s_block) - 1)

typedef struct s_block s_block;

#pragma pack(push, 1)

struct s_block  {
    size_t size;
    s_block* next;
    s_block* prev;
    int free;
    void* ptr;
    char data[1];
};

#pragma pack(pop)

static int valid_block(void* ptr);

static void split_block(s_block* b, size_t size);

static void copy_block(s_block* src, s_block* dest);

static s_block* fusion(s_block* b);

static s_block* find_block(s_block** last, size_t size);

static s_block* extend_heap(s_block* last, size_t size);

static s_block* get_block(void* ptr);

static s_block* base;

void* malloc(size_t size) {
    s_block* b;
    s_block* last;

    size = align4(size);

    if (base) {
        last = base;

        b = find_block(&last, size);
        
        if (b) {
            if ((b->size - size) >= BLOCK_SIZE + 4) {
                split_block(b, size);
            }

            b->free = 0;
        } else {
            b = extend_heap(last, size);
            
            if (!b) {  
                return NULL; 
            }
        }
    } else {
        b = extend_heap(NULL, size);
        
        if (!b) {
            return NULL;
        }
    
        base = b;
    }

    return &(b->data);
}

void* calloc(size_t count, size_t size) {
    char* new = malloc(size);

    if (new) {
        for (int i = 0; i < size; i++) {
            new[i] = 0;
        }
    }

    return new;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) {
        return malloc(size);
    }
    
    if (valid_block(ptr)) {
        size = align4(size);
        
        s_block* b = get_block(ptr);
        
        if (b->size >= size) {
            if(b->size - size >= BLOCK_SIZE + 4) {
                split_block(b, size);
            }
        } else {
            void* new_ptr = malloc(size);
            
            if (!new_ptr) {
                return NULL;
            }
            
            s_block* new = get_block(new_ptr);

            copy_block(b, new);

            free(ptr);

            return new_ptr;
        }

        return ptr;
    }    

    return NULL;
}

void free(void* ptr) {
    s_block* b;

    if (valid_block(ptr)) {
        b = get_block(ptr);
        b->free = 1;
        
        if (b->prev && b->prev->free) {
            b = fusion(b->prev);    
        }
        
        if (b->next) {
            fusion(b);
        } else {
            if (b->prev) {
                b->prev->next = NULL;
            } else {
                base = NULL;
            }
            brk(b);
        }
    }
}

static int valid_block(void* ptr){
    if (base) {
        if (ptr > base && ptr < sbrk(0)) {
            s_block* b = get_block(ptr);
            return (ptr == b->ptr);
        }
    }

    return 0;
}

static void split_block(s_block* b, size_t size) {
    s_block* new;
    
    new = (void*)(&(b->data) + size);
    new->size = b->size - size - BLOCK_SIZE;
    new->next = b->next;
    new->prev = b;
    new->free = 1;
    new->ptr = new->data;
    
    b->size = size;
    b->next = new;

    if (new->next) {
        new->next->prev = new;
    }
}

static void copy_block(s_block* src, s_block* dest) {
    char* srcdata = src->ptr;
    char* destdata = dest->ptr;

    for (int i = 0; i < src->size && i < dest->size; i++) {
        destdata[i] = srcdata[i];
    } 
}

static s_block* fusion(s_block* b) {
    if (b->next && b->next->free) {
        b->size += BLOCK_SIZE + b->next->size;
        b->next = b->next->next;   

        if (b->next) {
            b->next->prev = b;            
        }
    }

    return b;
}

static s_block* get_block(void* ptr) {
    return (char*)ptr - BLOCK_SIZE;
}

static s_block* find_block(s_block** last, size_t size) {
    s_block* b = base;   
    
    while(b && !(b->free && b->size >= size)) {
        *last = b;
        b = b->next;
    } 

    return b;    
}

static s_block* extend_heap(s_block* last, size_t size) {
    s_block* b = sbrk(0);

    if(sbrk(BLOCK_SIZE + size) == NULL) {
        return NULL;
    }
    
    b->size = size;
    b->next = NULL;
    b->prev = last;
    b->ptr = &(b->data);
    
    if (last) {
        last->next = b;
    }    
    
    b->free = 0;
    
    return b;
}
