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
#include <global.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <circular_buffer.h>


typedef struct {
   int fd;
   int socket;
   int port;
   int client;
   bool connected;
   struct sockaddr_in address;
}
RTPS_Connection;


typedef struct {
   int r, g, b, a;
}
RTPS_Color;


typedef struct {
//   char name[MAX_STR_LEN];
   char title[MAX_STR_LEN];
   char x_label[MAX_STR_LEN];
   char y_label[MAX_STR_LEN];
   int width;
   int height;
   int y_count;
   int max_points;
   double x_step;
   double x_range;
   double y_min;
   double y_max;
   double x_grid_step;
   double y_grid_step;
   SDL_Window *sdlwin;
   SDL_Renderer *sdlrendr;
   RTPS_Color y_color[MAX_Y_PLOTS];
   CircularBuffer cb;
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
 *  @fn         int RTPS_client_send(RTPS_Connection *conn, 
 *                                   RTPS_Window *win, 
 *                                   DataPoint *data)
 *
 *  @brief      Send a DataPoint 
 *
 *  @param      conn            RTPS_Connection
 *  @param      win             Pointer to RTPS_Window 
 *  @param      data            Pointer to DataPoint
 *
 *  @return     0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_client_send(RTPS_Connection *conn, RTPS_Window *win, DataPoint *data);



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_client_create_plot(RTPS_Connection *conn, RTPS_Window *plot)
 *  
 *  @brief      Initialize plot
 *
 *  @param      conn            Instantiated RTPS_Connection pointer
 *  @param      plot            Instantiated RTPS_Window pointer
 *
 *  @return     0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_client_create_plot(RTPS_Connection *conn, RTPS_Window *plot);



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_server_create(cJSON *root, RTPS_Window *window)
 *
 *  @brief      Create a new plot window from JSON
 *
 *  @param      root    cJSON root
 *  @param      window  Pointer to instantiated RTPS_Window
 *
 *  @return     0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_server_create(cJSON *root, RTPS_Window *window);



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         void RTPS_server_init()
 *
 *  @brief      Initialize RTPS server
 *
 *---------------------------------------------------------------------------------------
 */
void RTPS_server_init();



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_server_shutdown(RTPS_Window *win)
 *
 *  @brief      RTPS server shutdown
 *
 *  @param      win     RTPS_Window pointer
 *
 *  @return     0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_server_shutdown(RTPS_Window *win);


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         bool RTPS_server_forced_exit()
 *
 *  @return     true if forced exit
 *
 *---------------------------------------------------------------------------------------
 */
bool RTPS_server_forced_exit();



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_wait_for_connection(RTPS_Connection *conn, int port)
 *
 *  @brief      Initialize Real-Time Plot Server
 *
 *  @param      conn    Pointer to instantiated RTPS_Connection
 *  @param      port    TCP port number
 *
 *  @return     0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_wait_for_connection(RTPS_Connection *conn, int port);



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_server_update(RTPS_Connection *conn, RTPS_Window *win)
 *
 *  @brief      RTPS server loop update
 *
 *  @param      conn    RTPS_Connection pointer
 *  @param      win     RTPS_Window pointer
 *
 *  @return     0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_server_update(RTPS_Connection *conn, RTPS_Window *win);






#endif  // __RTPS_H__
