// OS-P3 2022-2023

#include "queue.h"
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define buf 1

int client_numop = 0;
int bank_numop = 0;
int global_balance = 0;
int numaccs = 0;
int account_balance[200] = {0};

pthread_mutex_t mutex;
pthread_cond_t c1, c2;

queue *circular_buffer;

int atm = 0, worker = 0;
int max_operations;

/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */
void *ATM_thread(void *arg) {

  char **list_client_ops = (char **)arg;
  char *command = malloc(10 * sizeof(char)); // allocate memory for command
  while (1) {
    pthread_mutex_lock(&mutex);
  
    if (client_numop >= atoi(list_client_ops[0])) {
      break;
    }

    

    while (queue_full(circular_buffer) == 1) {
      pthread_cond_wait(&c1, &mutex);
    }
    if (client_numop >= atoi(list_client_ops[0])) {
      break;
    }
    struct element *element = (struct element *)malloc(sizeof(struct element));
    sscanf(list_client_ops[client_numop + 1], "%s %d %d %d", command,
           &element->account, &element->quantity, &element->trans_quant);
    element->command = strdup(command);
    queue_put(circular_buffer, element);
    client_numop++;
    pthread_cond_signal(&c2);
    pthread_mutex_unlock(&mutex);

  }
  pthread_cond_signal(&c2);
  pthread_mutex_unlock(&mutex);
  free(command);
  pthread_exit(NULL);
}

void *worker_thread(void *arg) {
  // Possible instructions, create, deposit, withdraw, balance, transfer

  
  struct element *instruction = (struct element *)malloc(sizeof(struct element));

  while(1){
    pthread_mutex_lock(&mutex);

    if (client_numop >= max_operations && client_numop <= bank_numop) {
      break;
    }

    if (bank_numop >= client_numop){
      pthread_cond_wait(&c2, &mutex);
      if ( client_numop >= max_operations && client_numop <= bank_numop) {
      break;
      }
    }
    while (queue_empty(circular_buffer) == 1) {
      if (client_numop >= max_operations && client_numop <= bank_numop) {
      break;
      }
      pthread_cond_wait(&c2, &mutex);
    }
  
    if (client_numop >= max_operations && client_numop <= bank_numop) {
      break;
    }
    if (numaccs < 0) {
      perror("No more accounts can be created");
      exit(-1);
      break;
    }

    instruction = queue_get(circular_buffer);

    if (strcmp(instruction->command, "CREATE") == 0) {
      account_balance[instruction->account] = 0;
      numaccs--;
      printf("%d %s %i  BALANCE = %i TOTAL = %i\n", instruction->numop,
             instruction->command, instruction->account,
             account_balance[instruction->account], global_balance);
    }

    if (strcmp(instruction->command, "DEPOSIT") == 0) {
      account_balance[instruction->account] =
          instruction->quantity + account_balance[instruction->account];
      global_balance = global_balance + instruction->quantity;
      printf("%d %s %i %i BALANCE = %i TOTAL = %i\n", instruction->numop,
             instruction->command, instruction->account, instruction->quantity,
             account_balance[instruction->account], global_balance);
    }
    if (strcmp(instruction->command, "TRANSFER") == 0) {
      account_balance[instruction->account] =
          account_balance[instruction->account] - instruction->trans_quant;
      account_balance[instruction->quantity] =
          account_balance[instruction->quantity] + instruction->trans_quant;
      printf("%d %s %i %i %i BALANCE = %i TOTAL = %i\n", instruction->numop,
             instruction->command, instruction->account, instruction->quantity,
             instruction->trans_quant, account_balance[instruction->quantity],
             global_balance);
    }
    if (strcmp(instruction->command, "WITHDRAW") == 0) {
      account_balance[instruction->account] =
          account_balance[instruction->account] - instruction->quantity;
      global_balance = global_balance - instruction->quantity;

      printf("%d %s %i %i BALANCE = %i TOTAL = %i\n", instruction->numop,
             instruction->command, instruction->account, instruction->quantity,
             account_balance[instruction->account], global_balance);
    }
    if (strcmp(instruction->command, "BALANCE") == 0) {
      account_balance[instruction->account] =
          account_balance[instruction->account] - instruction->quantity;
      printf("%d %s %i  BALANCE = %i TOTAL = %i\n", instruction->numop,
             instruction->command, instruction->account,
             account_balance[instruction->account], global_balance);
    }

    bank_numop++;
    pthread_cond_signal(&c1);
    pthread_mutex_unlock(&mutex);

  }
  pthread_cond_signal(&c1);
  pthread_mutex_unlock(&mutex);
  free(instruction);
  pthread_exit(NULL);
}

int main(int argc, const char *argv[]) {

  int i = 0, j = 0, k = 0, l = 0;
  pthread_t ATM[atoi(argv[2])], worker[atoi(argv[3])];
  int MAX_NUM_LINES = 1000;
  int MAX_LINE_LEN = 256;
  char **list_client_ops;
  
  FILE *fp;
  if (atoi(argv[2]) < 1 || atoi(argv[3]) < 1 || atoi(argv[5]) < 1 ||
      atoi(argv[4]) < 1) {
    return -1;
  }
  numaccs = atoi(argv[4]);
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&c2, NULL);
  pthread_cond_init(&c1, NULL);

  circular_buffer = queue_init(atoi(argv[5]));
  list_client_ops = (char **)malloc(MAX_NUM_LINES * sizeof(char *));

  fp = fopen(argv[1], "r");

  int line_ct = 0;
  char line[MAX_LINE_LEN];
  while (fgets(line, MAX_LINE_LEN, fp) != NULL) {
    int len = strlen(line);
    list_client_ops[line_ct] = (char *)malloc(MAX_LINE_LEN * sizeof(char));
    strcpy(list_client_ops[line_ct], line);
    line_ct++;
  }
  max_operations = atoi(list_client_ops[0]);
  int num_elements = 0;
  while (list_client_ops[num_elements] != NULL) {
    num_elements++;
  }

  if ((num_elements - 1) != atoi(list_client_ops[0]) ||
      atoi(list_client_ops[0]) > 200) {
    // perror("Error cantidad de operaciones maximas");
    exit(-1);
  }

  /*printf("File contents:\n");
  for (i = 0; i < atoi(list_client_ops[0]); i++){
    printf("%s", list_client_ops[i]);
  }*/

  // ATM creations
  while (k < atoi(argv[2]) && l < atoi(argv[3])) {
    if (k < atoi(argv[2])) {
      // printf("I am the ATM");
      if (pthread_create(&ATM[k], NULL, &ATM_thread, list_client_ops) != 0) {
        printf("%d\n", -3);
      }
    }

    // workers creation
    if (l < atoi(argv[3])) {
      // printf("I am the worker");
      if (pthread_create(&worker[l], NULL, &worker_thread, NULL) !=
          0) {
        printf("%d\n", -4);
      }
    }
    k++;
    l++;
  }
  // ATM waiting
  while (i < atoi(argv[2]) && j < atoi(argv[3])) {
    if (i < atoi(argv[2])) {
      if (pthread_join(ATM[i], NULL) != 0) {
        printf("%d\n", -5);
      }
    }

    // workers waiting
    if (j < atoi(argv[3])) {
      if (pthread_join(worker[j], NULL) != 0) {
        printf("%d\n", -6);
      }
    }
    i++;
    j++;
  }

  for (int i = 0; i < line_ct; i++) {
    free(list_client_ops[i]);
  }

  queue_destroy(circular_buffer);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&c1);
  pthread_cond_destroy(&c2);
  free(list_client_ops);
  return 0;
}
