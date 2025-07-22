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
#include <unistd.h>
#include <ctype.h>
#include <cjson/cJSON.h>
#include <rtps.h>


// Global variables
RTPS_Window plotwin[MAX_WINDOWS] = {0};
int win_count = 0;
bool connected = false;
bool done = false;


/*!
 * --------------------------------------------------------------------------------
 *  
 *  @fn		int RTPS_wait_for_connection(int port)
 *  
 *  @brief	Initialize Real-Time Plot Server
 *
 *  @param	port	TCP port number
 *
 *  @return	positive client id if success; negative otherwise
 *
 *---------------------------------------------------------------------------------
 */
static 
int RTPS_wait_for_connection(int port) 
{
   int client = -1;
   RTPS_Connection conn = {0};

   conn.connected = false;
   conn.fd = socket(AF_INET, SOCK_STREAM, 0);
   if (conn.fd < 0) 
   {
     RTPS_perror("Socket failed.");
     goto _err_ret;
   }
   
   conn.port = port;
   conn.address.sin_family = AF_INET;
   conn.address.sin_addr.s_addr = INADDR_ANY; 
   conn.address.sin_port = htons(conn.port);

   if (bind(conn.fd, (struct sockaddr*)&conn.address, sizeof(conn.address)) < 0) 
   { 
      RTPS_perror("Bind failed");
      goto _err_ret;
   }

   int addrlen = sizeof(conn.address);
   if (listen(conn.fd, 1) < 0)
   {
      RTPS_perror("Listen failed");
      goto _err_ret;
   }
   client = accept(conn.fd, (struct sockaddr*)&conn.address, (socklen_t*)&addrlen);
   return client;

_err_ret:
   RTPS_disconnect(&conn);
   return -1;
}



/*!
 *---------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_recv(RTPS_Connection *conn, char *message, size_t sz)
 *
 *  @brief      Read data from a connection
 *
 *  @param      conn            Pointer to established connection
 *  @param      message         Pointer to message buffer
 *  @param      sz              Size of the buffer
 *
 *  @return     0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------
 */
static
int RTPS_recv(int client, char *message, size_t sz)
{
   int rc = 0;

   int valread = recv(client, message, sz-1, 0);
   if (valread > 0)
   {
      message[valread] = '\0'; // Null-terminate the received message
   }
   else
   {
      rc = -1;
   }
   return rc;
}

/*!
 *--------------------------------------------------------------------------------
 *
 *  @fn		cJSON *cJSON_extract(cJSON *root, char typ, const char *key)
 *
 *  @brief	Extract cJSON object with type check
 *
 *  @param	root	Root JSON structure
 *  @param	typ	type character 's', 'n' or 'a'
 *  @param	key	JSON key to extract
 *
 *  @return	Pointer to the extracted JSON item corresponding to 'key'
 *
 *--------------------------------------------------------------------------------
 */
static 
cJSON *cJSON_extract(cJSON *root, char typ, const char *key)
{
   cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);

   if (typ == 's')
      return (cJSON_IsString(item)) ? item : NULL;
   else if (typ == 'n')
      return (cJSON_IsNumber(item)) ? item : NULL;
   else if (typ == 'a')
      return (cJSON_IsArray(item)) ? item : NULL;
   else
      return NULL;
}



/*!
 *--------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_create(cJSON *root)
 *
 *  @brief	Create a new plot window from JSON
 *
 *--------------------------------------------------------------------------------
 */
static
int RTPS_create(cJSON *root)
{
   int i = win_count;
   cJSON *win, *title, *xl, *yl, *w, *h, *yc, *xs, *ymin, *ymax, *ycolor;

   
   if ((win = cJSON_extract(root, 's', "window")) != NULL)
   {
      if (i > MAX_WINDOWS-1) 
      {
         RTPS_perror("Max number of windows reached.");
         return -1;
      }
      strncpy(plotwin[i].name,  win->valuestring, sizeof(plotwin[i].name));

      if ((title = cJSON_extract(root, 's', "title")) != NULL)
         strncpy(plotwin[i].title,  title->valuestring, sizeof(plotwin[i].title));

      if ((xl = cJSON_extract(root, 's', "x_label")) != NULL)
         strncpy(plotwin[i].x_label,  xl->valuestring, sizeof(plotwin[i].x_label));

      if ((yl = cJSON_extract(root, 's', "y_label")) != NULL)
         strncpy(plotwin[i].y_label,  yl->valuestring, sizeof(plotwin[i].y_label));

      if ((w = cJSON_extract(root, 'n', "width")) != NULL)
         plotwin[i].width = w->valueint; 

      if ((h = cJSON_extract(root, 'n', "height")) != NULL)
         plotwin[i].height = h->valueint;

      if ((yc = cJSON_extract(root, 'n', "y_count")) != NULL)
         plotwin[i].y_count = yc->valueint;

      if ((xs = cJSON_extract(root, 'n', "x_step")) != NULL)
         plotwin[i].x_step = xs->valuedouble; 

      if ((ymin = cJSON_extract(root, 'n', "y_min")) != NULL)
         plotwin[i].y_min = ymin->valuedouble;

      if ((ymax   = cJSON_extract(root, 'n', "y_max")) != NULL)
         plotwin[i].y_max = ymax->valuedouble;

      if ((ycolor = cJSON_extract(root, 'a', "y_color")) != NULL)
      { 
         int k = 0;
         cJSON *color;
         cJSON_ArrayForEach(color, ycolor) 
         {
            cJSON *r, *g, *b;
	    if (k >= MAX_Y_PLOTS) break;

            if ((r = cJSON_extract(color, 'n', "r")) != NULL)
               plotwin[i].y_color[k].r = r->valueint;

            if ((g = cJSON_extract(color, 'n', "g")) != NULL)
               plotwin[i].y_color[k].g = g->valueint;

            if ((b = cJSON_extract(color, 'n', "b")) != NULL)
               plotwin[i].y_color[k].b = b->valueint;

	    k++;
	 }
      }
      win_count++;
   }
   return 0;
}


/*!
 *--------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_process_message(const char *json_str)
 *
 *  @brief      Process incoming message, which can take on the following forms:
 *
 *              New window
 *
 *              {
 *                 "cmd"     : "create"
 *                 "window"  : "window1",
 *                 "title"   : "My Window",
 *                 "width"   : 800,
 *                 "height"  : 600,
 *                 "x_label" : "Time (s)",
 *                 "y_label" : "Volt (V)",
 *                 "x_step"  : 0.05,
 *                 "x_range" : 6.28,
 *                 "y_min"   : -1.1,
 *                 "y_max"   :  1.1,
 *                 "y_count" : 3,
 *                 "y_color" : [{ "r" : 255, "g" :   0, "b" :   0 },
 *                              { "r" :   0, "g" : 255, "b" :   0 },
 *                              { "r" :   0, "g" :   0, "b" : 255 }]
 *              }
 *
 *
 *              Data frame:
 *
 *              {
 *                 "cmd"     : "plot"
 *                 "window"  : "window1",
 *                 "data" : [0.25, 0.1, 2.4, 9.14]   // x, y0, y1, y2, etc...
 *              }
 *
 *
 *              Terminate window:
 *
 *              {
 *                  "cmd"    : "destroy",
 *                  "window" : "window1"
 *              }
 *
 *  @param      json_str        Incoming JSON string (one of the above)
 *
 *  @return     0 if success; negative otherwise
 *
 *--------------------------------------------------------------------------------
 */
static
int RTPS_process_message(const char *json_str)
{
   int rc = 0;
   cJSON *cmd;

   // Parse JSON string 
   cJSON *root = cJSON_Parse(json_str);
   if (root == NULL)
   {
      RTPS_perror("parse error.");
      rc = -1;
   }
   else if ( (cmd = cJSON_extract(root, 's', "cmd")) != NULL)
   {
      printf("cmd = %s\n", cmd->valuestring);

      // Parse and process different command type 
      if (0 == strcmp(cmd->valuestring, "create"))
      {
         rc = RTPS_create(root);
      }
      else if (0 == strcmp(cmd->valuestring, "plot"))
      {
         // rc = RTPS_plot(root);
      }
      else if (0 == strcmp(cmd->valuestring, "destroy"))
      {
         // rc = RTPS_destroy(root);
      }
      else
      {
         RTPS_perror("unrecognized command.");
         rc = -1;
      }
   }
   else
   {
      RTPS_perror("no command found.");
      rc = -1;
   }

   return rc;
}



/*!
 *--------------------------------------------------------------------------------
 *
 *  @fn	 	main()
 *
 *  @brief	Main logic of Real-Time Plot Server
 *
 *--------------------------------------------------------------------------------
 */
int main(int argc, char *argv[]) 
{
   int rc = -1;
   int port = 12345;
   int client;
   char message[MAX_STR_LEN];

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


   // Extract port 
   port = atoi(argv[1]);

   
   while (!done)
   {
      // Wait for client connection
      if ((client = RTPS_wait_for_connection(port)) < 0)
      {
         RTPS_perror("Wait for connection failed.");
         goto _err_ret; 
      }
      printf("Server listening on port %d.\n", port);
      connected = true;

      // Receiving loop 
      while (connected)
      {
         bzero(message, sizeof(message));
         if (0 == RTPS_recv(client,  message, sizeof(message)))
         {
	    rc = RTPS_process_message(message);
	    if (rc < 0) 
            {
               RTPS_perror("Error processing message");
               goto _err_ret;
	    }
            printf("Processed message: %s\n", message);
         }
         else
         {
            close(client); 
            connected = false;
            client = -1;
         }
      }
   }
   return 0;

_err_ret:
   return rc;
}
