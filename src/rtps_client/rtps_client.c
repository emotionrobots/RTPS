/*!
 *=======================================================================================
 *
 * @file	rtps_client.c
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
#include <rtps.h>


// Default IP and port
#define PORT 12345
#define SERVER_IP "127.0.0.1"



int main() 
{
    char *message = "{\"cmd\"    : \"create\",\"window\":\"window1\"};";


    // Create socket
    RTPS_Connection *conn = RTPS_connect(SERVER_IP, PORT);
    if (conn == NULL)
    {
       RTPS_perror("Client cannot connect to server.");
       return -1;
    }

    // Send messages
    while (1) 
    {
       int rc = RTPS_send(conn, message, strlen(message));
       if (rc == 0)
          printf("Message sent: %s\n", message);
       sleep(1);
    }

    RTPS_disconnect(conn);

    return 0;
}
 
