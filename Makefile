VM: main.c vm.c vm.h
	gcc --std=c11 -o lc3VM *.c
2048::VM
	./lc3VM 2048.obj
clean: 
	rm -f *.o *.out 

