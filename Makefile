tomato: main.o
	gcc main.c -o tomato
main.o: main.c
	gcc -c main.c
