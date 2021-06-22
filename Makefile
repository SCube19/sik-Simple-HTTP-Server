CFLAGS = -c -Wall -Wextra -pedantic -std=c++17 -O2 -lstdc++fs

default: serwer

SocketTCP.o: SocketTCP.cpp
	g++ $(CFLAGS) SocketTCP.cpp -o SocketTCP.o

Server.o: Server.cpp SocketTCP.h Parser.h Http.h
	g++ $(CFLAGS) Server.cpp -o Server.o

Http.o: Http.cpp
	g++ $(CFLAGS) Http.cpp -o Http.o

Parser.o: Parser.cpp Http.h
	g++ $(CFLAGS) Parser.cpp -o Parser.o

main.o: main.cpp
	g++ $(CFLAGS) main.cpp -o main.o
    
serwer: main.o Server.o SocketTCP.o Parser.o Http.o
	g++ main.o Server.o SocketTCP.o Parser.o Http.o -lstdc++fs -o serwer

clean:
	rm -f Server.o
	rm -f SocketTCP.o
	rm -f serwer
	rm -f Parser.o
	rm -f Http.o
	rm -f main.o

	

