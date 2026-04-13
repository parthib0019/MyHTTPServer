#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "../include/myserver.h"
#include <pthread.h>

////// some variable declarations
// waiting queue
ClientNode *head = NULL;
ClientNode *tail = NULL;
bool dequeueAble = true;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;    

//////function declearation
void init_connection_pool(Connection *pool, size_t size){
    for(size_t i = 0; i < size; i++){
        pool[i].socket_fd =-1;
        pool[i].state = Disable;
    }
    return;
}

int get_free_connection_index(Connection *pool, size_t size){
    for(size_t i = 0; i < size; i++){
        if(pool[i].state == Disable){
            pool[i].state = Enable;
            return i;
        }
    }
    return -1;
}

int setup_server_socket(int port){
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    //asking for the socket connection from the os
    server_fd = socket(2, SOCK_STREAM, 0);
    if(server_fd < 0){
        perror("failed to get the socket");
        exit(EXIT_FAILURE);
    }else{
        printf("socket created successfully\n");
    }

    //setting the socket options just like phone number
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        perror("failed to set the socket options");
        exit(EXIT_FAILURE);
    }else{
        printf("socket options set successfully\n");
    }

    // 2. PICK THE PHONE NUMBER
    address.sin_family = AF_INET;           // We are using IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // Listen on any available network interface
    address.sin_port = htons(port);         // Set the port (htons converts it to network byte order)
    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    // 3. HIRE THE RECEPTIONIST
    // Put the socket into listening mode. The '128' means we allow 128 people
    // to wait in line before the OS starts rejecting their connections.
    if (listen(server_fd, 128) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

bool enqueueOfClient(int socket_fd, int node_id){
    ClientNode  *new_node = (ClientNode*)malloc(sizeof(ClientNode));
    if (new_node == NULL){
        perror("failed to allocate memory for new node");
        return false;
    }
    new_node->socket_fd = socket_fd;
    new_node->node_id = node_id;
    new_node->next = NULL;
    pthread_mutex_lock(&queue_mutex);
    if (tail == NULL){
        head = new_node;
        tail = new_node;
    }else{
        tail->next = new_node;
        tail = new_node;
    }
    pthread_mutex_unlock(&queue_mutex);
    return true;
}

int* dequeueOfClient(){
    pthread_mutex_lock(&queue_mutex);
    if (head == NULL || dequeueAble == false){
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
    }else if(head->next == NULL){
        dequeueAble = false;
        int *arrClient = (int *)malloc(2*sizeof(int));
        arrClient[0] = head->socket_fd;
        arrClient[1] = head->node_id;
        head = NULL;
        free(tail);
        tail = NULL;
        dequeueAble = true;
        pthread_mutex_unlock(&queue_mutex);
        return arrClient;
    }else{
        dequeueAble = false;
        ClientNode *temp = head;
        head = head->next;
        int *arrClient = (int *)malloc(2*sizeof(int));
        arrClient[0] = temp->socket_fd;
        arrClient[1] = temp->node_id;
        free(temp);
        dequeueAble = true;
        pthread_mutex_unlock(&queue_mutex);
        return arrClient;
    }
}

void *worker_thread(void *args){
    Connection *connection_pool = (Connection *)args;
    while(1){
        int* arrClient = dequeueOfClient();
        if (arrClient == NULL){
            usleep(1000);
            continue;
        }
        int client_socket = arrClient[0];
        int node_id = arrClient[1];

        // reading the request of the browser
        char buffer[4096];
        memset(buffer, 0, 4096);
        read(client_socket, buffer, 4096-1);
        printf("Browser says:\t%s\n", buffer);
        //writing the response to the browser
        char *http_response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, you are connected to the server\n";
        write(client_socket, http_response, strlen(http_response));
        close(client_socket);
        connection_pool[node_id].state = Disable;
        free(arrClient);
    }
}