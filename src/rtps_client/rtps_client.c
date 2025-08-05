/*!
 *=======================================================================================
 *
 * @file	rtps_client.c
 *
 * @brief	Test RTPS client main program
 *
 *=======================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <rtps.h>


// Default IP and port
#define PORT 12345
#define SERVER_IP "127.0.0.1"


char message[1024] = {0};


/*!
 *---------------------------------------------------------------------------------------
 *  Main logic
 *---------------------------------------------------------------------------------------
 */
int main() 
{
    RTPS_Window plotwin = {0};

    strcpy(plotwin.title, "y(t) = 2*cos(2*pi*f*t)");
    strcpy(plotwin.x_label, "t (sec)");
    strcpy(plotwin.y_label, "y");
    plotwin.width = 800;
    plotwin.height = 600;
    plotwin.y_count = 3;
    plotwin.max_points = 1024;
    plotwin.x_step = 0.01;
    plotwin.x_range = plotwin.x_step * (plotwin.max_points+1);
    plotwin.y_min = -2.0;
    plotwin.y_max =  2.0;
    plotwin.x_grid_step = 1.0;
    plotwin.y_grid_step = 0.5;

    plotwin.y_color[0].r = 255;
    plotwin.y_color[0].g =   0;
    plotwin.y_color[0].b =   0;
    plotwin.y_color[0].a = 255;

    plotwin.y_color[1].r =   0;
    plotwin.y_color[1].g = 255;
    plotwin.y_color[1].b =   0;
    plotwin.y_color[1].a = 255;

    plotwin.y_color[2].r =   0;
    plotwin.y_color[2].g =   0;
    plotwin.y_color[2].b = 255;
    plotwin.y_color[2].a = 255;


    // Create socket
    RTPS_Connection *conn = RTPS_connect(SERVER_IP, PORT);
    if (conn == NULL)
    {
       RTPS_perror("Client cannot connect to server.");
       return -2;
    }

    if (RTPS_client_create_plot(conn, &plotwin) < 0)
    {
       RTPS_perror("Client cannot connect to server.");
       return -3;
    }

    // Send messages
    bool done = false;
    DataPoint data = {0};
    double t = 0;
    double h = plotwin.x_step;
    double freq = 0.5;
    while (!done) 
    {
       for (int i=0; i < plotwin.y_count; i++)
          data.y[i] = (2.0 - 0.3*i) * cos(2.0*M_PI*freq*t);
       data.x = t;
       t += h;

       if (RTPS_client_send(conn, &plotwin, &data) < 0)
          RTPS_perror("RTPS_send() failed"); 

       usleep((int)(1000000*plotwin.x_step));
    }

    RTPS_disconnect(conn);
    return 0;
}
 
