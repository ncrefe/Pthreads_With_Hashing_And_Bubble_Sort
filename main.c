#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct node {
    struct node *r_node;
    unsigned value;
} node;

typedef struct hash_table {
    node **list;
    unsigned nof_element;
} hash_table;

typedef struct parameterPass {
    unsigned *numbers;
    unsigned offset;
    unsigned last_ele;
    hash_table *hashtable;
    pthread_mutex_t *mutexes;
    unsigned numOfThreads;
} parameterPass;

unsigned countNumOfElements(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 0;
    }

    unsigned count = 0;
    char line[256];
    //skip the title line
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file) != NULL) {
        count++;
    }

    fclose(file);
    return count;
}

unsigned *readNumbers(char *filename, unsigned num_element) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }

    //skip the title line
    char line[256];
    fgets(line, sizeof(line), file);

    //allocate memory for the array
    unsigned *numbers = (unsigned *) malloc(num_element * sizeof(unsigned));
    if (numbers == NULL) {
        fclose(file);
        printf("Error allocating memory.\n");
        return NULL;
    }

    //read numbers from the file to write into the array
    unsigned index = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        unsigned number;
        sscanf(line, "%*d,%u", &number);
        numbers[index] = number;
        index++;
    }

    fclose(file);
    return numbers;
}

hash_table *initializeHashTable(unsigned numOfThread, unsigned numOfElements) {
    hash_table *hashtable = (hash_table *) malloc(sizeof(hash_table));
    if (hashtable == NULL) {
        printf("Error allocating memory for hash table.\n");
        return NULL;
    }

    unsigned outlierSize = (numOfThread * (numOfThread + 1)) / 2;

    hashtable->list = (node **) malloc(outlierSize * sizeof(node *));
    if (hashtable->list == NULL) {
        free(hashtable);
        printf("Error allocating memory for list.\n");
        return NULL;
    }

    for (unsigned i = 0; i < outlierSize; i++) {
        hashtable->list[i] = NULL;
    }

    hashtable->nof_element = numOfElements;
    return hashtable;
}

void swap(node *a, node *b) {
    unsigned temp = a->value;
    a->value = b->value;
    b->value = temp;
}

void bubbleSort(node *start) {
    int swapped;
    node *ptr1;
    node *lptr = NULL;

    if (start == NULL)
        return;

    do {
        swapped = 0;
        ptr1 = start;

        while (ptr1->r_node != lptr) {
            if (ptr1->value > ptr1->r_node->value) {
                swap(ptr1, ptr1->r_node);
                swapped = 1;
            }
            ptr1 = ptr1->r_node;
        }
        lptr = ptr1;
    } while (swapped);
}

void *insertionFunction(void *parameters) {
    parameterPass *params = (parameterPass *) parameters;
    unsigned *numbers = params->numbers;
    unsigned offset = params->offset;
    unsigned last_ele = params->last_ele;
    hash_table *hashtable = params->hashtable;
    pthread_mutex_t *mutexes = params->mutexes;
    unsigned numOfThreads = params->numOfThreads;

    for (unsigned i = offset; i < last_ele; i++) {
        unsigned value = numbers[i];
        unsigned index = value % ((numOfThreads * (numOfThreads + 1)) / 2); //(thread * (thread + 1)) / 2.

        pthread_mutex_lock(&mutexes[index]);

        node *new_node = (node *) malloc(sizeof(node));
        if (new_node == NULL) {
            printf("Error allocating memory for new node.\n");
            pthread_mutex_unlock(&mutexes[index]);
            return NULL;
        }

        new_node->value = value;
        new_node->r_node = hashtable->list[index];
        hashtable->list[index] = new_node;

        pthread_mutex_unlock(&mutexes[index]);
    }

    return NULL;
}

//wrapper method to parallelize the bubble sort
void *sortingFunction(void *parameters) {
    parameterPass *params = (parameterPass *) parameters;
    unsigned index = params->offset;

    bubbleSort(params->hashtable->list[index]);

    return NULL;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./c_output_file csv_name thread_number\n");
        return 1;
    }
    
    char* filename = argv[1]; //get the file name from user argument
    unsigned count = countNumOfElements(filename);
    printf("Number of elements: %u\n", count);

    unsigned *numbers = readNumbers(filename, count);

    if (numbers == NULL) {
        return 1;
    }

    unsigned numOfThread = atoi(argv[2]); //get the number of threads from user argument

    hash_table *hashtable = initializeHashTable(numOfThread, count);
    if (hashtable == NULL) {
        free(numbers);
        return 1;
    }

    pthread_t *threads = (pthread_t *) malloc(numOfThread * sizeof(pthread_t));
    if (threads == NULL) {
        free(numbers);
        free(hashtable->list);
        free(hashtable);
        printf("Error allocating memory for threads.\n");
        return 1;
    }

    pthread_mutex_t *mutexes = (pthread_mutex_t *) malloc((numOfThread * (numOfThread + 1)) / 2 * sizeof(pthread_mutex_t));
    if (mutexes == NULL) {
        free(numbers);
        free(hashtable->list);
        free(hashtable);
        free(threads);
        printf("Error allocating memory for mutexes.\n");
        return 1;
    }

    //initialize mutexes
    for (unsigned i = 0; i < (numOfThread * (numOfThread + 1)) / 2; i++) {
        pthread_mutex_init(&mutexes[i], NULL);
    }

    parameterPass *params = (parameterPass *) malloc(numOfThread * sizeof(parameterPass));
    if (params == NULL) {
        free(numbers);
        free(hashtable->list);
        free(hashtable);
        free(threads);
        free(mutexes);
        printf("Error allocating memory for params.\n");
        return 1;
    }

    unsigned interval = count / numOfThread + 1; //interval = nof_element / nofThreas + 1 => count is used as nof_element
    unsigned offset = 0;
    //offset = interval * tid by loop
    for (unsigned i = 0; i < numOfThread; i++) {
        params[i].numbers = numbers;
        params[i].offset = offset; // csv start
        params[i].last_ele = offset + interval; // csv end
        if (params[i].last_ele > count) {
            params[i].last_ele = count;
        }
        params[i].hashtable = hashtable;
        params[i].mutexes = mutexes;
        params[i].numOfThreads = numOfThread;

        pthread_create(&threads[i], NULL, insertionFunction, (void *) &params[i]);
        offset += interval;
    }

    //wait for threads to complete
    for (unsigned i = 0; i < numOfThread; i++) {
        pthread_join(threads[i], NULL);
    }

    parameterPass *paramsBubbleSort = (parameterPass *) malloc((numOfThread * (numOfThread + 1)) / 2 * sizeof(parameterPass));
    if (paramsBubbleSort == NULL) {
        free(numbers);
        free(hashtable->list);
        free(hashtable);
        free(threads);
        free(mutexes);
        free(params);
        printf("Error allocating memory for params.\n");
        return 1;
    }
    pthread_t *threadsBubbleSort = (pthread_t *) malloc((numOfThread * (numOfThread + 1)) / 2 * sizeof(pthread_t));
    if (threads == NULL) {
        free(paramsBubbleSort);
        free(numbers);
        free(hashtable->list);
        free(hashtable);
        free(threads);
        free(mutexes);
        free(params);
        printf("Error allocating memory for params.\n");
        return 1;
    }

    //sort each linked list
    for (unsigned i = 0; i < (numOfThread * (numOfThread + 1)) / 2; i++) {
        paramsBubbleSort[i].offset = i; // Set the offset as the index of the linked list
        paramsBubbleSort[i].hashtable = hashtable;
        pthread_create(&threadsBubbleSort[i], NULL, sortingFunction, (void *) &paramsBubbleSort[i]);
    }

    //wait for threads to complete
    for (unsigned i = 0; i < numOfThread; i++) {
        pthread_join(threadsBubbleSort[i], NULL);
    }

    //free the memory allocations
    free(numbers);
    free(hashtable->list);
    free(hashtable);
    free(threads);
    free(mutexes);
    free(params);
    free(paramsBubbleSort);
    free(threadsBubbleSort);

    return 0;
}

