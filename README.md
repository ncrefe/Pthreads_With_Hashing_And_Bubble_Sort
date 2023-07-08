# Combined Hashing and Bubble-Sort Algorithms
This repository contains an implementation of combined hashing and bubble-sort algorithms using Pthreads. The goal is to sort a given set of random numbers according to specific rules. The code utilizes additional data structures, including linked lists and hash tables, to facilitate the sorting process.

## Data Structures
The code utilizes the following data structures:
### Linked List Node Structure
```
typedef struct node {
    struct node* r_node;
    unsigned value;
} node;
```
This structure represents a node in a linked list. It contains a pointer to the next node (r_node) and holds a numeric value.

### Hash Table Structure
```
typedef struct hash_table {
    node** list;
    unsigned nof_element;
} hash_table;
```
This structure represents the main hash table. It consists of a double-linked list (list) used for indexing corresponding linked lists. The nof_element field holds the total number of stored elements.
### Parameter Pass Structure
```
typedef struct parameterPass {
    // Define specialized structure for passing multiple sub-structures to a thread
} parameterPass;
```
This structure is used to pass multiple sub-structures to a thread during its creation. Its design can be customized as needed.

## Code Flow
The code follows the following flow:

1. Reading Numbers from CSV File:
+ The countNumOfElements function is used to count the number of elements in the given CSV file.
+ The readNumbers function dynamically allocates an array and reads the numbers from the CSV file into the array.
2. Initializing the Hash Table:
+ The hash table is initialized using the initializeHashTable function. It sets up a 2D linked list structure based on the number of threads and number of elements.
3. Inserting Elements into the Hash Table:
+ The insertionFunction is called for each thread. Each thread indexes a specific region of the array, calculates the modular values of the numbers within that region, and adds them to the relevant location in the hash table. Pthread mutex functions are used to avoid race conditions.
4. Sorting Elements in the Hash Table:
+ The bubble sort algorithm is used to sort the numbers within each 1D linked list in the hash table. Multiple threads are created to handle the sorting process, with each thread responsible for sorting a specific linked list.
5. Memory Management:
+ Proper memory management is crucial. Make sure to release any unused memory portions when finalizing your code.


