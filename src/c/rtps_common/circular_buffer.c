#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <circular_buffer.h>


//-------------------------------------------------------------------------
// Initialize the buffer
//-------------------------------------------------------------------------
void cb_init(CircularBuffer *cb, size_t y_count, size_t sz) 
{
    cb->buffer = malloc(sizeof(DATA_TYPE) * (sz+2));
    cb->sz = sz;
    cb->count = 0;
    cb->y_count = y_count;
    cb->head = 0;
    cb->tail = 0;
}


//-------------------------------------------------------------------------
// Free the buffer
//-------------------------------------------------------------------------
void cb_free(CircularBuffer *cb) 
{
    free(cb->buffer);
}


//-------------------------------------------------------------------------
//   Return next tail index
//-------------------------------------------------------------------------
static int cb_next_head(CircularBuffer *cb, int head)
{
    return (head >= cb->sz-1) ? 0 : (head + 1); 
}
   

//-------------------------------------------------------------------------
//   Return next tail index
//-------------------------------------------------------------------------
static int cb_prev_head(CircularBuffer *cb, int head)
{
    return (head <= 0) ? cb->sz-1 : (head - 1); 
}

//-------------------------------------------------------------------------
//   Return next tail index
//-------------------------------------------------------------------------
static int cb_next_tail(CircularBuffer *cb, int tail)
{
    return (tail >= cb->sz-1) ? 0 : (tail + 1);
    // return (tail + 1) % cb->count;
}


//-------------------------------------------------------------------------
// Add item to buffer 
// Returns true if overwrite there was overflow 
//-------------------------------------------------------------------------
bool cb_push(CircularBuffer *cb, DATA_TYPE item) 
{
    bool is_full = cb_full(cb);

    cb->buffer[cb->head] = item;
    cb->head = cb_next_head(cb, cb->head);
    if (!is_full) 
       cb->count = cb->count+1;
   
    return is_full;
}


//-------------------------------------------------------------------------
// Remove item from buffer (FIFO)
// Returns true item is valid, false otherwise 
//-------------------------------------------------------------------------
bool cb_pull(CircularBuffer *cb, DATA_TYPE *item) 
{
    // Checkif buffer is empty
    if (cb_empty(cb)) 
        return false;
   
    *item = cb->buffer[cb->tail];
    cb->tail = cb_next_tail(cb, cb->tail);
    cb->count = cb->count-1;

    return true;
}


//-------------------------------------------------------------------------
// Peek next item to starting from buffer head
// Returns head index, -1 if invalid  
// Set curr_head to -1 to get current head 
//-------------------------------------------------------------------------
int cb_peek_head(CircularBuffer *cb, int curr_head, DATA_TYPE *item) 
{

    // Checkif buffer is empty
    if (cb_empty(cb))
        return -1;

    int head = (curr_head < 0) ? cb_prev_head(cb, cb->head) : cb_prev_head(cb, curr_head); 
    *item = cb->buffer[head];
    return head;
}


//-------------------------------------------------------------------------
// Peek item from buffer tail 
// Returns tail index, -1 if invalid  
// SEt curr_tail to -1 to get current tail
//-------------------------------------------------------------------------
int cb_peek_tail(CircularBuffer *cb, int curr_tail, DATA_TYPE *item) 
{
    // Checkif buffer is empty
    if (cb_empty(cb))
        return -1;
   
    int tail = (curr_tail < 0) ? cb->tail : cb_next_tail(cb, curr_tail);
    *item = cb->buffer[tail];
    return tail;
}


//-------------------------------------------------------------------------
// Check if buffer is empty
//-------------------------------------------------------------------------
bool cb_empty(CircularBuffer *cb) 
{
    return (cb->count <= 0);
}



//-------------------------------------------------------------------------
// Check if buffer is full
//-------------------------------------------------------------------------
bool cb_full(CircularBuffer *cb) 
{
    return cb->count >= cb->sz;
}


//-------------------------------------------------------------------------
// Number of elements in buffer
//-------------------------------------------------------------------------
size_t cb_count(CircularBuffer *cb) 
{
    return cb->count;
}


//-------------------------------------------------------------------------
// Print buffer contents (for debugging)
//-------------------------------------------------------------------------
void cb_print(CircularBuffer *cb) 
{
    DataPoint data;
    size_t size = cb_count(cb);

    printf("Buffer contents (%ld): \n", size);
    int tail = -1;
    for (size_t i = 0; i < size; i++) 
    {
	tail = cb_peek_tail(cb, tail, &data); 
        printf("tail=%d, x=%g ", tail, data.x);
	for (int j=0; j < cb->y_count; j++)
           printf("y[%d]=%g ", j, data.y[j]);
        printf("\n");
    }
    printf("\n");
}

//#define UNIT_TEST

#ifdef UNIT_TEST 

#define BUFFER_SIZE		6

CircularBuffer cb;
DataPoint data; 
DataPoint head_data, tail_data;

//-------------------------------------------------------------------------
//  main()
//-------------------------------------------------------------------------
int main() 
{
    cb_init(&cb, 2, BUFFER_SIZE);

    // Test: Push BUFFER_SIZE elements
    for (int i = 0; i < BUFFER_SIZE; i++) 
    {
        data.x = (double)i;
        data.y[0] = sin(data.x);
        data.y[1] = cos(data.x);	
        cb_push(&cb, data);
        cb_print(&cb);
    }
    printf("head:%ld  tail=%ld\n", cb.head, cb.tail); 


    printf("Oldest first=================: \n"); 
    int tail = -1;
    for (int i = 0; i < BUFFER_SIZE; i++) 
    {
        tail = cb_peek_tail(&cb, tail, &tail_data);
        printf("%g, %g, %g\n", tail_data.x, tail_data.y[0], tail_data.y[1]);
    }


    printf("Newest first=================: \n"); 
    int head = -1;
    for (int i = 0; i < BUFFER_SIZE; i++) 
    {
        head = cb_peek_head(&cb, head, &head_data);
        printf("%d, %g, %g, %g\n", head, head_data.x, head_data.y[0], head_data.y[1]);
    }


#if 1 
    // Test: Buffer full, next push should overwrite oldest
    printf("Pushing 99.9 into full buffer (should overwrite oldest)\n");
    data.x = (double)101;
    data.y[0] = data.x+1;
    data.y[1] = data.x+2;	
    cb_push(&cb, data);
    cb_print(&cb);

    // Test: Pop all elements
    DataPoint val;
    printf("Pulling elements:\n");
    while (cb_pull(&cb, &val)) 
    {
        printf("Pulled: x=%g ", val.x);
	for (int i=0; i < cb.y_count; i++)
           printf("y[%d]=%g ", i, val.y[i]);
	printf("\n");
        cb_print(&cb);
    }

    // Test: Pull from empty buffer
    if (!cb_pull(&cb, &val)) 
    {
        printf("Pull failed, buffer is empty.\n");
    }
#endif

    cb_free(&cb);

    return 0;
}
#endif // UNIT_TEST
