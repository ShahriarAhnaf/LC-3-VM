VM: main.c vm.c vm.h
	gcc *.c --std=c11 -o VM 
2048::VM
	./VM 2048.obj
Debug: main.c vm.c vm.h
	gcc *.c --std=c11 -o Debug-VM -DDEBUGGER
clean: 
	rm -f *.o *.out 

