
pc:
	gcc -o pc main.c serial.c log.c comm.c checksum.c convert.c 

qr:
	lcc-x32 -o qr.ce x32_main.c x32_serial.c x32_log.c comm.c checksum.c convert.c -I${X32INC} -lib ${X32INC} -Wl-lib

server:
	gcc -o server server.c

commtest:
	gcc -o pc commtest.c socket.c log.c comm.c checksum.c convert.c
	gcc -o qr sink.c socket.c log.c comm.c checksum.c convert.c

logsendtest:
	gcc -o pc servertest1.c socket.c log.c comm.c checksum.c convert.c
	gcc -o qr servertest2.c socket.c log.c comm.c checksum.c convert.c

comm:
	gcc -o comm main.c comm.c checksum.c serial.c
	gcc -o comm main.c comm.c checksum.c serial.c convert.c log.c
x32_comm.ce:
	lcc-x32 x32_main.c comm.c checksum.c x32_serial.c -I${X32INC} -lib ${X32INC} -Wl-lib -o x32_comm.ce


