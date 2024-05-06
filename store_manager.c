//SSOO-P3 23/24
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


int profits = 0;
int product_stock [5] = {0};
queue *q;
int operations;
pthread_mutex_t mutex; // controls accesses to shared resources
pthread_mutex_t producer_mutex; // controls a unique access of an producer
pthread_mutex_t consumer_mutex; // controls a unique access of a consumer
pthread_cond_t notfull;

struct ThreadData {
    int start; // Start index of operations for the thread
    int end;   // End index of operations for the thread
} ThreadData;

struct element *elem;

void *producer(struct ThreadData *arg) {
    struct ThreadData *p = (struct ThreadData *) arg;
    int start = p->start;
    int end = p->end;
    for (int i = start; i < end; i++) {
        pthread_mutex_lock(&producer_mutex);
        pthread_mutex_lock(&mutex);
        if (queue_full(q) == 1) {
            pthread_cond_wait(&notfull, &mutex);
        }
        queue_put(q, &elem[i]);
        if (queue_full(q)==0){
            pthread_cond_signal(&notfull);
        }

        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&producer_mutex);
    }
    pthread_exit(0);

}

int consuop=0;
void *consumer(void) {
    while (consuop<operations) {
        pthread_mutex_lock(&consumer_mutex);
        if (consuop >= operations) {
            pthread_mutex_unlock(&consumer_mutex);
            break;
        }

    pthread_mutex_lock(&mutex);
        if (queue_empty(q)==1)
        {
            pthread_cond_wait(&notfull, &mutex);
        }

    struct element *ele = queue_get(q);
    if (ele->op == 1) {
        if (ele->product_id == 1) {
            product_stock[0] += ele->units;
            profits -= ele->units * 2;
        }
        if (ele->product_id == 2) {
            product_stock[1] += ele->units;
            profits -= ele->units * 5;
        }
        if (ele->product_id == 3) {
            product_stock[2] += ele->units;
            profits -= ele->units * 15;
        }
        if (ele->product_id == 4) {
            product_stock[3] += ele->units;
            profits -= ele->units * 25;
        }
        if (ele->product_id == 5) {
            product_stock[4] += ele->units;
            profits -= ele->units * 100;
        }
    } else {
        if (ele->product_id == 1) {
            product_stock[0] -= ele->units;
            profits += ele->units * 3;
        }
        if (ele->product_id == 2) {
            product_stock[1] -= ele->units;
            profits += ele->units * 10;
        }
        if (ele->product_id == 3) {
            product_stock[2] -= ele->units;
            profits += ele->units * 20;
        }
        if (ele->product_id == 4) {
            product_stock[3] -= ele->units;
            profits += ele->units * 40;
        }
        if (ele->product_id == 5) {
            product_stock[4] -= ele->units;
            profits += ele->units * 125;
        }
    }
        if (queue_full(q) == 0){
            pthread_cond_signal(&notfull);
        }
        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&consumer_mutex);
        consuop++;
    }
    pthread_exit(0);
}

int main (int argc, const char * argv[])
{

  if (argc > 5) {
      printf("Invalid number of parameters\n");
      return -1;
  }
  if (argc < 5) {
      printf("Invalid number of parameters\n");
      return -1;
  }
  int n=atoi(argv[2]);
  if (n < 1){
	printf("Invalid number of producers\n");
	return -1;
	}
  int m=atoi(argv[3]);
  if (m < 1){
	printf("Invalid number of consumers\n");
	return -1;
	}
  int buffersize=atoi(argv[4]);
	if (buffersize <= 1){
	printf("Invalid buffersize\n");
	return -1;
	}
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&producer_mutex, NULL);
  pthread_mutex_init(&consumer_mutex, NULL);

  pthread_cond_init(&notfull, NULL);


  FILE *descriptor = fopen(argv[1], "r");
  if(descriptor == NULL) {
      printf("Can't open file\n");
      exit(-1);
  }

  if(fscanf(descriptor, "%d", &operations) < 0){
      printf("Can't read file\n");
      exit(-1);
  }

  char line[256];
  int oper = -1;
  elem=malloc(operations*sizeof(struct element));
  while (fgets(line, sizeof(line), descriptor)) {
      struct element *ele = malloc(sizeof(struct element));

      char operation[10];

      sscanf(line, "%d %s %d", &(ele->product_id), operation, &(ele->units));
      //printf("Product: %d, Operation: %s, Units: %d\n", ele->product_id, operation, ele->units);
      if (strcmp(operation, "PURCHASE") == 0) {
          ele->op = 1; // Let's assume 1 for PURCHASE
      } else if (strcmp(operation, "SALE") == 0) {
          ele->op = 0; // and 0 for SALE
        }
	if (oper==-1){
	oper+=1;
  free(ele);

	}
	else{
	elem[oper] = *ele;
      	oper += 1;

	}

  }

  fclose(descriptor);


  int extraproducers = operations % n;
  int nthreads = operations/n;
  pthread_t thrs[n];
  pthread_t thrm[m];
  q = queue_init(buffersize);

  for (int i = 0; i < n; i++) {
          struct ThreadData *datas = malloc(sizeof(ThreadData));

          datas->start = i * nthreads;
          if (i == n - 1) {
              datas->end = (i + 1) * nthreads + extraproducers;
          } else {
              datas->end = (i + 1) * nthreads;
          }

          pthread_create(&thrs[i], NULL, (void *) producer, datas);


   }

    for (int a=0;a<m;a++){
        pthread_create(&thrm[a], NULL, (void *) consumer, NULL);
    }
    for (int b=0;b<n;b++){
        pthread_join(thrs[b],NULL);
    }
    for (int c=0;c<m;c++){
        pthread_join(thrm[c],NULL);
    }

  // Output
  printf("Total: %d euros\n", profits);
  printf("Stock:\n");
  printf("  Product 1: %d\n", product_stock[0]);
  printf("  Product 2: %d\n", product_stock[1]);
  printf("  Product 3: %d\n", product_stock[2]);
  printf("  Product 4: %d\n", product_stock[3]);
  printf("  Product 5: %d\n", product_stock[4]);

    // Destroying any mutex or condition
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&producer_mutex);
    pthread_mutex_destroy(&consumer_mutex);
    pthread_cond_destroy(&notfull);
// Free the previous space taken
	free(elem);
// Deleting the queue and its elements
    queue_destroy(q);
    return 0;
}
