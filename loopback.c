#include <stdio.h>
#include "x32_nexys.h"
#include "assert.h"

/* define some peripheral short hands
 */
#define X32_rs232_data		peripherals[PERIPHERAL_PRIMARY_DATA]
#define X32_rs232_stat		peripherals[PERIPHERAL_PRIMARY_STATUS]
#define X32_rs232_char		(X32_rs232_stat & 0x02)

// RX FIFO
#define FIFOSIZE 16
char	fifo[FIFOSIZE]; 
int	iptr, optr;

// Global vars
char	c;
int	demo_done;

// OUR PROTOCOL BUFFER
int headerbyte;


/*------------------------------------------------------------------
 * isr_rs232_rx -- rs232 rx interrupt handler
 * 		   dump in the same fifo as wireless link
 *------------------------------------------------------------------
 */
void isr_rs232_rx(void)
{
	int	c;

	/* signal interrupt
	 */

	/* may have received > 1 char before IRQ is serviced so loop
	 */
	while (X32_rs232_char) {
		fifo[iptr++] = X32_rs232_data;
		if (iptr >= FIFOSIZE)
			iptr = 0;
	}

}


/*------------------------------------------------------------------
 * getchar -- read char from rx fifo, return -1 if no char available
 *------------------------------------------------------------------
 */
int 	getchar(void)
{
	int	c;

	if (optr == iptr) {
		return -1;
	}
	c = fifo[optr++];
	if (optr >= FIFOSIZE) {
		optr = 0;
	}
	return c;
}

int parity(char ino)
{
	int noofones = 0;
	char mask = 0x01; /* start at first bit */

	while(mask != 0) /* until all bits tested */
	{
		if(mask & ino) /* if bit is 1, increment noofones */
		{
			noofones++;
		}
		mask = mask << 1; /* go to next bit */
	}

	/* if noofones is odd, least significant bit will be 1 */

	return (noofones & 1) == ((ino & 0x40) >> 6) ; 
	
	
}

int doubleparity(char c)
{
	unsigned int ino = (headerbyte << 8) | c;
	int noofones = 0;
	unsigned int mask = 0x01; /* start at first bit */

	while(mask < 0x100) /* until all bits tested */
	{
		if(mask & ino) /* if bit is 1, increment noofones */
		{
			noofones++;
		}
		mask = mask << 1; /* go to next bit */
	}

	/* if noofones is odd, least significant bit will be 1 */

	return (noofones & 1) == ((headerbyte & 0x40) >> 6); 
}

void source_identifier(int c) 
{	
	
	switch (c) 
	{
		case 0:	//ESC
			printf("ESC\n");
			break;
		case 1:	
			printf("RETURN\n");
			break;
		case 2:
			printf("0\n");
			break;
		case 3:
			printf("1\n");
			break;
		case 4:
			printf("2\n");
			break;
		case 5:
			printf("3\n");
			break;
		case 6:
			printf("4\n");
			break;
		case 7:
			printf("5\n");
			break;
		case 8:
			printf("a\n");
			break;
		case 9:
			printf("z\n");
			break;
		case 10:
			printf("RIGHT\n");
			break;
		case 11:
			printf("LEFT\n");
			break;
		case 12:
			printf("UP\n");
			break;
		case 13:
			printf("DOWN\n");
			break;
		case 14:
			printf("q\n");
			break;
		case 15:
			printf("w\n");
			break;
		case 16:
			printf("u\n");
			break;
		case 17:
			printf("j\n");
			break;
		case 18:
			printf("i\n");
			break;
		case 19:
			printf("k\n");
			break;
		case 20:
			printf("o\n");
			break;
		case 21:
			printf("l\n");
			break;
		case 60:
			printf("Roll\n");
			break;
		case 61:
			printf("Pitch\n");
			break;
		case 62:
			printf("Yaw\n");
			break;
		case 63:
			printf("Lift\n");
			break;			
		default:
			printf("key not mapped error\n");	//key not map
	}
	
	
}

int is_there_body(int ID){
	return ID>32;
}

void header_process(char c){
	int ID = c & 0x3F;
	if (is_there_body(ID)){
		headerbyte = c;
		return;
	}
	else {
	
		printf("\n***NEXYS***\n");
	
		headerbyte = 0;
		if(parity(c))
			printf("Parity ok!\n");
		else
			return	;	
		source_identifier(ID);		
	}
}

void body_process(char c) {
	int ID = headerbyte & 0x3F;
	int value = c & 0x7F;
	
	printf("\n***NEXYS***\n");
	
	if (headerbyte == 0) {
		printf("Error: Body without header!\n");
		return;
	}
	
	if (!doubleparity(c))
			printf("Parity ok!\n");
	else {
		printf("Error: Parity!\n");
		return;
	}
			
	
	source_identifier(ID);
	printf("value 0x%X\n",value);
	headerbyte = 0;
}

/*------------------------------------------------------------------
 * process_key -- process command keys
 *------------------------------------------------------------------
 */
void process_key(char c) 
{
	int source;
	peripherals[PERIPHERAL_DISPLAY] = c;
	
	if (c & 0x08)
		header_process(c);
	else
		body_process(c);
		
/*	if(!parity(c))
		printf("Parity ok!\n");
	if(!(c & 0x01))
		printf("single byte\n");
	source=c & 0x3F; //0x3F = 0011 1111 ie keep the source
	printf("source = 0X%X\t",source);
	source_identifier(source);*/
	
}

/*------------------------------------------------------------------
 * main -- do the test
 *------------------------------------------------------------------
 */
int main() 
{

	
	/* prepare rs232 rx interrupt and getchar handler
	 */
        SET_INTERRUPT_VECTOR(INTERRUPT_PRIMARY_RX, &isr_rs232_rx);
        SET_INTERRUPT_PRIORITY(INTERRUPT_PRIMARY_RX, 20);
	while (X32_rs232_char) c = X32_rs232_data; // empty buffer
        ENABLE_INTERRUPT(INTERRUPT_PRIMARY_RX);

	/* initialize remaining stuff
	 */
        iptr = optr = 0; // reset FIFO
	demo_done = 0;

	/* start the fun
	 */
        ENABLE_INTERRUPT(INTERRUPT_GLOBAL); 
	
	while (! demo_done) {

		/* process keyboard command (if any)
		 */
		c = getchar();
		if (c != -1) {
			process_key(c);
		}
	}

	printf("Exit\r\n");
        DISABLE_INTERRUPT(INTERRUPT_GLOBAL);
	return 0;
}

