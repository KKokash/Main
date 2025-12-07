
ourtest: dplist.c ourtest.c
	mkdir -p build
	gcc dplist.c ourtest.c -o build/dplist_test2

runourtest: ourtest
	./build/dplist_test2

randomtest: dplist.c randomtest.c
	mkdir -p build
	gcc dplist.c randomtest.c -o build/dplist_test3
	
runrandomtest: randomtest
	./build/dplist_test3

all: dplist.c dplist_test.c
	mkdir -p build
	gcc dplist.c dplist_test.c -o build/dplist_test
memTest: randomtest
	valgrind --leak-check=yes ./build/dplist_test3
run: all
	./build/dplist_test

clean:
	rm -r build

zip:
	zip milestone1.zip *.c *.h Makefile
