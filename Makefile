VM: main.c
	gcc --std=c11 -o VM *.c
2048::VM
	./VM 2048.obj
Debug: main.c VM.c VM.h 2048.obj
	gcc --std=c11 -o DEBUG-VM1 *.c -DDEBUGGER	
	./DEBUG-VM1 2048.obj
clean: 
	rm -f *.txt *.o *.out *.sym

