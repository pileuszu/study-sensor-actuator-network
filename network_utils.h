#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/**
 * Handle errors in a consistent way
 * @param message Error message to display
 */
void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

/**
 * Set up a server socket
 * @param port Port number to listen on
 * @return Socket file descriptor on success
 */
int setup_server_socket(int port) {
    int server_socket;
    struct sockaddr_in server_addr;
    
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(server_socket == -1)
        error_handling("socket() error");
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        error_handling("bind() error");
    
    if(listen(server_socket, 5) == -1)
        error_handling("listen() error");
    
    return server_socket;
}

/**
 * Accept a client connection
 * @param server_socket Server socket file descriptor
 * @return Client socket file descriptor on success
 */
int accept_client(int server_socket) {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
    if(client_socket == -1)
        error_handling("accept() error");
    
    return client_socket;
}

/**
 * Connect to a server
 * @param ip_address IP address of the server
 * @param port Port number of the server
 * @return Socket file descriptor on success
 */
int connect_to_server(char *ip_address, int port) {
    int sock;
    struct sockaddr_in server_addr;
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error_handling("socket() error");
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    server_addr.sin_port = htons(port);
    
    if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        error_handling("connect() error");
    
    return sock;
}

#endif /* NETWORK_UTILS_H */ 