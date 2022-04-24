spy.so: spy.c
		gcc -Wall -fPIC -shared -o spy.so spy.c -ldl

clean:
		rm -f *.o *.so
