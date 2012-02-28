#     foo.o : foo.c defs.h hack.h
#             cc -c $(CFLAGS) $^ -o $@
             
# $@ : The file name of the target of the rule.
# $< : The name of the first prerequisite.
# $^ : The names of all the prerequisites, with spaces between the

#######################  CONTROL  #######################
VPATH = ../commtest/

PCSOURCES = mode_main.c serial.c log.c comm.c checksum.c convert.c
PCOBJECTS = $(PCSOURCES:.c=.o)

X32SOURCES = mode_main_x32.c x32_serial.c x32_log.c comm.c checksum.c \
	     convert.c manual_mode_x32.c full_mode_x32.c \
	     calibration_mode_x32.c panic_mode_x32.c safe_mode_x32.c yaw_mode_x32.c
X32OBJECTS = $(X32SOURCES:.c=.co)



all: pc qr

pc: pc_ pccontrol

qr: qr_ qrcontrol

pc_: $(PCSOURCES)
	gcc -c $^ -I../commtest -Wall
	
pccontrol: $(PCOBJECTS)
	gcc -o pccontrol $^ -Wall
	
qr_: $(X32SOURCES)
	lcc-x32 -c $^ -I${X32INC} -I${X32LIBDIR} -I../commtest -Wl-lib

qrcontrol: $(X32OBJECTS)
	lcc-x32 -o qrcontrol.ce -I${X32INC} -I${X32LIBDIR} -I../commtest -Wl-lib $^

upload: qrcontrol.ce
	x32-upload qrcontrol.ce -c /dev/ttyUSB0 -e
	
clean:
	rm -f *.o *.co
