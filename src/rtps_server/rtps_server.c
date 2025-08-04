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
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <cjson/cJSON.h>
#include <rtps.h>


#define PLOT_MARGIN_LEFT 		60
#define PLOT_MARGIN_RIGHT 		20
#define PLOT_MARGIN_TOP 		60
#define PLOT_MARGIN_BOTTOM 		60


// Global variables
RTPS_Window plotwin = {0};
bool connected = false;
RTPS_Connection conn = {0};


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		void draw_grid(RTPS_Window *window, double x_offset)
 *
 *  @brief	Draw SDL grid
 *
 *---------------------------------------------------------------------------------------
 */
void draw_grid(RTPS_Window *window, double x_offset)
{
    // Draw vertical grid lines and X labels
    int plot_left   = PLOT_MARGIN_LEFT;
    int plot_right  = window->width - PLOT_MARGIN_RIGHT;
    int plot_top    = PLOT_MARGIN_TOP;
    int plot_bottom = window->height - PLOT_MARGIN_BOTTOM;
    int plot_width  = plot_right - plot_left;
    int plot_height = plot_bottom - plot_top;

    // Horizontal grid lines (X axis)
    double x_start_grid = ceil(x_offset / window->x_grid_step) * window->x_grid_step;

    for (double gx = x_start_grid; gx < x_offset + window->x_range; gx += window->x_grid_step)
    {
        int px = plot_left + (int)(((gx - x_offset) / window->x_range) * plot_width);
        thickLineRGBA(window->sdlrendr, px, plot_top, px, plot_bottom, 1, 200, 200, 200, 255);

        // Grid label
        char label[32];
        sprintf(label, "%.2f", gx);
        stringRGBA(window->sdlrendr, px - 10, plot_bottom + 5, label, 80, 80, 80, 255);
    }

    // Vertical grid lines (Y axis)
    for (double gy = ceil(window->y_min / window->y_grid_step) * window->y_grid_step; 
		    gy <= window->y_max; gy += window->y_grid_step)
    {
        int py = plot_top + (int)((window->y_max - gy) / (window->y_max - window->y_min) * plot_height);
        thickLineRGBA(window->sdlrendr, plot_left, py, plot_right, py, 1, 200, 200, 200, 255);
        // Grid label
        char label[16];
        sprintf(label, "%.1f", gy);
        stringRGBA(window->sdlrendr, plot_left - 35, py - 4, label, 80, 80, 80, 255);
    }
}


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		void draw_axes(RTPS_Window *window, double x_offset)
 *
 *  @brief	Draw SDL axes
 *
 *---------------------------------------------------------------------------------------
 */
static
void draw_axes(RTPS_Window *window, double x_offset)
{
    int plot_left   = PLOT_MARGIN_LEFT;
    int plot_right  = window->width - PLOT_MARGIN_RIGHT;
    int plot_top    = PLOT_MARGIN_TOP;
    int plot_bottom = window->height - PLOT_MARGIN_BOTTOM;
    int plot_width  = plot_right - plot_left;
    int plot_height = plot_bottom - plot_top;

    // Draw Y axis (y=0)
    if (window->y_min < 0 && window->y_max > 0)
    {
        int py = plot_top + (int)((window->y_max - 0) / (window->y_max - window->y_min) * plot_height);
        thickLineRGBA(window->sdlrendr, plot_left, py, plot_right, py, 2, 0, 0, 0, 255);
    }

#if 0
    // Draw X axis (x=0)
    if (x_offset < 0 && x_offset + window->x_range > 0)
    {
        int px = plot_left + (int)((0 - x_offset) / window->x_range * plot_width);
        thickLineRGBA(window->sdlrendr, px, plot_top, px, plot_bottom, 2, 0, 0, 0, 255);
    }
#endif    
}



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		void draw_title(RTPS_Window *window)
 *
 *  @brief	Draw SDL plot title
 *
 *---------------------------------------------------------------------------------------
 */
static
void draw_title(RTPS_Window *window)
{
    int title_x = window->width / 2 - (strlen(window->title) * 8) / 2;
    int title_y = 20;
    stringRGBA(window->sdlrendr, title_x, title_y, window->title, 0, 0, 0, 255);
}


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int draw_plot(SDL_Renderer *renderer, 
 *                            RTPS_Window *win, 
 *                            double x_offset) 
 *
 *  @brief	Draw SDL plot
 *
 *  @return	0 if success, negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int draw_plot(RTPS_Window *window, double x_offset)
{
    int tail = -1;
    DataPoint data1, data2;

    if (window == NULL) return -1;

    if (cb_empty(&window->cb)) return -2;

    int plot_left   = PLOT_MARGIN_LEFT;
    int plot_right  = window->width - PLOT_MARGIN_RIGHT;
    int plot_top    = PLOT_MARGIN_TOP;
    int plot_bottom = window->height - PLOT_MARGIN_BOTTOM;
    int plot_width  = plot_right - plot_left;
    int plot_height = plot_bottom - plot_top;

    tail = cb_peek_tail(&window->cb, tail, &data1);
    if (tail < 0) return -3;

    for (int i = 1; i < window->cb.count; ++i)
    {
       tail = cb_peek_tail(&window->cb, tail, &data2);

       if (data2.x < x_offset)
          continue;
       if (data1.x > x_offset + window->x_range)
          break;

       for (int j = 0; j < window->cb.y_count; j++)
       {
           int x1 = plot_left + (int)(((data1.x - x_offset) / window->x_range) * plot_width);
           int y1 = plot_top  + (int)((window->y_max- data1.y[j]) / (window->y_max - window->y_min) 
			                        * plot_height);
           int x2 = plot_left + (int)(((data2.x - x_offset) / window->x_range) * plot_width);
           int y2 = plot_top  + (int)((window->y_max - data2.y[j]) / (window->y_max - window->y_min) 
			                        * plot_height);

           if (x2 >= x1)
              thickLineRGBA(window->sdlrendr, x1, y1, x2, y2, 2, 
			    window->y_color[j].r, window->y_color[j].g, 
			    window->y_color[j].b, window->y_color[j].a);
       }
       data1 = data2;
    }
    return 0;
}



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		bool SDL_Exit()
 *
 *  @brief	Check if application should exit
 *
 *---------------------------------------------------------------------------------------
 */
bool SDL_Exit()
{
   SDL_Event e;
   while (SDL_PollEvent(&e))
      if (e.type == SDL_QUIT)
         return true;

   return false;
}


/*!
 *---------------------------------------------------------------------------------------
 *  
 *  @fn		int RTPS_wait_for_connection(int port)
 *  
 *  @brief	Initialize Real-Time Plot Server
 *
 *  @param	port	TCP port number
 *
 *  @return	positive client id if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
static 
int RTPS_wait_for_connection(int port) 
{
   int client = -1;

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
 *---------------------------------------------------------------------------------------
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
 *---------------------------------------------------------------------------------------
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
 *---------------------------------------------------------------------------------------
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
 *---------------------------------------------------------------------------------------
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
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_create(cJSON *root, RTPS_Window *window)
 *
 *  @brief	Create a new plot window from JSON
 *
 *  @param	root	cJSON root 
 *  @param	window	Pointer to instantiated RTPS_Window
 *
 *  @return	0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
static
int RTPS_create(cJSON *root, RTPS_Window *window)
{
   if (root == NULL) return -1;

   if (0 == RTPS_cjson_to_win(root, window))
   {
      // Init the Circular Buffer
      int max_points = (int)floor(window->x_range / window->x_step);  
      cb_init(&window->cb, MAX_Y_PLOTS, max_points);

      // Create & attach to SDL window/renderer
      window->sdlwin = SDL_CreateWindow(window->title,
                                        SDL_WINDOWPOS_CENTERED, // x pos
	                                SDL_WINDOWPOS_CENTERED, // y pos 
                                        window->width, 
					window->height, 
					SDL_WINDOW_SHOWN);

      window->sdlrendr = SDL_CreateRenderer(window->sdlwin, -1, SDL_RENDERER_ACCELERATED);
      return 0;
   }
   return -2;
}


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_plot(cJSON *root, DataPoint *data)
 *
 *  @brief	Plot a new point
 *
 *  return	 0 success
 *  		-1 root or data is NULL
 *  		-2 cannot find 'window' key
 *  		-3 cannot find 'data' key
 *		-4 cJSON to data conversion error
 * 		-5 specified window not found
 *
 *---------------------------------------------------------------------------------------
 */
static 
int RTPS_plot(cJSON *root, RTPS_Window *window)
{
   int rc = -5;
   cJSON *wdata;

   if (root == NULL || window == NULL) return -1;

   if ((wdata = cJSON_extract(root, 'a', "data")) == NULL)
      return -2;

   DataPoint data = {0};
   rc = RTPS_cjson_to_data(wdata, &data);
   if (rc < 0) return -3;	    	

   cb_push(&window->cb, data);
   double x_offset = data.x - window->x_range;

   SDL_SetRenderDrawColor(window->sdlrendr, 255, 255, 255, 255);
   SDL_RenderClear(window->sdlrendr);

   draw_grid(window, x_offset); 
   draw_axes(window, x_offset);
   rc = draw_plot(window, x_offset);
   if (rc != 0) printf("draw_plot returned rc=%d\n", rc);
   draw_title(window);

#if 1
   // Axis labels
   stringRGBA(window->sdlrendr,
              window->width/2 - 30, window->height - 35, 
              window->x_label, 
              0, 0, 0, 255);

   stringRGBA(window->sdlrendr,
	      10, window->height/2,
	      window->y_label, 
	      0, 0, 0, 255);
#endif

   SDL_RenderPresent(window->sdlrendr);
   SDL_Delay((int)window->x_step*1000);
   
   return rc;   // window not found
}



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn	 	main()
 *
 *  @brief	Main logic of Real-Time Plot Server
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
 *              }
 *
 *  @param      json_str        Incoming JSON string (one of the above)
 *
 *  @return     0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int main(int argc, char *argv[]) 
{
   int rc = 0;
   int port = 12345;
   int client;
   bool win_created = false;
   char message[MAX_JSON_LEN];
   cJSON *root, *cmd = NULL;


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
   SDL_Init(SDL_INIT_VIDEO);


   // Wait for client connection
   if ((client = RTPS_wait_for_connection(port)) < 0)
   {
      RTPS_perror("Wait for connection failed.");
      goto _err_ret;
   }
   printf("Server listening on port %d.\n", port);
   connected = true;


   while (connected)
   {
      // Check if SDL should exit
      if (SDL_Exit()) connected = false;

      // Receiving loop 
      memset(message, 0, sizeof(message));
      if (0 == RTPS_recv(client,  message, sizeof(message)))
      {
         printf("Processed message: %s\n", message);

         // Parse JSON string 
         if ((root = cJSON_Parse(message)) == NULL) return -1;

	 // Check for command
         if ((cmd = cJSON_extract(root, 's', "cmd")) != NULL)
         {
            // Parse and process different command type 
            if (0 == strcmp(cmd->valuestring, "create"))
            {
               if (win_created)
	       {
                  RTPS_perror("Window already created.");
		  rc = -2;
	          goto _err_ret;
	       }
	       else if ((rc = RTPS_create(root, &plotwin)) == 0)
	       {
                  win_created = true;
	       }
	       else
	       {
                  RTPS_perror("Window create.");
		  rc = -3;
	          goto _err_ret;
	       }
            }
            else if (0 == strcmp(cmd->valuestring, "plot"))
            {
               if (win_created)
	       {
                  rc = RTPS_plot(root, &plotwin);
                  printf("RTPS_plot() returned rc=%d\n", rc);
	       }
	       else
	       {
                  RTPS_perror("Window not created.");
		  rc = -4;
	          goto _err_ret;
	       }
            }
            else if (0 == strcmp(cmd->valuestring, "destroy"))
            {
	       goto _err_ret;
            }
            else
            {
               RTPS_perror("unrecognized command.");
               rc = -5;
	       goto _err_ret;
            }
	 }
	 else
	 {
            RTPS_perror("No cmd found.");
            rc = -6;
	    goto _err_ret;
	 }
      } // if (recv)
   } // while (connected)

_err_ret:
   SDL_DestroyRenderer(plotwin.sdlrendr);
   SDL_DestroyWindow(plotwin.sdlwin);
   SDL_Quit();
   return rc;
}
