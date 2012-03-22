
#     foo.o : foo.c defs.h hack.h
#             cc -c $(CFLAGS) $^ -o $@
             
# $@ : The file name of the target of the rule.
# $< : The name of the first prerequisite.
# $^ : The names of all the prerequisites, with spaces between them

#######################  COMMUNICATION  #######################	
#INCLUDE_DIRECTORIES = ../commtest ../control ../inputPC
GCCDIR += $(addprefix -I ,$(INCLUDE_DIRECTORIES))

#vpath %.h $(INCLUDE_DIRECTORIES)
#vpath %.c $(INCLUDE_DIRECTORIES)

PCSOURCES = PC_main.c serial.c log.c # console_IO.c checksum.c convert.c joy_function.c
PCOBJECTS = $(PCSOURCES:.c=.o)

X32SOURCES = x32_main.c x32_serial.c x32_log.c #checksum.c convert.c
X32OBJECTS = $(X32SOURCES:.c=.co)
 
all: pc qr

pc: pc_ pccomm

qr: qr_ qrcomm
	

pc_ : $(PCSOURCES)
	gcc -c $^ -Wall $(GCCDIR)
	
pccomm : $(PCOBJECTS)
	gcc -o pccomm $^ -Wall

qr_: $(X32SOURCES)
	lcc-x32 -I${X32INC} -c $^
	
qrcomm: $(X32OBJECTS)
	lcc-x32 -o qrcomm.ce $^

upload: qrcomm.ce pccomm
	x32-upload qrcomm.ce -c /dev/ttyUSB0 -e

clean:
	rm -f *.o *.co

