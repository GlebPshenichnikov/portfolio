CC = g++
PROG = task
CFLAGS = -g -Wall
all: scanner.o parser.o main.o execute.o $(PROG)
	

$(PROG): main.cpp
	$(CC) $(CFLAGS) main.o scanner.o parser.o execute.o -o $(PROG)
clean:
	rm -f *.o $(PROG) 
run: $(PROG)
	./$(PROG) $(ARGS)