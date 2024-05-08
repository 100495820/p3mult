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
pthread_mutex_t profits_mutex; // controls accesses to profits
pthread_cond_t notfull; // condition variable for full queue
pthread_cond_t notempty; // condition variable for empty queue


struct ThreadData { // Struct to pass data to threads
    int start; // Start index of operations for the thread
    int end;   // End index of operations for the thread
    int consum;
} ThreadData;

struct element *elem; // Array of elements to store the operations

void *producer(struct ThreadData *arg) { // Producer thread
    struct ThreadData *p = (struct ThreadData *) arg; // Casting the argument to the struct
    int start = p->start; // Start index of operations
    int end = p->end; // End index of operations
    for (int i = start; i < end; i++) { // Loop through the operations

        pthread_mutex_lock(&mutex); // Lock the mutex
        while (queue_full(q) == 1) { // If the queue is full, wait
            pthread_cond_wait(&notfull, &mutex); // Wait for the condition variable
        }

        queue_put(q, &elem[i]); // Put the element in the queue


        if (queue_empty(q)==0){ // If the queue is not empty, signal the condition variable
            pthread_cond_signal(&notempty); // Signal the condition variable
        }
        pthread_mutex_unlock(&mutex); // Unlock the mutex

    }
    pthread_exit(0); // Exit the thread

}


void *consumer(struct ThreadData *arg) { // Consumer thread
  struct ThreadData *c = (struct ThreadData *) arg; // Casting the argument to the struct
  int consum = c->consum; // Number of operations to consume
    while(consum>0)  { // Loop through the operations

    pthread_mutex_lock(&mutex); // Lock the mutex
        while (queue_empty(q)==1) // If the queue is empty, wait
          {
            pthread_cond_wait(&notempty, &mutex); // Wait for the condition variable
        }

    struct element *ele = queue_get(q); // Get the element from the queue

    if (ele->op == 1) { // If the operation is a purchase
        if (ele->product_id == 1) { // If the product is 1
            product_stock[0] += ele->units; // Add the units to the stock
            profits -= ele->units * 2; // Subtract the units from the profits
        }
        if (ele->product_id == 2) { // If the product is 2
            product_stock[1] += ele->units; // Add the units to the stock
            profits -= ele->units * 5; // Subtract the units from the profits
        }
        if (ele->product_id == 3) { // If the product is 3
            product_stock[2] += ele->units; // Add the units to the stock
            profits -= ele->units * 15; // Subtract the units from the profits
        }
        if (ele->product_id == 4) { // If the product is 4
            product_stock[3] += ele->units; // Add the units to the stock
            profits -= ele->units * 25; // Subtract the units from the profits
        }
        if (ele->product_id == 5) { // If the product is 5
            product_stock[4] += ele->units; // Add the units to the stock
            profits -= ele->units * 100; // Subtract the units from the profits
        }
    } else {
        if (ele->product_id == 1) { // If the product is 1
            product_stock[0] -= ele->units; // Subtract the units from the stock
            profits += ele->units * 3; // Add the units to the profits
        }
        if (ele->product_id == 2) { // If the product is 2
            product_stock[1] -= ele->units; // Subtract the units from the stock
            profits += ele->units * 10; // Add the units to the profits
        }
        if (ele->product_id == 3) { // If the product is 3
            product_stock[2] -= ele->units; // Subtract the units from the stock
            profits += ele->units * 20; // Add the units to the profits
        }
        if (ele->product_id == 4) { // If the product is 4
            product_stock[3] -= ele->units; // Subtract the units from the stock
            profits += ele->units * 40; // Add the units to the profits
        }
        if (ele->product_id == 5) { // If the product is 5
            product_stock[4] -= ele->units; // Subtract the units from the stock
            profits += ele->units * 125; // Add the units to the profits
        }
        }

        if (queue_full(q) == 0){ // If the queue is not full, signal the condition variable
            pthread_cond_signal(&notfull); // Signal the condition variable
        }
        pthread_mutex_unlock(&mutex); // Unlock the mutex
        consum--; // Decrease the number of operations to consume
    }

    pthread_exit(0); // Exit the thread
}

int main (int argc, const char * argv[])
{

  if (argc > 5) { // Check the number of parameters
      printf("Invalid number of parameters\n"); // Print an error message
      return -1;
  }
  if (argc < 5) { // Check the number of parameters
      printf("Invalid number of parameters\n"); // Print an error message
      return -1;
  }
  int n=atoi(argv[2]); // Number of producers
  if (n < 1){ // Check if the number of producers is less than 1
	printf("Invalid number of producers\n"); // Print an error message
	return -1;
	}
  int m=atoi(argv[3]); // Number of consumers
  if (m < 1){ // Check if the number of consumers is less than 1
	printf("Invalid number of consumers\n"); // Print an error message
	return -1;
	}
  int buffersize=atoi(argv[4]); // Size of the buffer
	if (buffersize <= 1){ // Check if the buffer size is less than or equal to 1
	printf("Invalid buffersize\n"); // Print an error message
	return -1;
	}
  pthread_mutex_init(&mutex, NULL); // Initialize the mutex
  pthread_mutex_init(&profits_mutex,NULL); // Initialize the profits mutex
  pthread_cond_init(&notfull, NULL); // Initialize the condition variable
  pthread_cond_init(&notempty, NULL); // Initialize the condition variable

  FILE *descriptor = fopen(argv[1], "r"); // Open the file
  if(descriptor == NULL) { // Check if the file can be opened
      printf("Can't open file\n"); // Print an error message
      exit(-1);
  }

  if(fscanf(descriptor, "%d", &operations) < 0){ // Read the number of operations
      printf("Can't read file\n"); // Print an error message
      exit(-1);
  }

  char line[256]; // Buffer to store the lines
  int oper = -1; // Index of the operations
  elem=malloc(operations*sizeof(struct element)); // Allocate memory for the operations
  while (fgets(line, sizeof(line), descriptor)) { // Read the lines of the file
      struct element *ele = malloc(sizeof(struct element)); // Allocate memory for the element

      char operation[10]; // Buffer to store the operation

      sscanf(line, "%d %s %d", &(ele->product_id), operation, &(ele->units)); // Read the operation
      //printf("Product: %d, Operation: %s, Units: %d\n", ele->product_id, operation, ele->units);
      if (strcmp(operation, "PURCHASE") == 0) { // Check the operation
          ele->op = 1; // Let's assume 1 for PURCHASE
      } else if (strcmp(operation, "SALE") == 0) { // Check the operation
          ele->op = 0; // and 0 for SALE
        }
	if (oper==-1){ // Check if the operation is the first one
	oper+=1; // Increase the index
  free(ele); // Free the memory

	}
	else{
	elem[oper] = *ele; // Store the element in the array
      	oper += 1; // Increase the index

	}

  }

  fclose(descriptor); // Close the file


  int extraproducers = operations % n; // Extra producers
  int nthreads = operations/n; // Number of operations per producer
  int extraconsumers = operations % m; // Extra consumers
  int mthreads = operations/m; // Number of operations per consumer
  pthread_t thrs[n]; // Array of producer threads
  pthread_t thrm[m]; // Array of consumer threads
  q = queue_init(buffersize); // Initialize the queue

  for (int i = 0; i < n; i++) { // Loop through the producers
          struct ThreadData *datas = malloc(sizeof(ThreadData)); // Allocate memory for the data

          datas->start = i * nthreads; // Start index of operations
          if (i == n - 1) { // Check if it is the last producer
              datas->end = (i + 1) * nthreads + extraproducers; // End index of operations
          } else {
              datas->end = (i + 1) * nthreads; // End index of operations
          }

          pthread_create(&thrs[i], NULL, (void *) producer, datas); // Create the producer thread


   }

    for (int a=0;a<m;a++){
      struct ThreadData *dataconsum = calloc(1,sizeof(ThreadData)); // Allocate memory for the data

      if (a == m-1){
        dataconsum->consum=mthreads+extraconsumers; // Number of operations to consume
      }
      else{
      dataconsum->consum=mthreads; // Number of operations to consume
      }
      pthread_create(&thrm[a], NULL, (void *) consumer, dataconsum); // Create the consumer thread
    }
    for (int b=0;b<n;b++){
        pthread_join(thrs[b],NULL); // Join the producer threads
    }
    for (int c=0;c<m;c++){
        pthread_join(thrm[c],NULL); // Join the consumer threads
    }

  // Output
  printf("Total: %d euros\n", profits);
  printf("Stock:\n");
  printf("  Product 1: %d\n", product_stock[0]);
  printf("  Product 2: %d\n", product_stock[1]);
  printf("  Product 3: %d\n", product_stock[2]);
  printf("  Product 4: %d\n", product_stock[3]);
  printf("  Product 5: %d\n", product_stock[4]);


    pthread_mutex_destroy(&mutex); // Destroy the mutex
    pthread_mutex_destroy(&profits_mutex); // Destroy the mutex
    pthread_cond_destroy(&notfull); // Destroy the condition variables
    pthread_cond_destroy(&notempty); // Destroy the condition variables

	free(elem); // Free the memory

    queue_destroy(q); // Destroy the queue
    return 0;
}
