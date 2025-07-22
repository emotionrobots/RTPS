/*!
 *=======================================================================================
 *
 * @file	client_main.c
 *
 * @brief	Test RTPS client main program
 *
 *=======================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORT 12345
#define SERVER_IP "127.0.0.1"


int main() 
{
    int sockfd;
    struct sockaddr_in server_addr;
    char *message = "{\"cmd\"    : \"create\",\"window\":\"window1\"};";


    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send message
    while (1) 
    {
       send(sockfd, message, strlen(message), 0);
       printf("Message sent to receiver: %s\n", message);
       sleep(1);
    }

    close(sockfd);
    return 0;
}
 
