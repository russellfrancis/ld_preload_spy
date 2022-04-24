run: main
		./main

main: main.o
		gcc main.o -o main

main.o: main.c
		gcc -c main.c -o main.o

spy.so: spy.c
		gcc -Wall -fPIC -shared -o spy.so spy.c -ldl

clean:
		rm -f *.o *.so
