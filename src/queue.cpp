#include "queue.h"
#include <stdlib.h>
#include "Arduino.h"

PriorityQueue* createPriorityQueue(sortingOrder order) {
    PriorityQueue* queue = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    queue->order = order;
    queue->size = 0;
    return queue;
}

void insert(PriorityQueue* queue, int content) {
    if (content >= NUMBER_OF_FLOORS) {
        return;
    } else {
        queue->content[content] = 1;
        queue->size += 1;
    }
}

int extractNext(PriorityQueue* queue) {
    if (queue->order == ASCENDING) {
        for (int i = 0; i < NUMBER_OF_FLOORS; i++) {
            if (queue->content[i] == 1) {
                queue->content[i] = 0;
                queue->size -= 1;
                return i;
            }
        }
    } else if (queue->order == DESCENDING) {
        for (int i = NUMBER_OF_FLOORS - 1; i >= 0; i--) {
            if (queue->content[i] == 1) {
                queue->content[i] = 0;
                queue->size -= 1;
                return i;
            }
        }
    }
    return -100;
}