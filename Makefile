SRC=client.c stack.c
FLAGS=-std=c99
MAIN=chat
CXX=gcc

all: $(SRC)
	$(CXX) -o $(MAIN) $(FLAGS) $(SRC)

a: $(MAIN)
	./$(MAIN) 8888 127.0.0.1 9999

b: $(MAIN)
	./$(MAIN) 9999 127.0.0.1 8888
