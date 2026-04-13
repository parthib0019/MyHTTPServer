#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "include/myserver.h"

int main(){
    Connection connection_pool[1024];
    init_connection_pool(connection_pool, 1024);
    pthread_t thread_pool[4];
    for(size_t i = 0; i < 4; i++){
        pthread_create(&thread_pool[i], NULL, worker_thread, connection_pool);
    }
    
    //openting the ports
    int server_fd = setup_server_socket(8080);                          //a very important function
    printf("server is listening on http://localhost:8080\n");
    while(1){
        struct sockaddr_in client_address;
        socklen_t client_addr_len = sizeof(client_address);

        // 1. PICK UP THE PHONE
        // This function will BLOCK (freeze your program) until someone connects.
        int client_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_addr_len);

        if (client_socket < 0) {
            perror("Failed to accept connection");
            continue; // Don't crash the server, just skip to the next loop iteration
        }
        int clientIndex = get_free_connection_index(connection_pool, 1024);
        if (clientIndex < 0){
            printf("no valid connection index found");
            continue;
        }
        connection_pool[clientIndex].socket_fd = client_socket;
        bool enqueueState = enqueueOfClient(client_socket, clientIndex);
        if (!enqueueState){
            printf("failed to enqueue client");
            char *http_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nSorry, server is under load\n please try again later";
            write(client_socket, http_response, strlen(http_response));
            close(client_socket);
            continue;
        }
        // char *http_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, you are connected to the server\n";
        // write(client_socket, http_response, strlen(http_response));
        // close(client_socket);
        // connection_pool[clientIndex].state = Disable;
    }
}