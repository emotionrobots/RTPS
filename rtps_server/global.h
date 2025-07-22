/*!
 *---------------------------------------------------------------------------
 *
 * @file	global.h
 *
 * @brief	Global header file
 *
 *---------------------------------------------------------------------------
 */
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define DATA_TYPE 		DataPoint
#define MAX_Y_COUNT		4

typedef struct {
   double x;
   double y[MAX_Y_COUNT];
} 
DataPoint;

typedef struct {
   int r; 
   int g; 
   int b;
   int a;
}
DataColor;

#endif
