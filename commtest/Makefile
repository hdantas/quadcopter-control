#VPATH = commtest:control

#     foo.o : foo.c defs.h hack.h
#             cc -c $(CFLAGS) $^ -o $@
             
# $@ : The file name of the target of the rule.
# $< : The name of the first prerequisite.
# $^ : The names of all the prerequisites, with spaces between them

#######################  COMMUNICATION  #######################	
PCSOURCES = main.c serial.c log.c comm.c checksum.c convert.c
PCOBJECTS = $(PCSOURCES:.c=.o)

X32SOURCES = x32_main.c x32_serial.c x32_log.c comm.c checksum.c convert.c
X32OBJECTS = $(X32SOURCES:.c=.co)
 
all: pc qr

pc: pc_ pccomm

qr: qr_ qrcomm
	

pc_ : $(PCSOURCES)
	gcc -c $(PCSOURCES) -Wall
	
pccomm : $(PCOBJECTS)
	gcc -o pccomm $^ -Wall

qr_: $(X32SOURCES)
	lcc-x32 -I${X32INC} -c $^
	
qrcomm: $(X32OBJECTS)
	lcc-x32 -o qrcomm.ce $^

upload: qrcomm.ce pccomm
	x32-upload qrcomm.ce -c /dev/ttyUSB0 -e
	
server:
	gcc -o server server.c

commtest:
	gcc -o pccommtest commtest.c socket.c log.c comm.c checksum.c convert.c
	gcc -o qrcommtest sink.c socket.c log.c comm.c checksum.c convert.c

logsendtest:
	gcc -o pclogsendtest servertest1.c socket.c log.c comm.c checksum.c convert.c
	gcc -o qrlogsendtest servertest2.c socket.c log.c comm.c checksum.c convert.c

comm:
	gcc -o comm main.c comm.c checksum.c serial.c
	gcc -o comm main.c comm.c checksum.c serial.c convert.c log.c

x32_comm.ce:
	lcc-x32 x32_main.c comm.c checksum.c x32_serial.c -I${X32INC} -lib ${X32INC} -Wl-lib -o x32_comm.ce
	
clean:
	rm -f *.o *.co

