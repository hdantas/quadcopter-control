#ifndef _COMM_H
#define _COMM_H

//Packet types
#define COMM_TYPE_SIZE_0	0
#define COMM_TYPE_SIZE_1	32
#define COMM_TYPE_SIZE_4	48
#define	COMM_TYPE_SIZE_16	60
#define	COMM_TYPE_SIZE_VAR	63

typedef enum {
	ACK			= COMM_TYPE_SIZE_0,			//0-byte sized body
	KEYQ,
	KEYA,
	KEYZ,
	KEYW,
	KEYE,
	KEYU,
	KEYJ,
	KEYI,
	KEYK,
	KEYO,
	KEYL,
	KEY0,
	KEY1,
	KEY2,
	KEY3,
	KEY4,
	KEY5,
	KEYRIGHT,
	KEYLEFT,
	KEYUP,
	KEYDOWN,
	KEYESC,
	KEYRETURN,
	REQLOG,

	// Modes
	SAFE,
	PANIC,
	MANUAL,
	CALIBRATION,
	YAW,
	FULL,


	SOMETHING		= COMM_TYPE_SIZE_1,			//1-byte sized body
	MODE			= COMM_TYPE_SIZE_1,

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

//typedef enum {
//	ACK				= COMM_TYPE_SIZE_0,			//0-byte sized body
//	KEY_Q				=1,
//	KEY_A				=2,
//	KEY_Z				=3,
//	KEY_W				=4,
//	KEY_E				=5,
//	KEY_U				=6,
//	KEY_J				=7,
//	KEY_I				=8,
//	KEY_K				=9,
//	KEY_O				=10,
//	KEY_L				=11,
//	KEY_0				=12,
//	KEY_1				=13,
//	KEY_2				=14,
//	KEY_3				=15,
//	KEY_4				=16,
//	KEY_5				=17,
//	KEY_RIGHT			=18,
//	KEY_LEFT			=19,
//	KEY_UP				=20,
//	KEY_DOWN			=21,
//	KEY_ESC				=22,
//	KEY_RETURN			=23,
//	REQ_LOG				=24


//	SOMETHING		= COMM_TYPE_SIZE_1,			//1-byte sized body

//	RPYL 			= COMM_TYPE_SIZE_4,			//4-byte sized body
//	REQ_LOG_CHUNK		=49,
//	LOG_SIZE		=50,

//	LOG_CHUNK 		= COMM_TYPE_SIZE_16,		//16-byte sized body

//	RANDOM_DATA		= COMM_TYPE_SIZE_VAR		//variable sized body
//} comm_type;
