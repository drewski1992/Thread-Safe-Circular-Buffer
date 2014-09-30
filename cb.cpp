#include "cb.h"

/* Create Circular Buffer with 'num_stages' number of
 * stages with each stage having a buffer of size 'size'
 */
cb::cb(int num_stages, size_t size) {

	c_num_stages = num_stages;
	c_data_size = size;
	
	c_buff = (CircualrBuffer_t*)malloc(sizeof(CircualrBuffer_t));
	
	/* Memory allocate data for the circular buffer's stages */
	/* Point the Circular Buffer at the Stages */
	c_buff->stages = (Stage_t*)malloc(sizeof(Stage_t)*c_num_stages);
	
	/* Point Each Stage in the Circular Buffer to a block of data */
	for(int i=0;i<c_num_stages;i++) {
		c_buff->stages[i].data = (void*)malloc(c_data_size);
	}
	
	
	init_cb();
}

/* Copy 'data' into a stage referenced by 'write_index */
int cb::write(void* data, int write_index) {
	pthread_mutex_lock(&c_buff->stages[write_index].lock);
	/* Wait for given stage to become empty */
	while(c_buff->stages[write_index].isfull == 1) {
		pthread_cond_wait(&c_buff->stages[write_index].empty, &c_buff->stages[write_index].lock);
	}
	memcpy(c_buff->stages[write_index].data, data, c_data_size);
	c_buff->stages[write_index].isempty = 0;
	c_buff->stages[write_index].isfull = 1;
	pthread_mutex_unlock(&c_buff->stages[write_index].lock);
	pthread_cond_signal(&c_buff->stages[write_index].full); /* Wake up sleeping thread waiting to read */
	
	return 1;
}

/* Copy data from stage referenced by 'read_index' into
 * 'data'
 */
int cb::read(void* data, int read_index) {
	pthread_mutex_lock(&c_buff->stages[read_index].lock);
	/* Wait for given stage to become full */
	while(c_buff->stages[read_index].isempty == 1) {
		pthread_cond_wait(&c_buff->stages[read_index].full, &c_buff->stages[read_index].lock);
	}
	memcpy(data, c_buff->stages[read_index].data, c_data_size);
	c_buff->stages[read_index].isempty = 1;
	c_buff->stages[read_index].isfull = 0;
	pthread_mutex_unlock(&c_buff->stages[read_index].lock);
	pthread_cond_signal(&c_buff->stages[read_index].empty); /* Wake up sleeping thread waiting to write */
	
	return 1;
}

cb::~cb() {
	
}

/* Returns next stage index to write new data. 
 *                    v
 * [ ][ ][x][x][x][x][ ][ ]
 */
int cb::write_spot(){
	int spot;
	pthread_mutex_lock(&c_buff->w_lock);
	spot = c_buff->w_i;
	/* Incriment the Write Index (w_i) */
	c_buff->w_i++;
	if(c_buff->w_i >= c_num_stages){
		c_buff->w_i = c_buff->w_i % c_num_stages;
	}
	pthread_mutex_unlock(&c_buff->w_lock);
	
	return spot;
}

/*  Returns next stage index to read new data. 
 *        v            
 * [ ][ ][x][x][x][x][ ][ ]
 */
int cb::read_spot(){
	int spot;
	pthread_mutex_lock(&c_buff->r_lock);
	spot = c_buff->r_i;
	/* Incriment the Read Index (r_i) */
	c_buff->r_i++;
	if(c_buff->r_i >= c_num_stages){
		c_buff->r_i = c_buff->r_i % c_num_stages;
	}
	pthread_mutex_unlock(&c_buff->r_lock);

	return spot;
}

int cb::init_cb() {
	c_buff->r_i = 0;
	c_buff->w_i = 0;
	pthread_mutex_init(&c_buff->r_lock,NULL);
	pthread_mutex_init(&c_buff->w_lock,NULL);
	for(int i=0;i<c_num_stages;i++) {
		pthread_cond_init(&c_buff->stages[i].empty, NULL);
		pthread_cond_init(&c_buff->stages[i].full, NULL);
		pthread_mutex_init(&c_buff->stages[i].lock, NULL);
		c_buff->stages[i].isempty = 1;
		c_buff->stages[i].isfull = 0;
	}
	return 1;
}
