#include "mysocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>






int my_socket(int domain, int type, int protocol) {
    // Only accept SOCK_MyTCP sockets
    if (type != SOCK_MyTCP) {
        exit(1);
    }

    // Create the TCP socket
    int sockfd = socket(domain, SOCK_STREAM, protocol);
    if (sockfd < 0) {
        exit(1);
    }
   // int num = 4;

    // Initialize the message tables
    
    send_table.head = 0;
    send_table.tail = 0;
    send_table.count = 0;
    pthread_mutex_init(&send_table.lock, NULL);
    send_table.send_cv = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
   // pthread_cond_init()

   // message_table_t receive_table;
    receive_table.head = 0;
    receive_table.tail = 0;
    receive_table.count = 0;
    pthread_mutex_init(&receive_table.lock, NULL);
    pthread_mutex_init(&sockfd_lock, NULL);

   // pthread_mutex_init(&sockfd_lock, NULL);
    receive_table.recv_cv = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    int* sockfd_ptr = malloc(sizeof(int)); // allocate memory for sockfd
    *sockfd_ptr = sockfd; // initialize sockfd_ptr with sockfd value
    //pthread_t send_tid;
    //pthread_create(&send_tid, NULL, S_thread, (void*)sockfd_ptr);

    // Create the send and receive threads

    printf("%d is fd1\n", sockfd);
    if (pthread_create(&send_tid, NULL, S_thread, (void*)sockfd_ptr) != 0)
    {
        perror("pthread_create S");
        return -1;
    }
    if (pthread_create(&send_tid, NULL, R_thread, (void*)sockfd_ptr) != 0)
    {
        perror("pthread_create R");
        return -1;
    }
   // pthread_create(&send_tid, NULL, S_thread, (void*)&num);
    //printf("%d is fd2\n", sockfd);
    //pthread_create(&receive_tid, NULL, R_thread, (void*)&sockfd);
    

    // TODO: Additional initialization, if necessary
    printf("MY_SOCKET DONE\n");
    return sockfd;
}


void my_send(int sockfd, const void *buf, size_t len) {
   // pthread_mutex_lock(&newfd_lock);
   // printf("%d is newfd in send\n", newfd);
   // pthread_mutex_unlock(&newfd_lock);
    //printf("%d is sockfd arg in send\n", sockfd);
    pthread_mutex_lock(&send_table.lock);
    while (send_table.messages[send_table.tail].length != 0) {
        pthread_cond_wait(&send_table.send_cv, &send_table.lock);
    }
    send_table.messages[send_table.tail].length = len;
    send_table.messages[send_table.tail].fd = sockfd;
   // printf("%ld is msglen\n", len);
    memcpy(send_table.messages[send_table.tail].message, buf, len);
   // printf("%s is buf1\n", send_table.messages[send_table.tail].message);
    send_table.tail = (send_table.tail + 1) % MAX_TABLE_ENTRIES;
       // printf("%s is buf2\n", send_table.messages[send_table.tail].message);
   // printf("%d is tail\n", send_table.tail);
    //printf("%d is head\n", send_table.head);
    pthread_cond_signal(&send_table.send_cv);
    pthread_mutex_unlock(&send_table.lock);
    
    printf("MY_SEND DONE\n");
}

char* my_recv(int sockfd, void *buf, size_t len) {
    //printf("in recv\n");
   // pthread_mutex_lock(&newfd_lock);
    //printf("%d is newfd in recv\n", newfd);
    int found = 0;
    int msg_len = 0;
    pthread_mutex_lock(&receive_table.lock);
    while (!found) {
    for (int i = 0; i < MAX_TABLE_ENTRIES; i++) {
        if (receive_table.messages[i].fd == sockfd) {
            found = 1;
            msg_len = receive_table.messages[i].length;
            //printf("%d is msglen\n", msg_len);
            memcpy(buf, receive_table.messages[i].message, msg_len);
            //printf("%p is buf\n", buf);
    
            memset(receive_table.messages[i].message, 0, MAX_MESSAGE_LENGTH);
            receive_table.messages[i].length = 0;
            receive_table.messages[i].fd = 0;
            receive_table.head = (i + 1) % MAX_TABLE_ENTRIES;
            //receive_table.messages[receive_table.head].fd = sockfd;
            pthread_cond_signal(&receive_table.send_cv);
            break;
        }
    }
    if (!found) {
        printf("No message found for sockfd %d. Waiting...\n", sockfd);
        //printf("cond wait %d is f`1\n", found);
        pthread_cond_wait(&receive_table.send_cv, &receive_table.lock);
       // printf("cond wait %d is f\n", found);
        continue;
    }
}


    pthread_mutex_unlock(&receive_table.lock);
    
    printf("MY_RECV DONE\n");
   // printf("%p is buf\n", buf);
    //} else return 0;
    return buf;
}


// Thread S
void* S_thread(void* arg) {
   // printf("IN STHREAD\n");
    

    int sock = *(int*)arg;
    //printf("%d is sockfd in s\n", sock);
    //printf("Sthread2\n");
    int var = send_table.tail - 1;

    while (1) {
        // Check if there is a message to send
        //printf("s3\n"); send_table.messages[var].length != 0
        
        if (send_table.head != send_table.tail ) {
            //printf("insend if\n");
            // Send the message using one or more send calls
            pthread_mutex_lock(&send_table.lock);
            while (send_table.tail != 0 && send_table.messages[send_table.tail - 1].length == 0) {
        pthread_cond_wait(&send_table.send_cv, &send_table.lock);
    }

            int remaining_len = send_table.messages[var].length;
            char* remaining_data = send_table.messages[var].message;
            int sendfd = send_table.messages[var].fd;

            //printf("%d is remlen\n", remaining_len);
            //printf("%s is remdata\n", remaining_data);
             //printf("%d is sendfd\n", sendfd);

            while (remaining_len > 0 && sendfd != 0) {
                int len_to_send = remaining_len > MAX_SEND_LEN ? MAX_SEND_LEN : remaining_len;
                //pthread_mutex_lock(&sockfd_lock);
                int bytes_sent = send(sendfd, remaining_data, len_to_send, 0);
                //pthread_mutex_unlock(&sockfd_lock);
                if (bytes_sent == -1) {
                    perror("send failed\n");
                    exit(1);
                }
                //printf("%d is bytes\n", bytes_sent);
                remaining_len -= bytes_sent;
                remaining_data += bytes_sent;
            }

            // Move to the next message
            memset(send_table.messages[var].message, 0, MAX_MESSAGE_LENGTH);
            send_table.messages[var].length = 0;
            send_table.messages[var].fd = 0;
            pthread_cond_signal(&send_table.send_cv);
            pthread_mutex_unlock(&send_table.lock);
            var = (var + 1) % MAX_TABLE_ENTRIES;
        }

        // Sleep for some time before checking again
        sleep(1);  // 10 ms
    }

    return NULL;
}

// Thread R
void* R_thread(void* arg) {
   // printf("IN rthread\n");

    //int sock = *(int *)arg;
    //pthread_mutex_lock(&sockfd_lock);
    //    int sockfd = sock;
    //    pthread_mutex_unlock(&sockfd_lock);

    int sock1 =  *(int*)arg;
    
    //printf("r2\n");
    int remaining_len;
    char *remaining_data;
    while (1) {
       // printf("r3\n");
      // printf("%d is sockfd in r\n", sock1);
      // printf("%d is newfd in r\n", newfd);
       

        // Receive data from the TCP socket
        //if (receive_table.head != receive_table.tail){
        char recv_buffer[MAX_MESSAGE_LENGTH];
        
        //int bytes_received; 
        int sock;
        if(newfd == 0) sock = sock1;
        else  sock = newfd;
        //pthread_mutex_lock(&sockfd_lock);
        int bytes_received = 0;
        //do{
        bytes_received = recv(sock, recv_buffer, sizeof(recv_buffer), 0);
        //pthread_mutex_unlock(&sockfd_lock);
        if (bytes_received <= 0) {
           // printf("receive problem\n");
            sleep(3);
            //exit(1);
        }
       // printf("%d is br\n", bytes_received);
        //int bytes_received1 = recv(newfd, recv_buffer, sizeof(recv_buffer), 0);
        
        /*else if (bytes_received == 0) {
            // The connection was closed by the other end
            printf("Connection closed by the other end\n");
            break;
        }*/
        remaining_len = bytes_received;
        remaining_data = recv_buffer;
        // Interpret the data as a message
        
         //printf("%d is rl rthread\n", bytes_received);
        
        // printf("%s is r data r thread\n", recv_buffer);
        // } while(bytes_received > 0);

        
        //int 
        //char*
        while (remaining_len > 0) {
            pthread_mutex_lock(&receive_table.lock);
            while (receive_table.messages[receive_table.head].length != 0) {
            pthread_cond_wait(&receive_table.send_cv, &receive_table.lock);
            }
            //printf("%d is rt head\n", receive_table.head);
            //printf("%d is rt tail\n", receive_table.tail);
            // Check if there is enough space in the Received_Message table
            if ((receive_table.head + 1) % MAX_TABLE_ENTRIES != receive_table.tail) {
                // Add the message to the Received_Message table
                int len_to_copy = remaining_len > MAX_MESSAGE_LENGTH ? MAX_MESSAGE_LENGTH : remaining_len;
                //printf("%d is len to copy\n", len_to_copy);
                memcpy(receive_table.messages[receive_table.head].message, remaining_data, len_to_copy);
                receive_table.messages[receive_table.head].length = len_to_copy;
                receive_table.messages[receive_table.head].fd = sock;
               // printf("%d is head\n", receive_table.head);
               // printf("%d is ffd\n",receive_table.messages[receive_table.head].fd);


                // Move to the next message
                receive_table.head = (receive_table.head + 1) % MAX_TABLE_ENTRIES;
                remaining_len -= len_to_copy;
                remaining_data += len_to_copy;
            } else {
                // The Received_Message table is full, drop the message
                printf("Received_Message table full, dropping message\n");
                break;
                
            }
            int ret = pthread_cond_signal(&receive_table.send_cv);
          //  printf("%d is returned\n", ret);
            pthread_mutex_unlock(&receive_table.lock);
            
        }
       // printf("MSG SENT\n");
    //}
    }

    return NULL;
}


// void* R_thread(void* arg) {
//     printf("IN rthread\n");

//     int sock1 = *(int *)arg;
    
//     int remaining_len = 0;
//     char recv_buffer[MAX_MESSAGE_LENGTH];
//     char *remaining_data = NULL;
//     while (1) {
        
//         // Receive data from the TCP socket
//         int bytes_received = recv(sock, recv_buffer, sizeof(recv_buffer), 0);
//         if (bytes_received <= 0) {
//             printf("receive problem\n");
//             sleep(3);
//         }
        
//         // Concatenate the received buffer with any remaining data from previous receives
//         char *message_start = recv_buffer;
//         int message_len = bytes_received;
//         if (remaining_len > 0) {
//             int total_len = remaining_len + bytes_received;
//             if (total_len > MAX_MESSAGE_LENGTH) {
//                 // The message is too long to fit in the buffer, drop it
//                 printf("Message too long, dropping it\n");
//                 remaining_len = 0;
//                 remaining_data = NULL;
//                 continue;
//             }
//             char *temp = malloc(total_len);
//             memcpy(temp, remaining_data, remaining_len);
//             memcpy(temp + remaining_len, recv_buffer, bytes_received);
//             message_start = temp;
//             message_len = total_len;
//             free(remaining_data);
//         }
        
//         // Find the end of the message, if it exists
//         char *message_end = memchr(message_start, '\n', message_len);
//         if (message_end != NULL) {
//             // We have a complete message, add it to the Received_Message table
//             int message_len_with_null = message_end - message_start + 1;
//             pthread_mutex_lock(&receive_table.lock);
//             while (receive_table.messages[receive_table.head].length != 0) {
//                 pthread_cond_wait(&receive_table.send_cv, &receive_table.lock);
//             }
//             if ((receive_table.head + 1) % MAX_TABLE_ENTRIES != receive_table.tail) {
//                 memcpy(receive_table.messages[receive_table.head].message, message_start, message_len_with_null);
//                 receive_table.messages[receive_table.head].length = message_len_with_null;
//                 receive_table.messages[receive_table.head].fd = sock;
//                 receive_table.head = (receive_table.head + 1) % MAX_TABLE_ENTRIES;
//             } else {
//                 printf("Received_Message table full, dropping message\n");
//             }
//             pthread_cond_signal(&receive_table.send_cv);
//             pthread_mutex_unlock(&receive_table.lock);
            
//             // Update the remaining data buffer, if there is any
//             int remaining_len_with_null = message_len - message_len_with_null;
//             if (remaining_len_with_null > 0) {
//                 remaining_data = malloc(remaining_len_with_null);
//                 memcpy(remaining_data, message_end + 1, remaining_len_with_null);
//                 remaining_len = remaining_len_with_null;
//             } else {
//                 remaining_data = NULL;
//                 remaining_len = 0;
//             }
//         } else {
//             // We don't have a complete message yet, save the remaining data for later
//             remaining_data = realloc(remaining_data, message_len);
//             memcpy(remaining_data, message_start, message_len);
//             remaining_len = message_len;
//         }
//     }

//     return NULL;
// }



int my_connect(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
{
    int ret = connect(sockfd, addr, addrlen);
    if (ret < 0)
    {
        fprintf(stderr, "Error: Failed to connect to server\n");
        return -1;
    }
    printf("MY_CONNECT DONE\n");
    // Create receive and send threads
    //pthread_create(&socket->thread_R, NULL, thread_R_func, (void *)socket);
    //pthread_create(&socket->thread_S, NULL, thread_S_func, (void *)socket);

    return 0;
}


int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
   // int newfd;
    
   // int *n =  malloc(sizeof(int));
    // Accept incoming connection and get new socket file descriptor
    pthread_mutex_lock(&newfd_lock);
    newfd = accept(sockfd, addr, addrlen);
    pthread_mutex_unlock(&newfd_lock);
    if (newfd == -1)
    {
        perror("accept\n");
        return -1;
    }

    printf("MY_ACCEPT DONE\n");

    return newfd;
}


int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    // Bind the socket to the specified address and port
    int bind_result = bind(sockfd, addr, addrlen);
    if (bind_result == -1)
    {
        perror("bind");
        exit(1);
    }
     printf("MY_BIND DONE\n");

    // Return 0 to indicate success
    return 0;
}

int my_listen(int sockfd, int b)
{
    // Listen for incoming connections
    int listen_result = listen(sockfd, BACKLOG);
    if (listen_result == -1)
    {
        perror("listen");
        exit(1);
    }
     printf("MY_LISTEN DONE\n");
    // Return 0 to indicate success
    return 0;
}



int my_close(int sockfd) {
    sleep(10);
    // Close the TCP socket
   // printf("waiting for join\n");
    pthread_join(receive_tid, NULL);
   // printf("R joined\n");
    pthread_join(send_tid, NULL);
   // printf("S joined\n");
    int ret = close(sockfd);
    if (ret == -1) {
        perror("Error closing socket");
        return -1;
    }
    
    printf("MY_CLOSE DONE\n");
    // Kill the threads and free the message buffers
   // free(*n);
    
   // pthread_cancel(send_tid);
   // pthread_cancel(receive_tid);
   // free(sockfd_ptr);
    
    //free(send_table.messages);
    //free(receive_table.messages);
    
    return 0;
}


/*void my_close(MyTCP *socket)
{
    if (socket == NULL)
    {
        fprintf(stderr, "Error: Invalid socket\n");
        return;
    }

    // Cancel receive and send threads
    pthread_cancel(thread_R);
    pthread_cancel(socket->thread_S);

    // Wait for the threads to finish and clean up resources
    pthread_join(socket->thread_R, NULL);
    pthread_join(socket->thread_S, NULL);

    // Close the socket
    close(socket->sockfd);

    // Free the memory used by the socket
    free(socket);
}*/





























/*
pthread_mutex_lock(&receive_table.lock);
int found = 0;
while (!found) {
    for (int i = 0; i < MAX_TABLE_ENTRIES; i++) {
        if (receive_table.messages[i].fd == sockfd) {
            found = 1;
            int msg_len = receive_table.messages[i].length;
            memcpy(buf, receive_table.messages[i].message, msg_len);
            memset(receive_table.messages[i].message, 0, MAX_MESSAGE_LENGTH);
            receive_table.messages[i].length = 0;
            receive_table.head = (i + 1) % MAX_TABLE_ENTRIES;
            receive_table.messages[receive_table.head].fd = sockfd;
            pthread_cond_signal(&receive_table.send_cv);
            break;
        }
    }
    if (!found) {
        printf("No message found for sockfd %d. Waiting...\n", sockfd);
        pthread_cond_wait(&receive_table.send_cv, &receive_table.lock);
    }
}
pthread_mutex_unlock(&receive_table.lock);
return msg_len;*/
