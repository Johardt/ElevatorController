#include "constants.h"

typedef enum {
    ASCENDING = -1,
    DESCENDING = 1
} sortingOrder;

typedef struct {
    int content[NUMBER_OF_FLOORS];
    int size;
    sortingOrder order;
} PriorityQueue;

PriorityQueue* createPriorityQueue(sortingOrder order);

// Inserts the element into the queue (lowest to highest).
// Does nothing if the value is already in the queue.
void insert(PriorityQueue* queue, int content);

// Get the element at the front of the queue
int extractNext(PriorityQueue* queue);