// OS-P3 2022-2023

#include "queue.h"
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// To create a queue
queue *queue_init(int size) {
  queue *q = (queue *)malloc(sizeof(queue));

  q->buff_size = size;
  q->elements = malloc(sizeof(struct element) * size);
  q->num_elements = 0;
  q->head = 0;
  q->tail = 0;
  q->numop = 0;
  return q;
}

// To Enqueue an element
int queue_put(queue *q, struct element *x) {
  // printf("Recieved we are going to introduce command: %s %d %d, Num elements
  // is %d, bufsize = %d\n", x->command, x->account, x->quantity, q->
  // num_elements, q->buff_size);

  q->numop++;
  x->numop = q->numop;
  q->num_elements++;
  q->elements[q->tail] = *x;
  q->tail = (q->tail + 1) % q->buff_size;

  return 0;
}

// To Dequeue an element.
struct element *queue_get(queue *q) {
  // printf("Received lets get it\n");
  struct element *element = (struct element *)malloc(sizeof(struct element));

  *element = q->elements[q->head];
  q->head = (q->head + 1) % q->buff_size;
  q->num_elements--;
  return element;
}

// To check queue state
int queue_empty(queue *q) {
  if (q->num_elements == 0) {
    return 1;
  }
  return 0;
}

int queue_full(queue *q) {
  if (q->num_elements >= q->buff_size) {
    return 1;
  }
  return 0;
}

// To destroy the queue and free the resources
int queue_destroy(queue *q) {
  free(q->elements);
  free(q);
  return 0;
}
