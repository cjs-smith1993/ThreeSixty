all: server

server: src/server.cpp src/sockets.h src/sockets.cpp src/debug.h src/concurrentQueue.h src/concurrentQueue.cpp
	@g++ -std=c++11 src/server.cpp src/sockets.cpp src/concurrentQueue.cpp -pthread -o server

clean:
	@rm server