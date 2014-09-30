#ifndef CB_H_
#define CB_H_

#include <stdlib.h>							/* malloc */
#include <string.h>							/* memcpy */
#include <pthread.h> 						/* Mutex and Conditions */

/* Circular Buffer Data Types */
typedef struct {
	pthread_cond_t 	empty, full;
	pthread_mutex_t lock;
	int 			isempty, isfull;
	void* 			data;
} Stage_t;

typedef struct {
	Stage_t* 		stages;
	int 			w_i, r_i;
	pthread_mutex_t w_lock;
	pthread_mutex_t r_lock;
} CircualrBuffer_t;

class cb {
	
public:
	cb(int num_stages, size_t size);		/* Create Circular Buffer with 'num_stages' number of
											 * stages with each stage having a data buffer of size 'size'
											 */
	
	int write(void* data, int write_index); /* Copy 'data' into a stage referenced by 'write_index */
	
	int read(void* data, int read_index); 	/* Copy data from stage referenced by 'read_index' into
											 * data
											 */
											
	int write_spot();						/* Returns next spot to write new data */
	
	int read_spot();						/* Returns next spot to read data */
	
	virtual ~cb();
	
	int c_num_stages; 						/* Number of stages inside the circular buffer */
	
	size_t c_data_size;						/* Size of data buffer in every stage */
	
private:
	int init_cb();							/* Initializes pthread types */
	
	CircualrBuffer_t* c_buff; 				/* Pointer to the Circular Buffer in memory */
};

#endif /*CB_H_*/
