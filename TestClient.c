// IRC Client Code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MESSAGE 200

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;

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

        // Receive and display messages from the server
        recv(client_socket, message, sizeof(message), 0);
        printf("Server: %s\n", message);
    }

    // Close the connection
    close(client_socket);

    return 0;
}
