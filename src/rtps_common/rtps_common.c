/*!
 *=======================================================================================
 *
 * @file	rtps_client_lib.c
 *
 * @brief	Real-Time Plot Client Library 
 *
 *=======================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>
#include <rtps.h>


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		void RTPS_perror(char *msg)
 *
 *  @brief	Print error message
 *
 *---------------------------------------------------------------------------------------
 */
void RTPS_perror(char *msg)
{
   printf("Error: %s\n", msg);
}


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_is_all_digits(const char *str) 
 *
 *  @brief 	 Returns 1 if str is made only of digits, 0 otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_is_all_digits(const char *str) 
{
   if (!str || !*str) return 0; // Empty string or NULL is not valid

   for (; *str; ++str) 
   {
      if (!isdigit((unsigned char)*str)) 
         return 0;
   }
   return 1;
}



/*!
 *---------------------------------------------------------------------------------------
 *  
 *  @fn		RTPS_Connection *RTPS_connect(char *ipaddr, int port)
 *
 *  @brief	Connect to a Real-Time Plot Server at <ipaddr:port>
 *
 *  @param	ipaddr	        IP address where the RTPS server is running	
 *  @param	port	 	Port at which RTPS is listening	
 *
 *  @return	RTPS_Connection pointer if successful, otherwise NULL 
 *
 *---------------------------------------------------------------------------------------
 */
RTPS_Connection *RTPS_connect(char *ipaddr, int port)
{
    RTPS_Connection *conn = NULL;
    
    if (ipaddr == NULL) 
    {
       RTPS_perror("IP address is NULL");
       goto _err_ret;
    }

    conn = (RTPS_Connection *)malloc(sizeof(RTPS_Connection));
    if (conn != NULL) 
    {
       conn->connected = false;
       conn->fd = socket(AF_INET, SOCK_STREAM, 0);
       if (conn->fd < 0)
       {
          RTPS_perror("Socket creation failed");
	  goto _err_ret; 
       }
       conn->port = port;
       conn->address.sin_family = AF_INET;
       conn->address.sin_port = htons(conn->port);
       inet_pton(AF_INET, ipaddr, &conn->address.sin_addr);

       if (connect(conn->fd, (struct sockaddr*)&conn->address, sizeof(conn->address)) < 0) 
       {
          RTPS_perror("Connection failed");
	  close(conn->fd);
	  goto _err_ret;
       }
       return conn;
    }

_err_ret:
    if (conn != NULL) free(conn);
    return NULL;
} 


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		void RTPS_disconnect(RTPS_Connection *conn)
 *
 *  @brief	Disconnect from RTPS
 *
 *---------------------------------------------------------------------------------------
 */
void RTPS_disconnect(RTPS_Connection *conn)
{
   if (conn != NULL)
   {
      close(conn->fd);
   }
}


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_send(RTPS_Connection *conn, char *message, size_t sz)
 *
 *  @brief	Send a message of sz bytes
 *
 *  @param	conn		RTPS_Connection
 *  @param	message		Message string
 *  @param	sz		Size of the message
 *
 *  @return	0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_send(RTPS_Connection *conn, char *message, size_t sz)
{
   if (conn == NULL)
   {
      RTPS_perror("Connection is NULL");
      return -1;
   }
   send(conn->fd, message, sz, 0);
   return 0;
}


