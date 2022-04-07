VM: main.c
	gcc --std=c11 -o VM *.c
2048::VM
	./VM 2048.obj
clean: 
	rm -f *.o *.out *.sym

