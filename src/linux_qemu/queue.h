#define MAX_QUEUE_SIZE 500

typedef struct {
    int data[MAX_QUEUE_SIZE];
    int front;
    int rear;
} Queue;

void initQueue(Queue* queue);
int isQueueEmpty(const Queue* queue);
int isQueueFull(const Queue* queue);
void enqueue(Queue* queue, int item);
int dequeue(Queue* queue);