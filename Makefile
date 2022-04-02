VM: main.c vm.c vm.h
	gcc --std=c11 -o lc3VM *.c
clean: 
	rm -f *.o *.out 

