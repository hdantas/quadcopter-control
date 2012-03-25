all:
	cd comm; make
	cd control; make
    
comm:
	cd comm; make
	
control:
	cd control; make
	
clean:
	cd comm; make clean
	cd control; make clean
