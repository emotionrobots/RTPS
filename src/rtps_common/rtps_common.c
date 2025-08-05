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


