#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

/*
 *
 ******Each time the program is run, the generated output file must be deleted.
 ******If it is not deleted, the results will be added to the bottom of the previous output file.
 *
 *
 * Ali Akif Yörük - 21328648
 */



pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_mutex_t subBoxLock;
pthread_mutex_t authenticationLock;

sem_t empty_semaphore, full_semaphore;
sem_t empty_semaphore2, full_semaphore2;
sem_t empty_subBox, full_subBox;
sem_t empty_authentication, full_authentication;

int buffer=0;
struct Queue
{
    int front, rear, size;
    int **array ;

};
int * keyBlock;
unsigned char SUBBOX[256] = {47, 164, 147, 166, 221, 246, 1, 13, 198, 78, 102, 219, 75, 97, 62, 140,
			     84, 69, 107, 99, 185, 220, 179, 61, 187, 0, 92, 112, 8, 33, 15, 119,
			     209, 178, 192, 12, 121, 239, 117, 96, 100, 126, 118, 199, 208, 50, 42, 168,
			     14, 171, 17, 238, 158, 207, 144, 58, 127, 182, 146, 71, 68, 157, 154, 88,
			     248, 105, 131, 235, 98, 170, 22, 160, 181, 4, 254, 70, 202, 225, 67, 205,
			     216, 25, 43, 222, 236, 128, 122, 77, 59, 145, 167, 54, 20, 55, 152, 149,
			     230, 211, 224, 111, 165, 124, 16, 243, 213, 114, 116, 63, 64, 176, 31, 161,
			     9, 229, 95, 247, 193, 18, 134, 79, 133, 173, 82, 51, 57, 136, 6, 49,
			     5, 197, 115, 65, 169, 255, 249, 195, 30, 162, 150, 53, 83, 46, 228, 81,
			     237, 104, 28, 223, 217, 251, 200, 60, 132, 194, 151, 137, 191, 74, 201, 103,
			     29, 80, 113, 101, 250, 172, 234, 180, 73, 141, 204, 27, 241, 188, 153, 155,
			     86, 94, 177, 87, 39, 91, 2, 48, 35, 40, 120, 159, 184, 123, 215, 138,
			     210, 108, 76, 106, 36, 189, 125, 226, 252, 37, 66, 156, 253, 218, 85, 203,
			     110, 10, 244, 45, 34, 242, 72, 93, 52, 135, 44, 245, 3, 32, 196, 163,
			     232, 240, 227, 24, 139, 183, 38, 233, 130, 143, 109, 41, 174, 231, 129, 23,
			     148, 89, 212, 19, 21, 142, 7, 214, 56, 90, 11, 190, 175, 206, 26, 186};



void writeFile( struct Queue *,char * ) ;
void permutation(int plainData[16]);
unsigned char get_subbox_val (unsigned char );
struct Queue* createQueue();
void xorOperaton(int plain[16],int key[16]);
int isFull(struct Queue*);
int readFile(int *);
int isEmpty(struct Queue* queue);
int enqueue(struct Queue* queue, int * plainBlock);
int * dequeue(struct Queue* queue);
void authenticationOperation();
void subboxOperation();
void * readDataOperation(void *dummy);
void* xorThreadOperation() ;
void* permutationOperation() ;

int main() {

     pthread_t readDataThread;
     pthread_attr_t readData_attr;

    /*initialize the mutex lock and check if the initialization was successful */
     pthread_mutex_init(&lock, NULL);
     pthread_mutex_init(&lock2, NULL);
     pthread_mutex_init(&subBoxLock, NULL);
     pthread_mutex_init(&authenticationLock, NULL);


    /* initialize the full semaphore check if the initialization was successful*/
     sem_init(&full_semaphore, 0, 0);
     sem_init(&empty_semaphore, 0, 5);
     sem_init(&full_semaphore2, 0, 0);
     sem_init(&empty_semaphore2, 0, 5);
     sem_init(&full_subBox, 0, 0);
     sem_init(&empty_subBox, 0, 5);
     sem_init(&full_authentication, 0, 0);
     sem_init(&empty_authentication, 0, 5);

     pthread_attr_init(&readData_attr);

    /* created for the  threads*/
     pthread_create(&readDataThread, &readData_attr, readDataOperation, NULL);
    /* wait for the  threads to finish*/
     pthread_join(readDataThread, NULL);
    /* destroy the attributes for the producer thread and check if it was successful*/
     pthread_attr_destroy(&readData_attr);

    /* destroy the mutex and check if the destruction was successful*/
     pthread_mutex_destroy(&lock);
     pthread_mutex_destroy(&lock2);
     pthread_mutex_destroy(&subBoxLock);
     pthread_mutex_destroy(&authenticationLock);

     sem_destroy(&full_semaphore);
     sem_destroy(&empty_semaphore);
     sem_destroy(&full_semaphore2);
     sem_destroy(&empty_semaphore2);
     sem_destroy(&full_authentication);
     sem_destroy(&empty_authentication);
     sem_destroy(&full_subBox);
     sem_destroy(&empty_subBox);
}


void writeFile( struct Queue *a,char * str) {

    char file_name[20];				/* for file name */

    int i,j; 				/* for loop */

    /* the file is opened with the file name created here using the number of incoming process number */
    sprintf(file_name, "process<0> ");
     strcat(file_name, str);
    strcat(file_name, ".txt");
    FILE *output = fopen(file_name, "a ");
    for(i = 0; i < 16; i++) {
	    fprintf(output, "%d", a->array[a->rear][i]);	/* write matrix element to file */
        if(i!=15)
            fprintf(output, "-");


    }
  fprintf(output,"\n");
    fclose(output);					/* close file */

}

int readFile(int *plain)
{
    char *word;
    FILE *in = fopen("plain.txt", "r");	/* open for reading */
    const size_t line_size = 30000;
    char* line = malloc(line_size);
    int countIndex=0;
    int count=0;
         while (fgets(line, line_size, in) != NULL)  {

                    if(count==buffer)
                    {
                         word = strtok(line, "-");		/* split line by - */
                        while(word != NULL) {
                /* the words are converted to integer and then the plainBlock is assigned to the required position */
                        plain[countIndex]=atoi(word);
                        word = strtok(NULL, "-");
                        countIndex++;
                        }
                    }
                    count++;
         }
         if(buffer>=count)
            return -1;

         fclose(in);
         return 0;
}


void permutation(int plainData[16])
{
    int i=0;
    int *firstHalf = (int*) malloc(8 * sizeof(int));
    int *secondHalf = (int*) malloc(8 * sizeof(int));

    memcpy(firstHalf,plainData,8*sizeof(int));
    memcpy(secondHalf,&plainData[8],8*sizeof(int));
    memcpy(plainData,secondHalf,8*sizeof(int));

    for(i = 8; i < 16; i++)
    {
        plainData[i]=firstHalf[i-8];
    }

}


unsigned char get_subbox_val (unsigned char val)
{
  unsigned char sub_box_val = SUBBOX[val];

  return sub_box_val;
}

/* function to create a queue of given capacity.
 It initializes size of queue as 0 */
struct Queue* createQueue()
{
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
      queue->array=(int **) malloc(5*sizeof(int *));
    int i=0;
    for(i=0;i<16;i++)
        queue->array[i]=(int *) malloc(16*sizeof(int));

    queue->front = queue->size = 0;
    queue->rear = 4;
   return queue;
}
/*Queue is full when size becomes equal to the capacity*/
int isFull(struct Queue* queue)
{  if(queue->size==5)
    return -1;
    return 0;
}
/*Queue is empty when size is 0 */
int isEmpty(struct Queue* queue)
{
    if(queue->size==0)
    return -1;
     return 0;
}

/*Function to add an item to the queue.
 It changes rear and size */
int enqueue(struct Queue* queue, int * plainBlock)
{
    if (isFull(queue)==-1)
        return -1;
    queue->rear = (queue->rear + 1)%5;
    int i=0;
    for(;i<16;i++)
    {
          queue->array[queue->rear][i] = plainBlock[i];
    }
    queue->size = queue->size + 1;
    return 0;
}


/* Function to remove an item from queue.
 It changes front and size*/
int * dequeue(struct Queue* queue)
{
    if (isEmpty(queue)==-1)
        return -1;

    int * plain = (int *) malloc(sizeof(int)*16);
    int i=0;
    for(;i<16;i++)
    {
        plain[i] = queue->array[queue->front][i];
    }

    queue->front = (queue->front + 1)%5;
    queue->size = queue->size - 1;

    return plain;
}

void xorOperaton(int plain[16],int key[16])
{
    int i=0;
    for(i=0;i<16;i++)
    {
       plain[i]=plain[i]^key[i];
    }
}
/*first thread */
void * readDataOperation(void *dummy) {
      struct Queue*  plainQueue=createQueue();
    keyBlock=(int*) malloc (16 * sizeof(int));
    char *word;

    FILE *key = fopen("key.txt", "r");	/* open for reading */
    const size_t line_size = 30000;
    char* line = malloc(line_size);
    int count=0;
    while (fgets(line, line_size, key) != NULL)  {
                word = strtok(line, "-");		/* split line by comma */
                while(word != NULL) {
            /* the words are converted to integer the keyBlock is assigned to the required position */
                    keyBlock[count]=atoi(word);
                    word = strtok(NULL, "-");		/* deleted words */
                    count++;
            }
    }
    fclose(key);

    int index=0;
    do {
        int * plainBlock = (int*) malloc (16 * sizeof(int));
        if (plainBlock == NULL) {
            printf("Could not allocate memory for plainBlock\n");
            exit(EXIT_FAILURE);
        }
        index=readFile(plainBlock);
        buffer++;
        /* decrement the empty semaphore */
        sem_wait(&empty_semaphore);
        /* acquire the lock*/
        pthread_mutex_lock(&lock);

        if(index !=-1 )
            enqueue(plainQueue,plainBlock);
        /* release the lock */
        pthread_mutex_unlock(&lock);
        /* increment the full semaphore */
        sem_post(&full_semaphore);


    } while (plainQueue->size < 5 && index!=-1 );

     pthread_t  xorThread;
     pthread_attr_t  xor_attr;
     pthread_attr_init(&xor_attr);

     /* created for the  threads*/
     pthread_create(&xorThread, &xor_attr, xorThreadOperation, (void*) plainQueue);

      /* wait for the  threads to finish*/
     pthread_join(xorThread, NULL);

    /* destroy the attributes for the producer thread and check if it was successful*/
     pthread_attr_destroy(&xor_attr);

    return NULL;
}

/*second thread */
void* xorThreadOperation(void * argumentsQueue) {
    struct Queue *arguments = (struct Queue*)argumentsQueue;
    struct Queue* permutationQueue=createQueue();
    do {
        /* decrement the full semaphore*/
        sem_wait(&full_semaphore);

        /* acquire the lock*/
        pthread_mutex_lock(&lock);

        int * data=dequeue(arguments);

        xorOperaton(data,keyBlock);
        sem_wait(&empty_semaphore2);
       /* acquire the lock*/
        pthread_mutex_lock(&lock2);

        enqueue(permutationQueue,data);

        pthread_mutex_unlock(&lock2);
        /* increment the full semaphore */
        sem_post(&full_semaphore2);

        /* release the lock*/
        pthread_mutex_unlock(&lock);
        /* increment the empty semaphore*/
        sem_post(&empty_semaphore);

        writeFile(permutationQueue,"thread<2>");

    } while (arguments->size>0);

     pthread_t permutationThread;
     pthread_attr_t permutation_attr;
     pthread_attr_init(&permutation_attr);

     pthread_create(&permutationThread, &permutation_attr, permutationOperation, (void*) permutationQueue);
      /* wait for the  threads to finish*/
     pthread_join(permutationThread, NULL);

     pthread_attr_destroy(&permutation_attr);

}

 /*third thread */
void* permutationOperation(void * argumentsQueue) {

    struct Queue *arguments = (struct Queue*)argumentsQueue;
     struct Queue *subBoxQueue=createQueue();
    do {
        /* decrement the full semaphore*/
        sem_wait(&full_semaphore2);
        /* acquire the lock*/
        pthread_mutex_lock(&lock2);
        int * data=dequeue(arguments);
        permutation(data);

         sem_wait(&empty_subBox);
       /* acquire the subBoxLock*/
        pthread_mutex_lock(&subBoxLock);

        enqueue(subBoxQueue,data);
        /*release the subBoxLock*/
        pthread_mutex_unlock(&subBoxLock);

       /* increment the empty semaphore*/
        sem_post(&full_subBox);

        pthread_mutex_unlock(&lock2);
        /* increment the empty semaphore*/
        sem_post(&empty_semaphore2);

        writeFile(subBoxQueue,"thread<3>");

    } while (arguments->size>0);

    pthread_t subBoxThread;
    pthread_attr_t subBox_attr;
    pthread_attr_init(&subBox_attr);

    pthread_create(&subBoxThread, &subBox_attr, subboxOperation,(void*) subBoxQueue);
      /* wait for the  threads to finish*/
    pthread_join(subBoxThread, NULL);

    pthread_attr_destroy(&subBox_attr);

}
/*fourth thread */
void subboxOperation(void *argumentsQueue )
{
    struct Queue * arguments = (struct Queue*)argumentsQueue;
     struct Queue * authenticationQueue=createQueue();
    do {

        sem_wait(&full_subBox);

        int j=0;
        int * data=dequeue(arguments);

        for(; j< 16; j++ ) {
            unsigned char rand_ind = data[j] % 256;
            data[j]=get_subbox_val(rand_ind);

        }
        /* acquire the subBoxLock*/
        pthread_mutex_lock(&subBoxLock);

        sem_wait(&empty_authentication);
        pthread_mutex_lock(&authenticationLock);

        enqueue(authenticationQueue,data);

        pthread_mutex_unlock(&authenticationLock);
       /* increment the efull_authentication*/
        sem_post(&full_authentication);

        /* release the subBoxLock*/
        pthread_mutex_unlock(&subBoxLock);
        /* increment the empty semaphore*/
        sem_post(&empty_subBox);

        writeFile(authenticationQueue,"thread<4>");

    } while (arguments->size>0);

      pthread_t authenticationThread;
      pthread_attr_t authentication_attr;

      pthread_create(&authenticationThread, &authentication_attr, authenticationOperation, (void*) authenticationQueue);
      /* wait for the  threads to finish*/
      pthread_join(authenticationThread, NULL);

      pthread_attr_destroy(&authentication_attr);

}

/*fifth thread */
void authenticationOperation(void *argumentsQueue)
{

   struct Queue *arguments = (struct Queue*)argumentsQueue;

    do {
        /* decrement the full semaphore*/
        sem_wait(&full_authentication);

        /* acquire the lock*/
        pthread_mutex_lock(&authenticationLock);

         int * data=dequeue(arguments);

        /* release the lock*/
        pthread_mutex_unlock(&authenticationLock);
        /* increment the empty semaphore*/
        sem_post(&empty_authentication);

    } while (arguments->size>0);


}






