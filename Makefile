VM: main.c
	gcc --std=c11 -o VM *.c
2048::VM
	./VM 2048.obj
Debug: main.c VM.c VM.h 2048.obj
	gcc --std=c11 -o DEBUG-VM *.c -DDEBUGGER	
	./DEBUG-VM 2048.obj
clean: 
	rm -f *.o *.out *.sym

