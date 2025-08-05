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
 *  @fn         cJSON *cJSON_extract(cJSON *root, char typ, const char *key)
 *
 *  @brief      Extract cJSON object with type check
 *
 *  @param      root    Root JSON structure
 *  @param      typ     type character 's', 'n' or 'a'
 *  @param      key     JSON key to extract
 *
 *  @return     Pointer to the extracted JSON item corresponding to 'key'
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
 *  @fn         void draw_grid(RTPS_Window *window, double x_offset)
 *
 *  @brief      Draw SDL grid
 *
 *---------------------------------------------------------------------------------------
 */
static
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
 *  @fn         void draw_title(RTPS_Window *window)
 *
 *  @brief      Draw SDL plot title
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
 *  @fn         void draw_axes(RTPS_Window *window, double x_offset)
 *
 *  @brief      Draw SDL axes
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
    int plot_height = plot_bottom - plot_top;

    // Draw Y axis (y=0)
    if (window->y_min < 0 && window->y_max > 0)
    {
        int py = plot_top + (int)((window->y_max - 0) / (window->y_max - window->y_min) * plot_height);
        thickLineRGBA(window->sdlrendr, plot_left, py, plot_right, py, 2, 0, 0, 0, 255);
    }

#if 0
    int plot_width  = plot_right - plot_left;

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
 *  @fn         int draw_plot(SDL_Renderer *renderer, 
 *                            RTPS_Window *win, 
 *                            double x_offset) 
 *
 *  @brief      Draw SDL plot
 *
 *  @return     0 if success, negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
static
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

    for (int i = 1; i < window->cb.count; i++)
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
 *  @fn         int RTPS_server_recv(RTPS_Connection *conn, char *message, size_t sz)
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
int RTPS_server_recv(int client, char *message, size_t sz)
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
 *  @fn		int RTPS_cjson_to_win(cJSON *root, RTPS_Window *win)
 *
 *  @brief	Convert cJSON to RTPS_Window object
 *
 *  @param	root		JSON root
 *  @param	win		Instantiated RTPS_Window object
 *
 *  @return	0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
static
int RTPS_cjson_to_win(cJSON *root, RTPS_Window *win)
{
   cJSON *title, *xl, *yl, *w, *h, *yc, *xr, *xs, *ymin, *ymax, *ycolor;

   if (win == NULL || root == NULL) return -1;

   if ((title = cJSON_extract(root, 's', "title")) == NULL) return -3;
   memset(win->title, 0, sizeof(win->title));
   strncpy(win->title,  title->valuestring, sizeof(win->title)-1);

   if ((xl = cJSON_extract(root, 's', "x_label")) == NULL) return -4;
   memset(win->x_label, 0, sizeof(win->x_label));
   strncpy(win->x_label,  xl->valuestring, sizeof(win->x_label)-1);

   if ((yl = cJSON_extract(root, 's', "y_label")) == NULL) return -5;
   memset(win->y_label, 0, sizeof(win->y_label));
   strncpy(win->y_label,  yl->valuestring, sizeof(win->y_label)-1);

   if ((w = cJSON_extract(root, 'n', "width")) == NULL) return -6;
   win->width = w->valueint;

   if ((h = cJSON_extract(root, 'n', "height")) == NULL) return -7;
   win->height = h->valueint;

   if ((yc = cJSON_extract(root, 'n', "y_count")) == NULL) return -8;
   win->y_count = yc->valueint;

   if ((xs = cJSON_extract(root, 'n', "x_step")) == NULL) return -9;
   win->x_step = xs->valuedouble;

   if ((xs = cJSON_extract(root, 'n', "x_grid_step")) == NULL) return -10;
   win->x_grid_step = xs->valuedouble;

   if ((xs = cJSON_extract(root, 'n', "y_grid_step")) == NULL) return -11;
   win->y_grid_step = xs->valuedouble;

   if ((xr = cJSON_extract(root, 'n', "x_range")) == NULL) return -12;
   win->x_range = xr->valuedouble;

   if ((ymin = cJSON_extract(root, 'n', "y_min")) == NULL) return -13;
   win->y_min = ymin->valuedouble;

   if ((ymax   = cJSON_extract(root, 'n', "y_max")) == NULL) return -14;
   win->y_max = ymax->valuedouble;

   if ((ycolor = cJSON_extract(root, 'a', "y_color")) == NULL) return -15;

   int k = 0;
   cJSON *color;

   cJSON_ArrayForEach(color, ycolor)
   {   
      cJSON *r, *g, *b, *a;
      if (k >= MAX_Y_PLOTS) break;

      if ((r = cJSON_extract(color, 'n', "r")) != NULL)
         win->y_color[k].r = r->valueint;

      if ((g = cJSON_extract(color, 'n', "g")) != NULL)
         win->y_color[k].g = g->valueint;

      if ((b = cJSON_extract(color, 'n', "b")) != NULL)
         win->y_color[k].b = b->valueint;

      if ((a = cJSON_extract(color, 'n', "a")) != NULL)
         win->y_color[k].a = a->valueint;

      k++;
   }

   return 0;
}


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_win_to_cjson(RTPS_Window *win, cJSON *root)
 *
 *  @brief	Converting RTPS_Window object to cJSON object
 *   
 *  @param	root		JSON root
 *  @param	win		Instantiated RTPS_Window object
 *
 *  @return	0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
static
int RTPS_win_to_cjson(RTPS_Window *win, cJSON *root)
{
   if (win == NULL || root == NULL) return -1;

   cJSON_AddStringToObject(root, "cmd", "create");
   cJSON_AddStringToObject(root, "title", win->title);
   cJSON_AddStringToObject(root, "x_label", win->x_label);
   cJSON_AddStringToObject(root, "y_label", win->y_label);
   cJSON_AddNumberToObject(root, "width", win->width);
   cJSON_AddNumberToObject(root, "height", win->height);
   cJSON_AddNumberToObject(root, "y_count", win->y_count);
   cJSON_AddNumberToObject(root, "x_step", win->x_step);
   cJSON_AddNumberToObject(root, "x_range", win->x_range);
   cJSON_AddNumberToObject(root, "y_min", win->y_min);
   cJSON_AddNumberToObject(root, "y_max", win->y_max);
   cJSON_AddNumberToObject(root, "x_grid_step", win->x_grid_step);
   cJSON_AddNumberToObject(root, "y_grid_step", win->y_grid_step);

   cJSON *ycolor = cJSON_CreateArray();
   for (int i = 0; i < win->y_count; i++)
   {
      cJSON *color = cJSON_CreateObject();
      cJSON_AddNumberToObject(color, "r", win->y_color[i].r);   
      cJSON_AddNumberToObject(color, "g", win->y_color[i].g);   
      cJSON_AddNumberToObject(color, "b", win->y_color[i].b);   
      cJSON_AddNumberToObject(color, "a", win->y_color[i].a);   
      cJSON_AddItemToArray(ycolor, color); 
   }
   cJSON_AddItemToObject(root, "y_color", ycolor);

   return 0;
}




/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_cjson_to_data(cJSON *root, DataPoint *data)
 *
 *  @brief	Extract cJSON into DataPoint object
 *
 *  @param	root		root cJSON
 *  @param	data		DataPoint pointer
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
static
int RTPS_cjson_to_data(cJSON *root, DataPoint *dat)
{
   cJSON *y;

   if (dat == NULL || root == NULL) return -1;

   int k = 0;
   cJSON_ArrayForEach(y, root)
   {  
      if (k == 0)
         dat->x = y->valuedouble;
      else
         dat->y[k-1] = y->valuedouble;
      k++;
   }

   return 0;
}


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_data_to_cjson(DataPoint *dat, cJSON *root, int sz)
 *
 *  @brief	Convert DataPoint to equivalent cJSON
 *
 *  @param	dat	Pointer to the DataPoint object
 *  @param	root	Pointer to already-created cJSON object
 *  @param	sz	Size of y array
 *
 *  @return	0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
static
int RTPS_data_to_cjson(DataPoint *dat, cJSON *root, RTPS_Window *win)
{
   if (dat == NULL || root == NULL || win == NULL) return -1;

   cJSON_AddStringToObject(root, "cmd", "plot");

   cJSON *yarray = cJSON_CreateArray();
   cJSON_AddItemToArray(yarray, cJSON_CreateNumber(dat->x)); 
   for (int i=0; i < win->y_count; i++)
   {
      cJSON_AddItemToArray(yarray, cJSON_CreateNumber(dat->y[i])); 
   }
   cJSON_AddItemToObject(root, "data", yarray);

   return 0;
}



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
 *  @fn		int RTPS_client_send(RTPS_Connection *conn, 
 *                                   RTPS_Window *win, 
 *                                   DataPoint *data)
 *
 *  @brief	Send a DataPoint 
 *
 *  @param	conn		RTPS_Connection
 *  @param	win		Pointer to RTPS_Window 
 *  @param	data		Pointer to DataPoint
 *
 *  @return	0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_client_send(RTPS_Connection *conn, RTPS_Window *win, DataPoint *data)
{
   if (conn == NULL || win == NULL || data == NULL) return -1;

   cJSON *cdata = cJSON_CreateObject();
   if (cdata == NULL) return -2;

   if (0 == RTPS_data_to_cjson(data, cdata, win))
   {
      char message[MAX_JSON_LEN] = {0};
      memset(message, 0, sizeof(message));
      char *json_str = cJSON_Print(cdata);
      if (json_str != NULL)
      {
         strcpy(message, json_str);
         free(json_str);
         send(conn->fd, message, strlen(message), 0);
      }
   }
   cJSON_Delete(cdata);
   return 0;
}



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_client_create_plot(RTPS_Connection *conn, RTPS_Window *plot)
 *  
 *  @brief	Initialize plot
 *
 *  @param	conn		Instantiated RTPS_Connection pointer
 *  @param	plot		Instantiated RTPS_Window pointer
 *
 *  @return	0 if success; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_client_create_plot(RTPS_Connection *conn, RTPS_Window *plot)
{
   int rc = -1;
   if (conn == NULL || plot == NULL) goto _err_ret; 

   cJSON *root = cJSON_CreateObject();
   if (root != NULL)
   {
      if (0 == RTPS_win_to_cjson(plot, root))
      {
         char *json_str = cJSON_Print(root);
         if (json_str != NULL)
         {
            char message[MAX_JSON_LEN];
            memset(message, 0, sizeof(message));
            strcpy(message, json_str);
            send(conn->fd, message, strlen(message), 0);
            free(json_str);
	    rc = 0;
         }
      }
      cJSON_Delete(root);
   }

_err_ret:
   return rc;
}



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
int RTPS_server_create(cJSON *root, RTPS_Window *window)
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
 *  @fn		void RTPS_server_init()
 *  
 *  @brief	Initialize RTPS server
 *
 *---------------------------------------------------------------------------------------
 */
void RTPS_server_init()
{
   SDL_Init(SDL_INIT_VIDEO);
}


/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_server_shutdown(RTPS_Window *win)
 *  
 *  @brief	RTPS server shutdown
 *
 *  @param	win	RTPS_Window pointer 
 *
 *  @return	0 if successful; negative otherwise
 *
 *---------------------------------------------------------------------------------------
 */
int RTPS_server_shutdown(RTPS_Window *win)
{
   if (win == NULL) return -1;

   SDL_DestroyRenderer(win->sdlrendr);
   SDL_DestroyWindow(win->sdlwin);
   SDL_Quit();

   return 0;
}

/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		bool RTPS_server_forced_exit()
 *
 *  @return	true if forced exit
 *
 *---------------------------------------------------------------------------------------
 */
bool RTPS_server_forced_exit()
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
int RTPS_wait_for_connection(RTPS_Connection *conn, int port)
{
   conn->connected = false;
   conn->fd = socket(AF_INET, SOCK_STREAM, 0);
   if (conn->fd < 0)
   {
     RTPS_perror("Socket failed.");
     goto _err_ret;
   }

   conn->port = port;
   conn->address.sin_family = AF_INET;
   conn->address.sin_addr.s_addr = INADDR_ANY;
   conn->address.sin_port = htons(conn->port);

   if (bind(conn->fd, (struct sockaddr*)&conn->address, sizeof(conn->address)) < 0)
   {
      RTPS_perror("Bind failed");
      goto _err_ret;
   }

   int addrlen = sizeof(conn->address);
   if (listen(conn->fd, 1) < 0)
   {
      RTPS_perror("Listen failed");
      goto _err_ret;
   }
   conn->client = accept(conn->fd, (struct sockaddr*)&conn->address, (socklen_t*)&addrlen);
   return 0;

_err_ret:
   RTPS_disconnect(conn);
   return -1;
}



/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn         int RTPS_plot(cJSON *root, DataPoint *data)
 *
 *  @brief      Plot a new point
 *
 *  return       0 success
 *              -1 root or data is NULL
 *              -2 cannot find 'window' key
 *              -3 cannot find 'data' key
 *              -4 cJSON to data conversion error
 *              -5 specified window not found
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
   draw_plot(window, x_offset);
   draw_title(window);

   // Axis labels
   stringRGBA(window->sdlrendr,
              window->width/2 - 30, window->height - 35,
              window->x_label,
              0, 0, 0, 255);

   stringRGBA(window->sdlrendr,
              10, window->height/2,
              window->y_label,
              0, 0, 0, 255);

   SDL_RenderPresent(window->sdlrendr);
   SDL_Delay((int)window->x_step*1000);

   return rc;   // window not found
}




/*!
 *---------------------------------------------------------------------------------------
 *
 *  @fn		int RTPS_server_update(RTPS_Connection *conn)
 *
 *  @brief	RTPS server loop update
 *
 *  @param	conn	RTPS_Connection pointer 
 *  @param	win	RTPS_Window pointer
 *
 *  @return	0 if successful; negative otherwise
 *  
 *---------------------------------------------------------------------------------------
 */
int RTPS_server_update(RTPS_Connection *conn, RTPS_Window *win)
{
   int rc = -1;	
   static bool win_created = false;
   cJSON *root, *cmd;
   char message[MAX_JSON_LEN];

   if (conn == NULL || win == NULL) goto _err_ret;

   memset(message, 0, sizeof(message));
   if (0 == RTPS_server_recv(conn->client,  message, sizeof(message)))
   {
      // Parse JSON string 
      if ((root = cJSON_Parse(message)) == NULL) goto _err_ret;

      // Check for command
      if ((cmd = cJSON_extract(root, 's', "cmd")) == NULL) goto _err_ret;
      
      // Parse and process different command type 
      if (0 == strcmp(cmd->valuestring, "create"))
      {
         if (win_created)
         {
            RTPS_perror("Window already created.");
         }
         else if ((rc = RTPS_server_create(root, win)) == 0)
         {
            win_created = true;
         }
         else
         {
            RTPS_perror("Window create.");
         }
      }
      else if (0 == strcmp(cmd->valuestring, "plot"))
      {
         if (win_created)
         {
            rc = RTPS_plot(root, win);
         }
         else
         {
            RTPS_perror("Window not created.");
         }
      }
      else if (0 == strcmp(cmd->valuestring, "destroy"))
      {
         rc = 0;
      }
      else
      {
         RTPS_perror("unrecognized command.");
      }
   } // if (recv)

_err_ret:
   return rc;   
}

