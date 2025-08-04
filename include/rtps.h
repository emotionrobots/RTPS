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
 *  @fn         int RTPS_cjson_to_win(cJSON *root, RTPS_Window *win)
 *
 *  @brief      Convert cJSON to RTPS_Window object
 *
 *  @param      root            JSON root
 *  @param      win             Instantiated RTPS_Window object
 *
 *  @return     0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_cjson_to_win(cJSON *root, RTPS_Window *win);

/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_win_to_cjson(RTPS_Window *win, cJSON *root)
 *
 *  @brief      Converting RTPS_Window object to cJSON object
 *   
 *  @param      root            JSON root
 *  @param      win             Instantiated RTPS_Window object
 *
 *  @return     0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_win_to_cjson(RTPS_Window *win, cJSON *root);



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_cjson_to_data(cJSON *root, DataPoint *data)
 *
 *  @brief      Extract cJSON into DataPoint object
 *
 *  @param      root            root cJSON
 *  @param      data            DataPoint pointer
 *
 *  @return     0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_cjson_to_data(cJSON *root, DataPoint *dat);



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_data_to_cjson(DataPoint *dat, cJSON *root, int sz)
 *
 *  @brief      Convert DataPoint to equivalent cJSON
 *
 *  @param      dat     Pointer to the DataPoint object
 *  @param      root    Pointer to already-created cJSON object
 *  @param      win     RTPS_Window instance pointer 
 *
 *  @return     0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_data_to_cjson(DataPoint *dat, cJSON *root, RTPS_Window *win);




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
