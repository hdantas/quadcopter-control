all:
	cd commtest; make
	cd control; make
    
commtest:
	cd commtest; make
	
control:
	cd control; make
	
clean:
	cd commtest; make clean
	cd control; make clean
