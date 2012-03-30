#include "comm.h"
#include "serial.h"
#include "checksum.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Global variables
unsigned char frame_buffer[FRAME_BUFFER_SIZE];
//int frame_buffer_base;
int frame_buffer_top;

frame_type await_frame_type;

/*
	Initialise everything that has to do with communication
*/
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

/*	
	Uninitialise everything that had to do with communication
*/
void comm_uninit() {
	//Close serial connection
	serial_uninit();
}

/*
	Transmit data over the serial link
	Input:
		type	packet type (see comm.h)
		data	pointer to data buffer
		len		length of data buffer
	Returns:
		-1	on error
		0	on success
*/
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

	//Log
	log_data(LOG_SEND_PACKET, packet, len+2);

	//Send packet byte-by-byte over serial connection
	for (I = 0; I < len+2; I++) {
		if (-1 == serial_write(packet[I])) {
			free(packet);
			return -1;
		}
	}
			
	//Deallocate memory
	free(packet);

	return 0;
}

/*
	Shorthand to send a string (max length: 16)
*/
int send_text(const char* text) {
	//Declare Variables
	int len;
	unsigned char buffer[16];
	
	//Fetch length
	len = strlen(text);
	//Copy text to buffer, fixing length
	if (len >= 16) {
		memcpy(buffer, text, 16);
	} else {
		memcpy(buffer, text, len);
		buffer[len] = 0;
	}
	//Send
	return send_data(TEXT_CHUNK, buffer, 16);
}

/*
	Receive data using the serial link
	Input:
		type	pointer
		data	pointer
		len		pointer
	Output:
		*type	type of packet (see comm.h)
		*data	packet data buffer (dynamically allocated, needs to be free'd)
		*len	length of dynamically allocated data buffer
	Returns:
		-1	on error
		0	if no data was available
		1	if data was available and read
*/
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
				log_msg("Unexpected frame type: head; discarding frame buffer");
				log_byte(c);
				log_data(LOG_COMM_DISCARD_BUFFER, frame_buffer, frame_buffer_top);
				//Empty buffer
				frame_buffer_top = 0;
			}
			//Wait for tail
			await_frame_type = TAIL;
		} else if (ftype == TAIL) {
			if (await_frame_type != TAIL) {
				//Unexpected end of packet
				log_msg("Unexpected frame type: tail; discarding frame and frame buffer");
				log_byte(c);
				log_data(LOG_COMM_DISCARD_BUFFER, frame_buffer, frame_buffer_top);
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
				log_byte(c);
				log_msg("Unexpected frame type: body; discarding single frame");
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
					log_data(LOG_RECV_PACKET, frame_buffer, frame_buffer_top+1);

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
					log_msg("Invalid checksum; discarding frame buffer");
					log_data(LOG_COMM_DISCARD_BUFFER, frame_buffer, frame_buffer_top+1);
					
					//Empty buffer, wait for head
					frame_buffer_top = 0;
					await_frame_type = HEAD;
				}
			} else {
				//Invalid packet length
				log_msg("Invalid packet length; discarding frame buffer");
				log_data(LOG_COMM_DISCARD_BUFFER, frame_buffer, frame_buffer_top+1);

				//Empty buffer, wait for head
				frame_buffer_top = 0;
				await_frame_type = HEAD;
			} 
		} else {
			//Update buffer index
			frame_buffer_top++;
			if (frame_buffer_top >= FRAME_BUFFER_SIZE) {
				//Buffer is full
				log_msg("Frame buffer full; discarding");
				log_data(LOG_COMM_DISCARD_BUFFER, frame_buffer, frame_buffer_top);

				//Empty buffer, wait for head
				frame_buffer_top = 0;
				await_frame_type = HEAD;
			}
		}
	}

	return 0;
}

//Frame crap
/*
	Translate frame type from binary values into clear enumerations
*/
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

/*
	Calculate packet data length based on packet type
*/
int data_length(comm_type type) {
	if (type >=	COMM_TYPE_SIZE_VAR)
		return -1;	//Undefined
	else if (type >= COMM_TYPE_SIZE_16)
		return 16;
	else if (type >= COMM_TYPE_SIZE_8)
		return 8;
	else if (type >= COMM_TYPE_SIZE_4)
		return 4;
	else if (type >= COMM_TYPE_SIZE_1)
		return 1;
	else if (type >= COMM_TYPE_SIZE_0)
		return 0;
	return -1;	//Should not happen
}
