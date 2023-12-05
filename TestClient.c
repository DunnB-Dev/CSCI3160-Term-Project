// IRC Client Code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_MESSAGE 200

int client_socket;

// Function to receive and display messages from the server
void* receive_messages(void* arg) {
    char message[MAX_MESSAGE];
    int read_size;

    while ((read_size = recv(client_socket, message, sizeof(message), 0)) > 0) {
        message[read_size] = '\0'; // Null-terminate the received message
        printf("Server: %s\n", message);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    pthread_t receive_thread;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%s\n", argv[1], argv[2]);

    // Create a thread to receive and display messages from the server
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
        perror("Error creating receive thread");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char message[MAX_MESSAGE];

    // Accept user input and send messages to the server
    while (1) {
        printf("Enter message (type 'quit' to exit): ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0'; // Remove trailing newline

        // Send the message to the server
        send(client_socket, message, strlen(message), 0);

        // Check if the user wants to quit
        if (strcmp(message, "quit") == 0) {
            break;
        }
    }

    // Close the connection
    close(client_socket);

    // Wait for the receive thread to finish
    pthread_join(receive_thread, NULL);

    return 0;
}
