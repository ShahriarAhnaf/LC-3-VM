VM: src/main.c src/VM.c src/VM.h
	gcc --std=c11 -o VM src/*.c

Log: src/main.c src/VM.c src/VM.h objects/2048.obj
	gcc --std=c11 -o LOG-VM src/*.c -DLOGGER	
	./LOG-VM objects/2048.obj
Debug: src/main.c src/VM.c src/VM.h objects/2048.obj
	gcc --std=c11 -o DEBUG-VM src/*.c -DDEBUGGER	
	./DEBUG-VM objects/2048.obj
run-2048:: VM /objects/2048.obj
	./VM objects/2048.obj
run-rogue:: VM /objects/rogue.obj
	./VM objects/rogue.obj
clean: 
	rm -rf *.txt *.o *.out *.sym

