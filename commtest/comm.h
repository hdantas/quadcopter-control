#ifndef _COMM_H
#define _COMM_H

//Packet types
#define COMM_TYPE_SIZE_0	0
#define COMM_TYPE_SIZE_1	32
#define COMM_TYPE_SIZE_4	48
#define	COMM_TYPE_SIZE_16	60
#define	COMM_TYPE_SIZE_VAR	63
typedef enum {
	ACK				= COMM_TYPE_SIZE_0,			//0-byte sized body
	KEY_Q,
	KEY_W,
	KEY_E,
	KEY_ESC,
	REQ_LOG,

	SOMETHING		= COMM_TYPE_SIZE_1,			//1-byte sized body

	RPYL 			= COMM_TYPE_SIZE_4,			//4-byte sized body
	REQ_LOG_CHUNK,
	LOG_SIZE,

	LOG_CHUNK 		= COMM_TYPE_SIZE_16,		//16-byte sized body

	RANDOM_DATA		= COMM_TYPE_SIZE_VAR		//variable sized body
} comm_type;

//High level functions
int comm_init();
void comm_uninit();
int send_data(comm_type type, unsigned char* data, int len);
int recv_data(comm_type* type, unsigned char** data, int* len);


//Frame helpers
#define FRAME_BUFFER_SIZE	32
typedef enum {
	NONE,
	HEAD,
	BODY,
	TAIL
} frame_type;

frame_type get_frame_type(unsigned char c);
int data_length(comm_type type);


#endif
