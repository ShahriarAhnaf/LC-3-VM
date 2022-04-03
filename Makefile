VM: main.c vm.c vm.h
	gcc --std=c11 -o lc3VM *.c
2048::VM
	./lc3VM 2048.obj
DEBUG: main.c vm.c vm.h
	gcc --std=c11 -o VM *.c -DDEBUGGER
clean: 
	rm -f *.o *.out 

