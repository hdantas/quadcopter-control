#ifndef _X32_LOG_H
#define _X32_LOG_H

#define LOG_TYPE_EVENT			0
#define LOG_TYPE_DATA_STATIC		32
#define LOG_TYPE_DATA_VAR		48
#define LOG_TYPE_ERROR			64

typedef enum {
	LOG_START		= LOG_TYPE_EVENT,
	LOG_STOP,
	LOG_SEND,

	LOG_RPYL		= LOG_TYPE_DATA_STATIC,

	LOG_MSG			= LOG_TYPE_DATA_VAR,


	ERROR_EVENT		= LOG_TYPE_ERROR,
	ERROR_ANOTHER
} log_type;


void log_start(void);
void log_stop(void);
void log_retrieve(void);
void log_write(char* data);
#endif
