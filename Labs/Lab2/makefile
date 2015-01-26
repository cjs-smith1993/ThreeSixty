all: server

server: src/*.cpp src/*.h
	@g++ -std=c++11 src/*.cpp -pthread -o server

clean:
	@rm server