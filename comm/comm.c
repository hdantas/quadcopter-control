#include "comm.h"
#include "serial.h"
#include "checksum.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Global variables
unsigned char frame_buffer[FRAME_BUFFER_SIZE];
//int frame_buffer_base;
int frame_buffer_top;

frame_type await_frame_type;

//Initialize communication
int comm_init() {
	//Frame buffer
	frame_buffer_top = 0;

	await_frame_type = HEAD;

	//Prepare CRC lookup table
	crc_init();

	//Prepare connection
	if (serial_init() != 0)
		return -1;

	return 0;
}

void comm_uninit() {
	//Close serial connection
	serial_uninit();
}

int send_data(comm_type type, unsigned char* data, int len) {
	//Declare variables
	int expected_length;
	unsigned char* packet;
	unsigned char checksum;
	int I;

	//Verify length
	expected_length = data_length(type);
	if	((len < 0) ||
		((expected_length != -1) && (expected_length != len)))
		
		return -1;

	//Create blank packet
	packet = (unsigned char*) malloc(len+2);

	//Fill head
	packet[0] = 0x80 | (type & 0x3F);
	
	//Fill body
	for (I = 0; I < len; I++)
		packet[I+1] = data[I] & 0x7F;

	//Fill tail with blank checksum
	packet[len+1] = 0xC0;

	//Calculate packet checksum
	checksum = calc_checksum(packet, len+2);
	
	//Add checksum to tail
	packet[len+1] |= checksum;


	//Send packet byte-by-byte over serial connection
	for (I = 0; I < len+2; I++)
		if (-1 == serial_write(packet[I]))
			return -1;

	return 0;
}

int recv_data(comm_type* type, unsigned char** data, int* len) {
	//Declare variables
	unsigned char c;
	frame_type ftype;
	unsigned char processFlag;
	int I;

	//Retrieve data while it is available
	while (1 == serial_read(&c)) {
		//Assume packet will not need to be processed
		processFlag = 0;

		//Check type
		ftype = get_frame_type(c);
		//Validate heads & tails
		if (ftype == HEAD) {
			if (await_frame_type != HEAD) {
				//Missed end of previous packet, 
				//Empty buffer
				frame_buffer_top = 0;
			}
			//Wait for tail
			await_frame_type = TAIL;
		} else if (ftype == TAIL) {
			if (await_frame_type != TAIL) {
				//Unexpected end of packet
				//Empty buffer
				frame_buffer_top = 0;
				continue;

			} else {
				//Received tail when expecting: packet complete
				processFlag = 1;
			}
			//Wait for head
			await_frame_type = HEAD;
		} else if (ftype == BODY) {
			if (await_frame_type == HEAD) {
				//Unexpected body frame, ignore
				continue;
			}
		}

		//Add character to frame buffer
		frame_buffer[frame_buffer_top] = c;

		//Process if complete
		if (processFlag) {
			//Validate packet length
			if (frame_buffer_top-1 == data_length(frame_buffer[0] & 0x3F)) {
				//Verify packet using checksum
				if (verify_checksum(frame_buffer, frame_buffer_top+1)) {
					//Packet is valid

					//Dismantle and return values
					*type = frame_buffer[0] & 0x3F;
					*len = frame_buffer_top-1;
					*data = (unsigned char*) malloc(*len);
					memcpy(*data, &(frame_buffer[1]), *len);
					//Empty buffer, wait for head
					frame_buffer_top = 0;
					await_frame_type = HEAD;
					return 1;

				} else {
					//Invalid checksum
					
					//Empty buffer, wait for head
					frame_buffer_top = 0;
					await_frame_type = HEAD;
				}
			} else {
				//Invalid packet length

				//Empty buffer, wait for head
				frame_buffer_top = 0;
				await_frame_type = HEAD;
			} 
		} else {
			//Update buffer index
			frame_buffer_top++;
			if (frame_buffer_top >= FRAME_BUFFER_SIZE) {
				//Buffer is full

				//Empty buffer, wait for head
				frame_buffer_top = 0;
				await_frame_type = HEAD;
			}
		}
	}

	return 0;
}

//Frame crap
frame_type get_frame_type(unsigned char c) {
	if ((c & 0xC0) == 0x80)
		return HEAD;
	else if ((c & 0x80) == 0x00)
		return BODY;
	else if ((c & 0xC0) == 0xC0)
		return TAIL;
	else
		return NONE;

}

int data_length(comm_type type) {
	if (type >=	COMM_TYPE_SIZE_VAR)
		return -1;	//Undefined
	else if (type >= COMM_TYPE_SIZE_16)
		return 16;
	else if (type >= COMM_TYPE_SIZE_4)
		return 4;
	else if (type >= COMM_TYPE_SIZE_1)
		return 1;
	else if (type >= COMM_TYPE_SIZE_0)
		return 0;
	return -1;	//Should not happen
}
