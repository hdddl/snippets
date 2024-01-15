#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CONNECTIONS 5

void handle_client(int client_socket) {
    char buffer[1024];
    ssize_t bytes_received;

    // Receive data from the client
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        // Process received data (in this example, just print it)
        buffer[bytes_received] = '\0';
        printf("Received from client: %s", buffer);

        // Echo back to the client
        send(client_socket, buffer, bytes_received, 0);
    }

    // Close the socket when done
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pid_t child_pid;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CONNECTIONS) == -1) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept a client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Accepting client failed");
            continue;
        }

        // Fork a new process to handle the client
        child_pid = fork();

        if (child_pid == 0) {
            // In child process
            close(server_socket);  // Child process doesn't need the server socket
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        } else if (child_pid > 0) {
            // In parent process
            close(client_socket);  // Parent process doesn't need the client socket
        } else {
            perror("Forking failed");
            exit(EXIT_FAILURE);
        }
    }

    // Close the server socket (this part will not be reached in this example)
    close(server_socket);

    return 0;
}