VM: main.c
	gcc --std=c11 -o VM *.c
2048::VM
	./VM 2048.obj
Debug: main.c VM.c VM.h objects/2048.obj
	gcc --std=c11 -o DEBUG-VM2 *.c -DDEBUGGER	
	./DEBUG-VM2 objects/2048.obj
clean: 
	rm -f *.o *.out *.sym

