all:run
	
control:
	cd control; make
	
upload: qrcontrol.ce
	cd control; make; make upload;
	
run: control
	cd control; make; make upload; ./pccontrol;

clean:
	cd control; make clean
	
	
