//SSOO-P3 23/24

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"

//To create a queue
queue* queue_init(int num_elements)
{
  queue* q = (queue *)malloc(sizeof(queue));
  if (q == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  q->buffer = malloc(sizeof(struct element) * num_elements);
  if (q->buffer == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  q->capacity = num_elements;
  q->front = 0;
  q->rear = -1;
  q->count = 0;
  return q;
}

// To Enqueue an element
int queue_put(queue *q, struct element* ele) {
    while (queue_full(q)) {
        // Busy wait if the queue is full
    }

    // Add the element to the rear of the queue
    q->rear = (q->rear + 1) % q->capacity;
    q->buffer[q->rear] = *ele;
    q->count++;

    return 0; // Return 0 to indicate success
}

// To Dequeue an element.
struct element* queue_get(queue *q)
{
  while (queue_empty(q)) {}
  // Get the element at the front of the queue
  struct element* element = (struct element *) &(q->buffer[q->front]);

  // Update the front and count of the queue
  q->front = (q->front + 1) % q->capacity;
  q->count--;
  return element;
}

//To check queue state
int queue_empty(queue *q) {
    if (q->count == 0) {
        return 1; // Queue is empty
    } else {
        return 0; // Queue is not empty
    }
}

int queue_full(queue *q) {
    if (q->count == q->capacity) {
        return 1; // Queue is full
    } else {
        return 0; // Queue has available positions
    }
}

//To destroy the queue and free the resources
int queue_destroy(queue *q)
{

    free(q->buffer);
    free(q);
    return 0;
}
