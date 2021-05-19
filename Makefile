all: client.cpp server.cpp
	g++ client.cpp -o client
	g++ server.cpp -o server