#ifndef _LOG_H
#define _LOG_H

#define LOG_TYPE_EVENT			0
#define LOG_TYPE_DATA_STATIC	32
#define LOG_TYPE_DATA_VAR		48
#define LOG_TYPE_ERROR			64
typedef enum {
	LOG_START						= LOG_TYPE_EVENT,
	LOG_STOP,
	LOG_TRANSMIT,

	LOG_RPYL						= LOG_TYPE_DATA_STATIC,
	LOG_BYTE,
	LOG_INT,
	LOG_VALUES,

	LOG_RECV_PACKET					= LOG_TYPE_DATA_VAR,
	LOG_SEND_PACKET,
	LOG_COMM_DISCARD_BUFFER,
	LOG_MSG,
	LOG_DEBUG,

	ERROR_EVENT 					= LOG_TYPE_ERROR,
	ERROR_ANOTHER
} log_type;

void log_data(log_type type, unsigned char* data, int len);
void log_msg(const char* msg);
//Shorthands
/*
#define log_event(event)	log_data(event, 0, 0)
#define log_int(value)		log_data(LOG_INT, (unsigned char*) &value, 4)
#define log_byte(value)		log_data(LOG_BYTE, &value, 1)
*/

void log_event(log_type event);
void log_int(int value);
void log_byte(unsigned char c);


#endif
