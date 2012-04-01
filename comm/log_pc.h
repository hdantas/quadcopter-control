#ifndef _LOG_PC_H
#define _LOG_PC_H

/*	Author: Maurijn Neumann
 */

#include "log.h"

int retrieve_log();
void parse_log(unsigned char* logfile, int logsize);
const char* logtype_to_string(log_type type);

#endif
