// include libraries
#include <stdio.h>
#include <stdlib.h>

// defining all the constraints
#define MAX_NO_OF_GROUPS 500000
#define MAX_NO_OF_GROUP_SIZE 1000000
#define MAX_ARRIVAL_TIME 1000000000
#define MAX_PROCESSING_TIME 1000000

// Some global variables
long long int leftQueueSize = 0;
long long int rightQueueSize = 0;
long long int leftLastExitTime = 0;
long long int rightLastExitTime = 0;

// defining all the structs of the program
typedef struct Node Node;
typedef struct Queue Queue;
typedef struct Group Group;

// Group struct for storing the data of each group
struct Group {
    long long int arrival;
    long long int size;
    long long int process;
    long long int exitTime;
};

// Node struct for storing the data of each node
struct Node {
    Group data;
    Node * next;
};

// Queue struct for storing the data of each queue
struct Queue {
    Node * head;
    Node * tail;
};

// DISCLAMER: The following functions of Queue and Circular Linked List are not written by me, they are taken as a reference from the lecturer of the course.
// Files where the functions are taken from: Queue.c

// Queue functions prototypes 
Queue * createQueue();
void enqueue(Queue * queue, Group data);
void dequeue(Queue * queue, Group data);

// Circular Linked List functions prototypes
Node * createNode(Group data);
Node * addTail(Node * tail, Group data);
Node * removeHead(Node * tail, Group data);

// Queue functions

// Remove the front of the queue
void dequeue(Queue * q, Group data) {
    q->tail = removeHead(q->tail, data);
}

// Add to the back of the queue
void enqueue(Queue * q, Group data) {
    q->tail = addTail(q->tail, data);
}

// Create the Queue dynamically
Queue * createQueue() {
    Queue * result = (Queue *) malloc(sizeof(Queue));
    result->tail = NULL; // the queue is empty
    return result;
}

// Circular Linked List functions

// Remove the head node of a circular linked list and return the resulting tail
Node * removeHead(Node * tail, Group data) {
    // Corner Case: Empty List
    if (tail == NULL) {
        return NULL;
    }
    
    // Corner Case: 1 node
    if (tail->next == tail) {
        tail->data.arrival = 0;
        tail->data.size = 0;
        tail->data.process = 0;
        free(tail); // head and tail are the same
        return NULL; // list is now empty
    }

    Node * oldTail = tail;
    Node * oldHead = tail->next;
    Node * newHead = oldHead->next;

    // Update pointers
    oldTail->next = newHead;

    // Free old Memory
    free(oldHead);

    // Return the tail of the list
    return oldTail;
}

// Add a group to the tail of a circular linked list and returns the resulting tail
Node * addTail(Node * tail, Group data) {
    if (tail == NULL) {
        // create a node that is the head and tail of the list
        return createNode(data);
    }

    // The list was not empty
    // Get the pointers for modification
    Node * oldTail = tail;
    Node * oldHead = tail->next;
    Node * newTail = createNode(data);
    
    // Update pointers
    oldTail->next = newTail;
    newTail->next = oldHead;

    // Return the resulting tail
    return newTail;
}

// Create a node with a new group as data
Node * createNode(Group data) {
    Node * result = (Node *) malloc(sizeof(Node));
    result->data = data;
    result->next = result;
    return result;
}

// DISCLAMER: From here the functions are written by me and with my best knowledge

// Heapify the heap
void heapify(Group * groups, int n, int i) {

    // Find the largest among root, left child and right child
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    // Find largerst arrival time of all the groups from left side
    if (left < n && groups[left].arrival > groups[largest].arrival) {
        largest = left;
    }

    // Find largerst arrival time of all the groups from right side
    if (right < n && groups[right].arrival > groups[largest].arrival) {
        largest = right;
    }

    // Swap and continue heapifying if root is not largest
    if (largest != i) {
        Group temp = groups[i];
        groups[i] = groups[largest];
        groups[largest] = temp;

        heapify(groups, n, largest);
    }
}

// Function to sort the groups by arrival time using heap sort
void heapSort(Group * groups, int n) {

    // Build max heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(groups, n, i);
    }

    for (int i = n - 1; i >= 0; i--) {

        // Swap the root with the 'i' node
        Group temp = groups[0];
        groups[0] = groups[i];
        groups[i] = temp;

        // Heapify the root group data
        heapify(groups, i, 0);
    }
}

// Function to remove the group from the queue who are already processed and update the queue size
void removeProcessedNodes(long long int currentTime, Queue * leftQueue, Queue * rightQueue) {

    // If the left queue is empty and the right queue is empty then return to the main function without any changes
    if (leftQueue->tail == NULL && rightQueue->tail == NULL) {
        return;
    } else if (leftQueue->tail == NULL) { // If the left queue is empty then check the right queue

        // Using Node* temp to store the right queue tail
        Node * temp = rightQueue->tail;

        // Looping through the right queue to find the group who are already processed and remove them from the queue
        do {

            // If the current group's exit time is less than current then remove it from the queue
            if (temp->data.exitTime < currentTime) {
                rightQueueSize -= temp->data.size; // Update the right queue size
                dequeue(rightQueue, temp->data); // Remove the group from the right queue
            }

            // If right queue tail is empty now then break the loop
            if (rightQueue->tail == NULL) {
                break;
            }

            // Check the next group in the right queue
            temp = temp->next;

        } while (temp != rightQueue->tail); // Looping until we reach the right queue tail

    } else if (rightQueue->tail == NULL) { // If the right queue is empty then check the left queue

        // Using Node* temp to store the left queue tail
        Node * temp = leftQueue->tail;

        // Looping through the left queue to find the group who are already processed and remove them from the queue
        do {

            // If the current group's exit time is less than current then remove it from the queue
            if (temp->data.exitTime < currentTime) {
                leftQueueSize -= temp->data.size; // Update the left queue size
                dequeue(leftQueue, temp->data); // Remove the group from the left queue
            }

            // If left queue tail is empty now then break the loop
            if (leftQueue->tail == NULL) {
                break;
            }
            
            // Check the next group in the left queue
            temp = temp->next;
        } while (temp != leftQueue->tail); // Looping until we reach the left queue tail

    } else { // If both the queues are not empty then check both the queues

        // Declaring int variable to store the complement value as 1 to use it later and also to restrict do-while loop to less number of times
        int complement = 1;

        // Using Node* temp to store the left queue tail
        Node * temp = leftQueue->tail;

        // Looping through the left queue to find the group who are already processed and remove them from the queue
        do {
            // If the current group's exit time is less than current then remove it from the queue
            if (temp->data.exitTime < currentTime) {
                leftQueueSize -= temp->data.size; // Update the left queue size
                dequeue(leftQueue, temp->data); // Remove the group from the left queue
            }

            // If the loop has not run twice and current group's exit time is less than current time then make changes
            if(complement < 2 && temp->data.exitTime < currentTime) {
                leftQueueSize -= temp->data.size; // Update the left queue size
                dequeue(leftQueue, temp->data); // Remove the group from the left queue
            }

            // If the loop has run twice and current group's exit time is more than current time then break the loop
            // Because all the groups after this group will have exit time more than current time
            if(temp->data.exitTime > currentTime && complement > 2) {
                break;
            }

            // If left queue tail is empty now then break the loop
            if (leftQueue->tail == NULL) {
                leftQueueSize = 0; // Update the left queue size
                break;
            }

            // Check the next group in the left queue
            temp = temp->next;

            // Increment the complement value by 1 to check the loop has run twice or not
            complement++;
        } while (temp != leftQueue->tail); // Looping until we reach the left queue tail

        // Changing the complement value back to 1 to use it for right queue now
        complement = 1;

        // Using Node* temp to store the right queue tail
        temp = rightQueue->tail;

        // Looping through the right queue to find the group who are already processed and remove them from the queue
        do {

            // If the current group's exit time is less than current then remove it from the queue
            if (temp->data.exitTime < currentTime) {
                rightQueueSize -= temp->data.size; // Update the right queue size
                dequeue(rightQueue, temp->data); // Remove the group from the right queue
            }

            // If the loop has not run twice and current group's exit time is less than current time then make changes
            if(complement < 2 && temp->data.exitTime < currentTime) {
                rightQueueSize -= temp->data.size; // Update the right queue size
                dequeue(rightQueue, temp->data); // Remove the group from the right queue
            }

            // If the loop has run twice and current group's exit time is more than current time then break the loop
            // Because all the groups after this group will have exit time more than current time
            if(temp->data.exitTime > currentTime && complement > 2) {
                break;
            }

            // If right queue tail is empty now then break the loop
            if (rightQueue->tail == NULL) {
                rightQueueSize = 0; // Update the right queue size
                break;
            }

            // Check the next group in the right queue
            temp = temp->next;

            // Increment the complement value by 1 to check the loop has run twice or not
            complement++;
        } while (temp != rightQueue->tail);
    }
}

// Main function
int main() {

    // Declaring the variable to store the number of groups in total to process
    long long int NumberOfGroups = 0;

    // Taking the input from the user
    scanf("%lld", &NumberOfGroups);

    // Using while loop to check the input is valid or not and if not then ask the user to enter the input again
    while(NumberOfGroups < 1 || NumberOfGroups > MAX_NO_OF_GROUPS) {
        scanf("%lld", &NumberOfGroups); // Taking the input from the user again
    }

    // Allocating memory in heap for all the groups using calloc function 
    Group * groups = (Group *) calloc(NumberOfGroups, sizeof(Group)); 

    // Using for loop to take the input from the user of every group's size, arrival time and processing time
    for(int i = 0; i < NumberOfGroups; i++) { 

        // Read the data from the input
        scanf("%lld %lld %lld", &groups[i].size, &groups[i].arrival, &groups[i].process);

        // Using while loop to check the input is valid or not and if not then ask the user to enter the input again
        while((groups[i].size < 1 || groups[i].size > MAX_NO_OF_GROUP_SIZE) || (groups[i].arrival < 1 || groups[i].arrival > MAX_ARRIVAL_TIME) || (groups[i].process < 1 || groups[i].process > MAX_PROCESSING_TIME)) {
            scanf("%lld %lld %lld", &groups[i].size, &groups[i].arrival, &groups[i].process); // Taking the input from the user again
        }

    }

    // Sorting all the groups using heapSort function in ascending order of arrival time
    heapSort(groups, NumberOfGroups); 

    // Declaring the mainLine which will contain all the groups in ascending order of arrival time and as a circular linked list
    Node * mainLine = NULL;

    // Using for loop to add all the groups in the mainLine as a circular linked list
    for(int i = 0; i < NumberOfGroups; i++) {
        mainLine = addTail(mainLine, groups[i]); // Adding the group in the mainLine
    }

    // Declaring the left queue which will contain all the groups who are waiting to be processed in the left ticket counter
    Queue * leftQueue = createQueue();
    // Declaring the right queue which will contain all the groups who are waiting to be processed in the right ticket counter
    Queue * rightQueue = createQueue();

    // Declaring the variable to store the total processing time of all the groups
    long long int TotalProcessingTime = 0;

    // Using node* temp to store the mainLine tail and using it to traverse the mainLine
    Node * temp = mainLine->next;

    // Using while loop to traverse the mainLine
    do {
        // This function will check if any group has already been processed and remove them from the left and right queue
        removeProcessedNodes(temp->data.arrival, leftQueue, rightQueue);

        // If the left queue is empty then add the group in the left queue
        if(leftQueue->tail == NULL) {
            temp->data.exitTime = temp->data.arrival + temp->data.process; // Calculate the exit time of the group

            enqueue(leftQueue, temp->data); // Add the group in the left queue
            leftQueueSize += temp->data.size; // Update the left queue size

            leftLastExitTime = temp->data.exitTime; // Update the left last exit time

        } else if(rightQueue->tail == NULL) { // If the right queue is empty then add the group in the right queue

            temp->data.exitTime = temp->data.arrival + temp->data.process; // Calculate the exit time of the group

            enqueue(rightQueue, temp->data); // Add the group in the right queue
            rightQueueSize += temp->data.size; // Update the right queue size

            rightLastExitTime = temp->data.exitTime; // Update the right last exit time

        } else { // If both the queues are not empty then check which queue has less number of people and add the group in that queue

            // If the left queue has less number of people or equal to the rightQueueSize then add the group in the left queue
            if(leftQueueSize <= rightQueueSize) {
                temp->data.exitTime = leftLastExitTime + temp->data.process; // Calculate the exit time of the group

                enqueue(leftQueue, temp->data); // Add the group in the left queue
                leftQueueSize += temp->data.size; // Update the left queue size

                leftLastExitTime = temp->data.exitTime; // Update the left last exit time

            } else { // If the right queue has less number of people then add the group in the right queue

                temp->data.exitTime = rightLastExitTime + temp->data.process; // Calculate the exit time of the group

                enqueue(rightQueue, temp->data); // Add the group in the right queue
                rightQueueSize += temp->data.size; // Update the right queue size

                rightLastExitTime = temp->data.exitTime; // Update the right last exit time
            }
        }

        // Update the total processing time
        TotalProcessingTime += (temp->data.exitTime - temp->data.arrival) * temp->data.size; 

        // Move to the next group in the mainLine
        temp = temp->next;
        
    } while (temp != mainLine->next); // Run the loop till we reach the mainLine tail again

    // Print the total processing time
    printf("%lld\n", TotalProcessingTime);

    // Free all the memory allocated in heap
    free(leftQueue);
    free(rightQueue);
    free(groups);
    free(mainLine);

    return 0;
}