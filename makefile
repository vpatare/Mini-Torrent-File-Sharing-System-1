CC = g++
CFLAGS = -Wall -std=c++1z -g
OBJ = peer.o
OBJ1 = tracker1.o
%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< -lcrypto -pthread

all:peer tracker1

peer: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lcrypto -pthread

tracker1: $(OBJ1)
	$(CC) $(CFLAGS) -o $@ $^ -lcrypto -pthread



