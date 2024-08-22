#ifndef MAIN_H
#define MAIN_H

#define SOCK_MyTCP 6

#define MAX_MESSAGE_LENGTH 5000
#define MAX_TABLE_ENTRIES 10
//#define MAX_MESSAGE_LEN 5000
//#define MAX_TABLE_ENTRIES 10
#define MAX_SEND_LEN 1000
#define BACKLOG 10
#include<pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<stdlib.h>

// Define the message structure
typedef struct {
    char message[MAX_MESSAGE_LENGTH];
    int length;
    int fd;
} message_t;

// Define the send and receive message tables
typedef struct {
    message_t messages[MAX_TABLE_ENTRIES];
    pthread_mutex_t lock;
    pthread_cond_t send_cv;
    pthread_cond_t recv_cv;
    int head;
    int tail;
    int count;
} message_table_t;

pthread_mutex_t sockfd_lock;
pthread_mutex_t newfd_lock;

typedef struct
{
    int sockfd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    //char send_buf[MAX_MESSAGES];
    //int send_len;
    //char recv_buf[MAX_MESSAGE_LEN];
    //int recv_len;
    //int recv_pos;
} MyTCP;

int newfd;

// Define the send and receive threads
void* S_thread(void* arg);
void* R_thread(void* arg);

pthread_t send_tid;
pthread_t receive_tid;

message_table_t send_table;
message_table_t receive_table;

int my_socket(int domain, int type, int protocol);
void my_send(int sockfd, const void *buf, size_t len);
char* my_recv(int sockfd, void *buf, size_t len);
//int my_connect(MyTCP *socket, const char *ip_address, const int port);
int my_connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen);
//int my_accept(MyTCP *sockfd, MyTCP *newsockfd);
int my_accept(int sockfd, struct sockaddr * addr, socklen_t *addrlen);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_listen(int sockfd, int b);
int my_close(int sockfd);

#endif /* MAIN_H */
