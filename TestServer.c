// IRC Server Code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define MAX_MESSAGE 200

// Client struct to hold client information
struct client_struct {
    int socket_file_descriptor;
    struct sockaddr_in address;
};

// Function declarations
void* handle_client(void* arg);
void broadcast_message(char* message, int sender_socket);

// Global variables
struct client_struct clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %s...\n", argv[1]);

    // Accept and handle incoming connections
    while (1) {
        socklen_t client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket == -1) {
            perror("Error accepting connection");
            continue;
        }

        // Add client to the array
        pthread_mutex_lock(&client_mutex);
        clients[client_count].socket_file_descriptor = client_socket;
        memcpy(&clients[client_count].address, &client_addr, sizeof(client_addr));
        client_count++;
        pthread_mutex_unlock(&client_mutex);

        // Create a new thread to handle the client
        if (pthread_create(&tid, NULL, handle_client, (void*)&client_socket) != 0) {
            perror("Error creating thread");
            close(client_socket);
            continue;
        }
    }

    close(server_socket);
    return 0;
}

void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    char buffer[MAX_MESSAGE];
    int read_size;

    while ((read_size = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0'; // Null-terminate the received message

        // Broadcast the message to all clients
        broadcast_message(buffer, client_socket);

        // Check if the client wants to quit
        if (strcmp(buffer, "quit") == 0) {
            break;
        }
    }

    // Client disconnected or quit
    pthread_mutex_lock(&client_mutex);

    // Find and remove the disconnected client
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket_file_descriptor == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }

    pthread_mutex_unlock(&client_mutex);

    // Close the socket and exit the thread
    close(client_socket);
    pthread_exit(NULL);
}

void broadcast_message(char* message, int sender_socket) {
    pthread_mutex_lock(&client_mutex);

    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket_file_descriptor != sender_socket) {
            send(clients[i].socket_file_descriptor, message, strlen(message), 0);
        }
    }

    pthread_mutex_unlock(&client_mutex);
}
