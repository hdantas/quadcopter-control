#     foo.o : foo.c defs.h hack.h
#             cc -c $(CFLAGS) $^ -o $@
             
# $@ : The file name of the target of the rule.
# $< : The name of the first prerequisite.
# $^ : The names of all the prerequisites, with spaces between the

#######################  CONTROL  #######################
INCLUDE_DIRECTORIES = ../commtest ../control ../inputPC
GCCDIR += $(addprefix -I ,$(INCLUDE_DIRECTORIES))

vpath %.h $(INCLUDE_DIRECTORIES)
vpath %.c $(INCLUDE_DIRECTORIES)


PCSOURCES = control_main.c serial.c log.c comm.c checksum.c convert.c console_IO.c joy_function.c
PCOBJECTS = $(PCSOURCES:.c=.o)

X32SOURCES = x32_control_main.c x32_serial.c x32_log.c comm.c checksum.c \
	     convert.c x32_manual_mode.c x32_full_mode.c \
	     x32_calibration_mode.c x32_panic_mode.c x32_safe_mode.c x32_yaw_mode.c
X32OBJECTS = $(X32SOURCES:.c=.co)



all: pc qr

pc: pc_ pccontrol

qr: qr_ qrcontrol

pc_: $(PCSOURCES)
	gcc -c $^ $(GCCDIR)
	
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
