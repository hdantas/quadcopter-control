#ifndef _X32_LOG_H
#define _X32_LOG_H

/*	Author: Maurijn Neumann
 */

void log_start();
void log_stop();

void log_transmit();

void write_to_log(log_type type, unsigned char* data, int len);

#endif
