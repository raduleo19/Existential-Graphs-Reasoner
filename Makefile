CC=gcc
CFLAGS=-fPIC -Wall -Wextra --std=c++14
COMPILE=$(CC) $(CFLAGS)

.PHONY: build clean

build: libaegraph.so

libaegraph.so: aegraph.cpp
	$(COMPILE) -shared -o $@ $<

clean:
	rm -f libaegraph.so

debug: 
	rm -f test
	g++ test.cpp aegraph.cpp -o test

run_debug:
	./test
	cat corect_output.txt

