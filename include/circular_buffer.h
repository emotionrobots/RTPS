/*!
 *---------------------------------------------------------------------------
 *
 * @file	circular_buffer.h
 *
 * @brief	Circular buffer header file
 *
 *---------------------------------------------------------------------------
 */
#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__
#include <global.h>

typedef struct {
   double x;
   double y[MAX_Y_PLOTS];
}
DataPoint;

typedef struct {
    DATA_TYPE *buffer;
    size_t head;
    size_t tail;
    size_t sz;
    size_t count;
    size_t y_count;
} CircularBuffer;

void cb_init(CircularBuffer *cb, size_t y_count, size_t size);
void cb_free(CircularBuffer *cb);
bool cb_push(CircularBuffer *cb, DATA_TYPE item);
bool cb_pull(CircularBuffer *cb, DATA_TYPE *item);
bool cb_empty(CircularBuffer *cb);
bool cb_full(CircularBuffer *cb); 
int cb_peek_head(CircularBuffer *cb, int curr_head, DATA_TYPE *item); 
int cb_peek_tail(CircularBuffer *cb, int curr_tail, DATA_TYPE *item); 
size_t cb_count(CircularBuffer *cb);
void cb_print(CircularBuffer *cb); 

#endif
