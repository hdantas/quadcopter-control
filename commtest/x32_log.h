#ifndef _X32_LOG_H
#define _X32_LOG_H

void log_start();
void log_stop();

void log_transmit();

void write_to_log(log_type type, unsigned char* data, int len);
void log_msg(const char* msg);
void log_data(log_type type, unsigned char* data, int len);
void log_event(log_type event);
void log_int(int value);
void log_byte(unsigned char c);
#endif
