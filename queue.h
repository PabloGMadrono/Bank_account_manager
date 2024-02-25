#ifndef HEADER_FILE
#define HEADER_FILE

struct element {
  // Define the struct yourself
  char *command;
  int account;
  int quantity;
  int trans_quant;
  int numop;
};

typedef struct queue {
  // Define the struct yourself
  struct element *elements;
  int numop, head, tail, num_elements, buff_size;
} queue;

queue *queue_init(int size);
int queue_destroy(queue *q);
int queue_put(queue *q, struct element *elem);
struct element *queue_get(queue *q);
int queue_empty(queue *q);
int queue_full(queue *q);

#endif
