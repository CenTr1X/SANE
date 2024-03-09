#include <stdio.h>
#include "queue.h"

void initQueue(Queue* queue) {
    queue->front = 0;
    queue->rear = -1;
}

int isQueueEmpty(const Queue* queue) {
    return (queue->rear < queue->front);
}

int isQueueFull(const Queue* queue) {
    return (queue->rear >= MAX_QUEUE_SIZE - 1);
}

void enqueue(Queue* queue, int item) {
    if (isQueueFull(queue)) {
        printf("queue full\n");
        return;
    }
  
    queue->rear++;
    queue->data[queue->rear] = item;
}

int dequeue(Queue* queue) {
    if (isQueueEmpty(queue)) {
        printf("queue empty\n");
        return -1;
    }
  
    int item = queue->data[queue->front];
    queue->front++;
    return item;
}
