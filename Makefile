all: client.cpp server.cpp
	gcc client.cpp -o client
	gcc server.cpp -o server