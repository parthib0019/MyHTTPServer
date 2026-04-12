#pragma once
#include <stddef.h>
#include <stdbool.h>
//user defined data types
enum ConnectionState{
    Enable = 0,
    Disable = 1,
};
typedef enum ConnectionState ConnectionState;

//connection pool structures
struct Connection{
    int socket_fd;
    ConnectionState state;
};
typedef struct Connection Connection;

typedef struct client_node{
    int socket_fd;
    int node_id;
    struct client_node *next;
}ClientNode;



void init_connection_pool(Connection *pool, size_t size);
int get_free_connection_index(Connection *poll, size_t size);
int setup_server_socket(int port);

bool enqueueOfClient(int socket_fd, int node_id);
int* dequeueOfClient();
void *worker_thread(void *arg);
