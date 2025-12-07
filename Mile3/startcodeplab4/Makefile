all:main.out
	./main.out
main.out: main.c sbuffer.c
	gcc -g main.c sbuffer.c -o main.out -lpthread
zip:
	zip milestone3.zip *.c *.h Makefile
clean:
	rm -f main.out