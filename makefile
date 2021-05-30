# List your *.h files here:
headers = 

# List your *.c files here:
sources = testlib.c

program: $(headers) $(sources)
	gcc -fsanitize=undefined -g -Wall -pedantic $(sources) -o program -lm

.PHONY: clean check debug

clean:
	-rm program

check: program
	valgrind ./program

debug: program
	gdb program
