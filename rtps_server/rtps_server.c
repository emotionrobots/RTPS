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
#include <arpa/inet.h>
#include <cjson/cJSON.h>

#include <global.h>


#define MAX_Y_PLOTS 		8
#define MAX_WINDOWS 	  	8	
#define MAX_STR_LEN 		64	


typedef struct {
   int fd;
   int socket;
   int port;
   bool connected;
   struct sockaddr_in address;
}
RTPS_Server;


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


RTPS_Window plotwin[MAX_WINDOWS] = {0};
int win_count = 0;



/*!
 * --------------------------------------------------------------------------------
 *
 *  @fn		int is_all_digits(const char *str) 
 *
 *  @brief 	 Returns 1 if str is made only of digits, 0 otherwise
 *
 * --------------------------------------------------------------------------------
 */
int is_all_digits(const char *str) 
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
 * --------------------------------------------------------------------------------
 *  
 *  @fn		int RTPS_init(RTPS_Server *svr, int port)
 *  
 *  @brief	Initialize Real-Time Plot Server
 *
 *  @param	svr	Preallocated RTPS_Server storage.
 *  @param	port	TCP port number
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------
 */
int RTPS_init(RTPS_Server *svr, int port)
{
    if (svr == NULL) return -1;

    svr->connected = false;
    svr->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr->fd < 0) 
    {
        perror("socket failed");
        return -2;
    }
    svr->port = port;
    svr->address.sin_family = AF_INET;
    svr->address.sin_addr.s_addr = INADDR_ANY; 
    svr->address.sin_port = htons(svr->port);

    if (bind(svr->fd, (struct sockaddr*)&svr->address, sizeof(svr->address)) < 0) 
    {
        perror("bind failed");
        close(svr->fd);
        return -3; 
    }

    return 0;
}


/*!
 * --------------------------------------------------------------------------------
 *  
 *  @fn		int RTPS_connect(RTPS_Server *svr, int *client_fd)
 *
 *  @brief	Connect to Real-Time Plot Server
 *
 *  @param	svr		RTPS server instance pointer
 *  @param	client_fd	Client file descriptor
 *
 *  @return	0 if success, negative otherwise
 *
 *--------------------------------------------------------------------------------
 */
int RTPS_connect(RTPS_Server *svr, int *client_fd)
{
   int rc = 0;

   *client_fd = -1;

   if (svr == NULL) 
      return -1; 
    

   int addrlen = sizeof(svr->address);
   if (listen(svr->fd, 1) < 0)
   {
      perror("listen failed");
      close(svr->fd);
      return -2;
   }
   
   *client_fd = accept(svr->fd, (struct sockaddr*)&svr->address, (socklen_t*)&addrlen);
   return 0;
}


/*!
 *---------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_recv(RTPS_Server *svr, int client_fd, 
 *                            char *message, size_t sz)
 *
 *  @brief	Read data from a connected socket
 *
 *  @param	svr		RTPS server instance pointer
 *  @param	client_fd	Client file descriptor
 *  @param 	message		Pointer to message buffer
 *  @param	sz		Size of the buffer
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------
 */
int RTPS_recv(RTPS_Server *svr, int client_fd, char *message, size_t sz)
{
   int rc = 0;

   int valread = recv(client_fd, message, sz-1, 0);
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
 *  @fn		int process_message(const char *json_str)
 *
 *  @brief      Process incoming message, which can take on the following forms:
 *
 *  		New window
 *
 *  		{  
 *  		   "cmd"     : "create"
 *  		   "window"  : "window1",
 *     		   "title"   : "My Window",
 *     		   "width"   : 800,
 *     		   "height"  : 600,
 *     		   "x_label" : "Time (s)",
 *     		   "y_label" : "Volt (V)",
 *     		   "x_step"  : 0.05,
 *     		   "x_range" : 6.28,
 *     		   "y_min"   : -1.1, 
 *     		   "y_max"   :  1.1,
 *     	    	   "y_count" : 3,
 *     		   "y_color" : [{ "r" : 255, "g" :   0, "b" :   0 },
 *                    	        { "r" :   0, "g" : 255, "b" :   0 },
 *                              { "r" :   0, "g" :   0, "b" : 255 }]
 *              }
 *
 *
 *  		Data frame:
 *
 *  		{  
 *  		   "cmd"     : "plot"
 *  		   "window"  : "window1",
 *      	   "data" : [0.25, 0.1, 2.4, 9.14]   // x, y0, y1, y2, etc...
 *  	        }
 *  
 *
 *           	Terminate window:
 *
 *  		{  
 *  		    "cmd"    : "destroy", 
 *  		    "window" : "window1"
 *   	        }
 *
 *  @param      json_str	Incoming JSON string (one of the above) 
 *
 *  @return     0 if success; negative otherwise
 *
 *--------------------------------------------------------------------------------
 */
int process_message(const char *json_str)
{
   int rc = 0;
   int i = win_count; 
   cJSON *cmd, *win, *title, *xl, *yl, *w, *h, *yc, *xs, *ys, *ymin, *ymax, *ycolor;
  
   // Parse root 
   cJSON *root = cJSON_Parse(json_str);
   if (root == NULL) 
   {
      printf("Parse error\n");
      rc = -1;
   }

   // Parse cmd
   if ( (cmd = cJSON_extract(root, 's', "cmd")) != NULL)
      printf("cmd = %s\n", cmd->valuestring);
   else
      printf("cmd is NULL\n");
      

   if ( (win = cJSON_extract(root, 's', "window")) != NULL)
      printf("window = %s\n", win->valuestring);
   else
      printf("window is NULL\n");
   
#if 0
   // Check new plot creation 
   int i = win_count; 
   if ( (win = cJSON_extract(root, 's', "window")) != NULL)
   {
      if (i > MAX_WINDOWS-1) 
      {
	 printf("Error: max number of windows reached.");
         rc = -2;
	 goto error_exit;
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
	    if (k >= MAX_Y_COUNT) break;
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

error_exit:
#endif
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
   bool done = false;
   char message[1024];
   RTPS_Server server = {0};

   // Check usage 
   if (argc < 2)
   {
      printf("Usage: rtps_server <port>\n");
      return -1;
   }
   else if (!is_all_digits(argv[1]))
   {
      printf("Error: <port> must be an integer.\n");
      return -1;
   }


   // Extract port 
   port = atoi(argv[1]);


   // Initialize the server
   if ((rc = RTPS_init(&server, port)) < 0)
   {
      printf("Server init failed. rc=%d\n", rc);
      return rc;
   }


   while (!done)
   {
      if (!server.connected)
      {
	 if ((rc = RTPS_connect(&server, &client)) < 0)
         {   
            printf("Socket connect failed. rc=%d\n", rc);
            return rc; 
         }
	 server.connected = true;
      }

      printf("Server listening on port %d.\n", port);

      // Start receiving loop 
      while (server.connected)
      {
         bzero(message, sizeof(message));
         if (0 == RTPS_recv(&server, client,  message, sizeof(message)))
         {
	    rc = process_message(message);
	    if (rc < 0) 
            {
               printf("Error processing message: %s\n", message);
               goto error_exit;
	    }
            printf("Processed message: %s\n", message);
         }
         else
         {
            server.connected = false;
         }
      }
   }
   return 0;

error_exit:
   printf("error returned %d\n", rc);
   return rc;
}
