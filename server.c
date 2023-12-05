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
#include <time.h>

#define MAX_CLIENTS 10 // the maximum number of clients that can connect at once is 10
#define MAX_MESSAGE 2000 // character cap for message length is 2000 characters
#define MAX_NAME_LENGTH 20 // character cap for username is 20

//
// client_t struct includes the client's socket address, file descriptor,
// user ID, and name. Name length is defined here.
//
typedef struct {

    struct sockaddr_in address;
    int socket_fd;
    int user_id;
    char name[MAX_NAME_LENGTH];

} client_t;

FILE *file = fopen(CHAT_FILE, "chat_log.txt");
fclose(file);

client_t *clients[MAX_CLIENTS]; // MAX_CLIENT pointers
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // initialize mutual exclusion lock
int user_id = 0; // initialize user_id to 0

//
// lock_clients function attempts to lock the mutex. if the mutex is not locked by
// another thread, calling thread with acquire lock. if mutex is locke dbya nother thread,
// calling thread will be blocked.
//
void lock_clients() {

    pthread_mutex_lock(&clients_mutex);

}

//
// unlock_clients function attempts to unlock the mutex.
//
void unlock_clients() {

    pthread_mutex_unlock(&clients_mutex);

}

//
// add_client function takes parameters client_t and the client pointers
// and checks the total client limit to ensure that no addition clients
// exceed MAX_CLIENTS.
//
void add_client(client_t *client) {

    lock_clients();

    for(int i = 0; i < MAX_CLIENTS; i++) {

        if(!clients[i]) {
            clients[i] = client;
            break;
        }

    }

    unlock_clients();

}

//
// remove_client function take the user ID as a function and frees the client upon disconnect.
//
void remove_client(int uid) {

    lock_clients();

    for(int i = 0; i < MAX_CLIENTS; i++) {

        if(clients[i] && clients[i]->user_id == uid) {

            free(clients[i]);
            clients[i] = NULL;
            break;

        }

    }

    unlock_clients();

}

//
// send_message function takes the message and user ID as parameters and sends the message to clients.
// gets current time and formats it into a timestamp. opens chat log file in append mode.
//
void send_message(char *msg, int uid) {

    lock_clients();
    char buf[MAX_MESSAGE];
    time_t now = time(NULL);

    for (int i = 0; i < MAX_CLIENTS; i++) {

        if (clients[i] && clients[i]->user_id != uid) {

            snprintf(buf, sizeof(buf), "%s - %s\n", clients[uid]->name, msg);
            send(clients[i]->socket_fd, buf, strlen(buf), 0);

        }

    }

    unlock_clients();

}