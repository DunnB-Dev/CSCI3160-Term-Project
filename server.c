//
// Date: 12/02/2023
// Authors: Brycen Dunn
// Purpose: Handle server logic for IRC-like Application
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

#define MAX_CLIENTS 5
#define MAX_MESSAGE 200
#define MAX_NAME_LENGTH 10

static int client_count = 0; // Number of clients connected
static int user_id = 5; 

// Client struct to hold client information and link to next client
struct client_struct{

    struct sockaddr_in address;
    int socket_file_descriptor; // File descriptor for client socket
    int user_id;
    char name[MAX_NAME_LENGTH];
    struct client_struct *next; // Pointer to next client

};

// Initialize mutex lock and client list
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;  // Initialize mutex lock
struct client_struct *clients = NULL; // Initialize client list

// Function to add client to client list and lock mutex 
void enqueue_client(struct client_struct *client){

    pthread_mutex_lock(&lock); 
    client->next = clients; // Set next client to current client
    clients = client; // Set current client to head of list
    pthread_mutex_unlock(&lock);

}

// Function to remove client from client list and unlock mutex
void dequeue_client(int user_id){

    pthread_mutex_lock(&lock);
    struct client_struct *client = clients; // Set client to head of list
    struct client_struct *prev = NULL; // Set previous client to NULL

    while (client) { // Loop through client list

        if (client->user_id == user_id) { // If client is found

            if (prev) { // and client is not head of list

                prev->next = client->next; // Set previous client's next to current client's next

            } else { // If client is head of list

                clients = client->next; // Set head of list to next client

            }

            free(client);
            break;

        }

        prev = client; // Set previous client to current client
        client = client->next; // Set current client to next client

    }

    pthread_mutex_unlock(&lock);

}

// Function to send message to all clients except the client who sent the message
void send_message(char *message, int user_id){

    pthread_mutex_lock(&lock); // Lock mutex
    struct client_struct *client = clients; // Set client to head of list

    while (client) { // Loop through client list
 
        if (client->user_id != user_id) { // If client is not the client who sent the message

            write(client->socket_file_descriptor, message, strlen(message)); // Write message to client

        }

        client = client->next; // Set current client to next client

    }

    pthread_mutex_unlock(&lock);

}
