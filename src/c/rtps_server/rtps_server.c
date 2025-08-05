/*!
 *=======================================================================================
 *
 * @file	rtps_server.c
 *
 * @brief	Real-Time Plot Server
 *
 *=======================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
//#include <SDL2/SDL.h>
//#include <SDL2/SDL2_gfxPrimitives.h>
//#include <cjson/cJSON.h>
#include <rtps.h>


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn	 	main()
 *
 *---------------------------------------------------------------------------------------
 */
int main(int argc, char *argv[]) 
{
   int rc = 0;
   int port = 12345;
   RTPS_Window plotwin = {0};
   RTPS_Connection conn = {0};


   // Check usage 
   if (argc < 2)
   {
      printf("Usage: rtps_server <port>\n");
      return -1;
   }
   else if (!RTPS_is_all_digits(argv[1]))
   {
      printf("Error: <port> must be an integer.\n");
      return -1;
   }
   port = atoi(argv[1]);


   // Init SDL
   RTPS_server_init();


   // Wait for client connection
   if (RTPS_wait_for_connection(&conn, port) < 0)
   {
      RTPS_perror("Wait for connection failed.");
      goto _err_ret;
   }
   printf("Server listening on port %d.\n", port);


   while (!RTPS_server_forced_exit())
   {
      RTPS_server_update(&conn, &plotwin);
   } 

_err_ret:
   RTPS_server_shutdown(&plotwin);
   return rc;
}
