VM: main.c
	gcc --std=c11 -o VM src/*.c
Debug: main.c VM.c VM.h 2048.obj
	gcc --std=c11 -o DEBUG-VM1 src/*.c -DDEBUGGER	
	./DEBUG-VM1 objects/2048.obj
clean: 
	rm -f *.txt *.o *.out *.sym

