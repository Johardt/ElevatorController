#include "queue.h"
#include <stdlib.h>


// Helper method for insert. Do not use.
void insertAfter(QueueNode* after, int content);

PriorityQueue* createPriorityQueue(sortingOrder order) {
    PriorityQueue* queue = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    QueueNode* first = (QueueNode*) malloc(sizeof(QueueNode));
    queue->first = first;
    queue->order = order;
    queue->size = 0;
    return queue;
}

void insert(PriorityQueue* queue, int content) {
    QueueNode* current = queue->first;
    while (current->next != NULL) {
        if (current->next->content == content) {
            return; // We don't want to save the same value twice.
        }
        if (queue->order == ASCENDING && current->next->content - content > 0) {
            insertAfter(current, content);
            queue->size++;
            return;
        }
        else if (queue->order == DESCENDING && current->next->content - content < 0) {
            insertAfter(current, content);
            queue->size++;
            return;
        }
        else {
            current = current->next; // Continue
        }
    }
    insertAfter(current, content);
    queue->size++;
}

int extractNext(PriorityQueue* queue) {
    if (queue->first->next == NULL) {
        return -100;
    }
    QueueNode* node = queue->first->next;
    queue->first->next = node->next;
    int value = node->content;
    free(node);
    queue->size--;
    return value;
}

void insertAfter(QueueNode* after, int content) {
    QueueNode* newNode = (QueueNode*) malloc(sizeof(QueueNode));
    newNode->content = content;
    newNode->next = after->next;
    after->next = newNode;
}