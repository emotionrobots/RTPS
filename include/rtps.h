/*!
 *---------------------------------------------------------------------------
 *
 * @file	rtps.h
 *
 * @brief	RTPS header file
 *
 *---------------------------------------------------------------------------
 */
#ifndef __RTPS_H__
#define __RTPS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>


#define DATA_TYPE 	 	RTPS_Data	
#define MAX_Y_PLOTS             8
#define MAX_WINDOWS             8       
#define MAX_STR_LEN             64   


typedef struct {
   int fd;
   int socket;
   int port;
   bool connected;
   struct sockaddr_in address;
}
RTPS_Connection;


typedef struct {
   int count;
   double x;
   double y[MAX_Y_PLOTS];
} 
RTPS_Data;


typedef struct {
   int r, g, b, a;
}
RTPS_Color;


typedef struct {
   char name[MAX_STR_LEN];
   char title[MAX_STR_LEN];
   char x_label[MAX_STR_LEN];
   char y_label[MAX_STR_LEN];
   int width;
   int height;
   int y_count;
   double x_step;
   double x_range;
   double y_min;
   double y_max;
   RTPS_Color y_color[MAX_Y_PLOTS];
}
RTPS_Window;


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         void RTPS_perror(char *msg)
 *
 *  @brief      Print error message
 *
 *---------------------------------------------------------------------------------------
 */
void RTPS_perror(char *msg);



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_is_all_digits(const char *str)
 *
 *  @brief       Returns 1 if str is made only of digits, 0 otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_is_all_digits(const char *str);


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         RTPS_Connection *RTPS_connect(char *ipaddr, int port)
 *
 *  @brief      Connect to a Real-Time Plot Server at <ipaddr:port>
 *
 *  @param      ipaddr          IP address where the RTPS server is running
 *  @param      port            Port at which RTPS is listening
 *
 *  @return     RTPS_Connection pointer if successful, otherwise NULL
 *
 *---------------------------------------------------------------------------------------
 */
RTPS_Connection *RTPS_connect(char *ipaddr, int port);


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         void RTPS_disconnect(RTPS_Connection *conn)
 *
 *  @brief      Disconnect from RTPS
 *
 *---------------------------------------------------------------------------------------
 */
void RTPS_disconnect(RTPS_Connection *conn);



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_send(RTPS_Connection *conn, char *message, size_t sz)
 *
 *  @brief      Send a message of sz bytes
 *
 *  @param      conn            RTPS_Connection
 *  @param      message         Message string
 *  @param      sz              Size of the message
 *
 *  @return     0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_send(RTPS_Connection *conn, char *message, size_t sz);




#endif  // __RTPS_H__
