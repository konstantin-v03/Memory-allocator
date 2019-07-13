#include <stdio.h>
#include <limits.h>
#include "memory.h"

//BE CAREFUL
//If you try to print value using printf(...) heap may be corrupted   
//due to the internall heap allocation in printf that allocate memory over the 
//memory that was allocated with malloc(...) in "memory.h"

int main() {
    int* a = malloc(sizeof(int));
    int* b = malloc(sizeof(int));
    
    *a = 102;
    *b = INT_MIN;

    int* c = malloc(2 * sizeof(int));
        
    c[0] = INT_MAX;
    c[1] = INT_MAX;

    free(a);
    free(b);
    free(c);

    return 0;
}
