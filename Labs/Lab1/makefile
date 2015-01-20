all: client

client: src/client.cpp src/headers.h
	@g++ src/client.cpp src/headers.cpp -o download

clean:
	@rm download out/*

clean-responses:
	@rm out/*