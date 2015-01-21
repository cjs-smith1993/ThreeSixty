all: client

client: src/client.cpp src/headers.h
	@g++ src/client.cpp src/headers.cpp -o download

clean:
	@rm download downloads/*

clean-responses:
	@rm downloads/*